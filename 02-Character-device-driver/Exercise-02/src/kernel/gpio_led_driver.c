/**
 * @file gpio_led_driver.case
 * @brief Simple GPIO LED driver for Raspberry Pi 3B+ using direct register access
 * 
 * This driver implements direct maipulation of BCM2837 GPIO registers
 * to control LED. It creates a character device driver interface with basic read/write operation
 */

 #include <linux/module.h>  /* For MODULE_marcos */
 #include <linux/kernel.h>  /* For kernel logging functions */
 #include <linux/fs.h>      /* For file_operations, register_chrdev_region */
 #include <linux/cdev.h>    /* For character device functions */
 #include <linux/device.h>  /* For device_create/class_create */
 #include <linux/uaccess.h> /* For copy_to/from_user */
 #include <linux/slab.h>    /* For kmalloc, kfree */
 #include <linux/mutex.h>   /* For mutex operations */
 #include <linux/io.h>      /* For ioremap, iounmap */

 /* Module information and constant */
 #define DRIVER_NAME     "gpio_led"         /* Device name in /dev/ */
 #define DRIVER_CLASS    "gpio_led_class"   /* Device class name */
 #define BUFFER_SIZE     PAGE_SIZE          /* Size of data buffer (4KB) */

 /* Raspberry Pi 3B+ GPIOO register (BCM2837) */
 #define BCM2837_GPIO_BASE     0x3F200000  /* Physical base address of GPIO */
 #define GPIO_REG_SIZE         0x1000      /* GPIO register area size (4KB) */

 /* GPIO pin for LED c */
 #define GPIO_LED_PIN          17          /* GPIO pin for LED (pin 17) */

 /* Register offsets */
 #define GPFSEL0               0x00        /* GPIO Function Select 0 */
 #define GPFSEL1               0x04        /* GPIO Function Select 1 */
 #define GPFSEL2               0x08        /* GPIO Function Select 2 */
 #define GPSET0                0x1C        /* GPIO Pin Output Set 0 */
 #define GPCLR0                0x28        /* GPIO Pin Output Clear 0 */

 /* GPIO function select values */
 #define GPIO_FUNCTION_IN      0           /* Input */
 #define GPIO_FUNCTION_OUT     1           /* Output */

 /* Command values for LED control via write operation */
 #define LED_CMD_ON    '1'     /* Turn LED on */
 #define LED_CMD_OFF   '0'     /* Turn LED off */

 /**
  * Device structure holding all driver state information
  */
 struct gpio_led_dev {
    struct mutex lock;         /* Mutex to protect concurent access */
    unsigned char *buffer;     /* Memory buffer to store data */
    size_t buffer_size;        /* Current amount of data in buffer */
    dev_t dev_num;             /* Device number (major + minor) */
    struct cdev cdev;          /* Character device structure */
    struct class *class;       /* Device class */
    struct device *device;     /* Device structure */
    void __iomem *gpio_base;    /* Virtual address of GPIO registers */
    int led_state;             /* Current LED state (0 = off, 1 = on)*/
 };

 /* Global instance of our device */
 static struct gpio_led_dev gpio_led_device = {0};

 /* Forward declarations for file operations */
 static int gpio_led_open(struct inode *inode, struct file *file);
 static int gpio_led_release(struct inode *inode, struct file *file);
 static ssize_t gpio_led_read(struct file *file, char __user *buf, size_t count, loff_t *pos);
 static ssize_t gpio_led_write(struct file *file, const char __user *buf, size_t count, loff_t *pos);

/**
 * File operation structure defining the driver's capabilities
 */
 static const struct file_operations gpio_led_fops = {
    .owner = THIS_MODULE,           /* Module that owns this structure */
    .open = gpio_led_open,          /* Called on open() */
    .release = gpio_led_release,    /* Called on close() */
    .read = gpio_led_read,          /* Called on read() */
    .write = gpio_led_write,        /* Called on write() */
 };

 /**
  * @brief Configure the GPIO pin for LED as output
  */
 static void gpio_led_configure_pin(void) {
   unsigned int fsel_reg;
   unsigned int fsel_bit;
   unsigned int value;

   /* Calculate which FESEL register and bit position within that register */
   fsel_reg = GPFSEL0 + ((GPIO_LED_PIN / 10) * 4);
   fsel_bit = (GPIO_LED_PIN % 10) * 3;

   /* Read current value */
   value = readl(gpio_led_device.gpio_base + fsel_reg);

   /* Clear the bits for this pin */
   value &= ~(7 << fsel_bit);    // 7=> 111 (binary)

   /* Set as output */
   value |= (GPIO_FUNCTION_OUT << fsel_bit);

   /* Write updated value */
   writel(value, gpio_led_device.gpio_base + fsel_reg);

   pr_info("gpio_led_driver: Configured GPIO pin %d as output\n", GPIO_LED_PIN);
 }

 /**
  * @brief Turn the LED off by writing to GPCLR register
  */
 static void gpio_led_off(void) {
   /* Clear the pin to turn LED off */
   writel(1 << GPIO_LED_PIN, gpio_led_device.gpio_base + GPCLR0);
   gpio_led_device.led_state = 0;
   pr_info("gpio_led_driver: LED turned OFF\n");
 }

 /**
  * @brief Turn the LED on by writing to GPSET register
  */
 static void gpio_led_on(void) {
   /* Set the pin to turn LED on */
   writel(1 << GPIO_LED_PIN, gpio_led_device.gpio_base + GPSET0);
   gpio_led_device.led_state = 1;
   pr_info("gpio_led_driver: LED turned ON\n");
 }

 /**
  * @brief Handler for device open() operation
  * 
  * Called when a process opens our device file
  * 
  * @param inode Pointer to inode structure of the device
  * @param file Pointer to file structure for this open instance
  * @return 0 on success
  */
 static int gpio_led_open(struct inode *inode, struct file *file) {
   /* Store our device data in the file's private_data for later use */
   file->private_data = &gpio_led_device;

   /* Log the open operation */
   pr_info("gpio_led_driver: Device openned\n");
   return 0;
 }

 /**
 * @brief Handler for device close() operation
 *
 * Called when a process closes our device file.
 *
 * @param inode Pointer to inode structure of the device
 * @param file Pointer to file structure for this open instance
 * @return 0 on success
 */
 static int gpio_led_release(struct inode *inode, struct file *file) {
   /* Log the close operation */
   pr_info("gpio_led_driver: Device closed\n");
   return 0;
 }

 /**
  * @brief Handler for device read() operation
  * 
  * Copies the current LED status to user space.
  * 
  * @param file Pointer to file structure
  * @param count Number of bytes to read
  * @param pos Current position in file
  * @return Number of bytes read, or negative error code
  */
 static ssize_t gpio_led_read(struct file *file, char __user *buf, size_t count, loff_t *pos) {
   struct gpio_led_dev *dev = file->private_data;
   char status[8];
   size_t status_size;  
   ssize_t ret = 0;

   /* Lock to protect against concurrent access */
   if (mutex_lock_interruptible(&dev->lock)) {
      return -ERESTARTSYS;
   }

   /* Only proceed if we haven't sent data yet */
   if (*pos > 0) {
    goto out;
   }

   /* Generate status string */
   sprintf(status, "LED=%d\n", dev->led_state);
   status_size = strlen(status);

   /* Only copy up to the user's requested amount */
   if (count < status_size) {
      status_size = count;
   }

   /* Copy to user space */
   if (copy_to_user(buf, status, status_size)) {
      ret = -EFAULT;
      goto out;
   }

   /* Update position and return bytes read */
   *pos += status_size;
   ret = status_size;

out:
   mutex_unlock(&dev->lock);
   return ret;
 }

 /**
  * @brief Handler for device driver write() operation
  * 
  * Controls the LED based on user input.
  * Write '1' to turn LED on, '0' to turn LED off.
  * 
  * @param file Pointer to file structure
  * @param buf User space buffer to copy data from
  * @param count Number of bytes to write
  * @param pos Current position in file
  * @return Number of bytes written, or negative error code
  */
 static ssize_t gpio_led_write(struct file *file, const char __user *buf, size_t count, loff_t *pos) {
   struct gpio_led_dev *dev = file->private_data;
   char cmd[8];
   ssize_t ret = 0;

   /* Check for valid data */
   if (count < 1) {
      return -EINVAL;
   }
   
   /* Truncate input to prevent buffer overflow */
   if (count > sizeof(cmd) - 1) {
      count = sizeof(cmd) - 1;
   }
   
   /* Lock to protect against concurrent accesss */
   if (mutex_lock_interruptible(&dev->lock)) {
      return -ERESTARTSYS;
   }

   /* Copy command from user space */
   if (copy_from_user(cmd, buf, count)) {
      ret = -EFAULT;
      goto out;
   }

   /* Null-terminate the command */
   cmd[count] = '\0';

   /* Process the commnad */
   switch (cmd[0]) {
    case LED_CMD_ON:
        gpio_led_on();
        break;
    
    case LED_CMD_OFF:
        gpio_led_off();
        break;

    default:
        /* Store the input in our buffer for future read operations */
        if (count > BUFFER_SIZE)
            count = BUFFER_SIZE;
            
        memcpy(dev->buffer, cmd, count);
        dev->buffer_size = count;
        break;
    }

    /* Return the number of bytes processed */
    ret = count;
    
out:
    mutex_unlock(&dev->lock);
    return ret;
 }

 /**
  * @brief Initialize the module
  * 
  * Called when the module is loaded. Sets up the device and GPIO.
  * 
  * @return 0 on success, negative error code on failure
  */
 static int __init gpio_led_init(void) {
   int ret;
   
   /* Initialize device structure */
   memset(&gpio_led_device, 0, sizeof(struct gpio_led_dev));

   /* Initialize mutex */
   mutex_init(&gpio_led_device.lock);

   /* Allocate memory buffer for our device */
   gpio_led_device.buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
   if (!gpio_led_device.buffer) {
      pr_err("gpio_led_driver: Failed to allocate memory\n");
      ret = -ENOMEM;
   }

   /* Map GPIO register */
   gpio_led_device.gpio_base = ioremap(BCM2837_GPIO_BASE, GPIO_REG_SIZE);
   if (!gpio_led_device.gpio_base) {
      pr_err("gpio_led_driver: Failed to map GPIO registers\n");
      ret = -ENOMEM;
      goto fail_ioremap;
   }

   /* Configure GPIO pin for LED control */
   gpio_led_configure_pin();

   /* Initialize LED state (turn off at starup) */
   gpio_led_off();

   /* Allocat a device number (major and minor) */
   ret = alloc_chrdev_region(&gpio_led_device.dev_num, 0, 1, DRIVER_NAME);
   if (ret < 0) {
      pr_err("gpio_led_driver: Failed to allocate device number\n");
      goto fail_alloc_chrdev;
   }

   /* Create a device class */
   gpio_led_device.class = class_create(DRIVER_CLASS);
   if (IS_ERR(gpio_led_device.class)) {
      pr_err("gpio_led_driver: Failed to create device class\n");
      ret = PTR_ERR(gpio_led_device.class);
      goto fail_class_create;
   }

   /* Create a device file in /dev */
   gpio_led_device.device = device_create(gpio_led_device.class, NULL, 
                               gpio_led_device.dev_num, NULL, DRIVER_NAME);
   if (IS_ERR(gpio_led_device.device)) {
      pr_err("gpio_led_driver: Failed to create device file\n");
      ret = PTR_ERR(gpio_led_device.device);
      goto fail_device_create;
   }

   /* Initialize character device sructure with our file operations */
   cdev_init(&gpio_led_device.cdev, &gpio_led_fops);
   gpio_led_device.cdev.owner = THIS_MODULE;

   /* Add character device to the system */
   ret = cdev_add(&gpio_led_device.cdev, gpio_led_device.dev_num, 1);
   if (ret < 0) {
      pr_err("gpio_led_driver: Failed to add character device\n");
      goto fail_cdev_add;
   }
   
   /* Log sucessful initalization */
   pr_info("gpio_led_driver: Initialized with major = %d, minor = %d\n", 
            MAJOR(gpio_led_device.dev_num), MINOR(gpio_led_device.dev_num));
   pr_info("gpio_led_driver: Created device file: /dev/%s\n", DRIVER_NAME);
   pr_info("gpio_led_driver: Write '1' to turn LED on, '0' to turn LED off\n");

   return 0;

 /* Error handling with cleanup */
 fail_cdev_add:
      device_destroy(gpio_led_device.class, gpio_led_device.dev_num);
 fail_device_create:
      class_destroy(gpio_led_device.class);  
 fail_class_create:
      unregister_chrdev_region(gpio_led_device.dev_num, 1);
 fail_alloc_chrdev:
      iounmap(gpio_led_device.gpio_base);
 fail_ioremap:
      kfree(gpio_led_device.buffer);

  return ret;
 }

 /**
  * @brief Clean up the module
  * 
  * Called when the module is unloaded. Release all resources. 
  */
 static void __exit gpio_led_exit(void) {
   /* Turn off LED when unloading */
   gpio_led_off();
   
   /* Remove character device from system */
   cdev_del(&gpio_led_device.cdev);

   /* Remove device file */
   device_destroy(gpio_led_device.class, gpio_led_device.dev_num);

   /* Remove device file */
   class_destroy(gpio_led_device.class);

   /* Release device number */
   unregister_chrdev_region(gpio_led_device.dev_num, 1);

   /* Unmap GPIO registers */
   iounmap(gpio_led_device.gpio_base);
    
   /* Free the memory buffer */
   kfree(gpio_led_device.buffer);
    
   /* Log successful unloading */
   pr_info("gpio_led_driver: Module unloaded\n");
 }

 /* Register the initialization and exit function */
 module_init(gpio_led_init);
 module_exit(gpio_led_exit);

 /* Module information */
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("TungNHS");
MODULE_DESCRIPTION("Simple GPIO LED driver for Raspberry Pi using direct register access");
MODULE_VERSION("1.0");