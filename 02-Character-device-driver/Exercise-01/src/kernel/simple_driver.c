/**
 * @file simple_driver.c
 * @brief Simple character device driver implementation
 *
 * This file implements a basic character device driver that:
 * - Creates a device file (/dev/simple_dev)
 * - Allocates a memory buffer to store data
 * - Implements read/write operations for user space interaction
 * - Handles synchronization for concurrent access
 *
 * @author Developer Name
 * @copyright (C) 2023
 * @license GPL-2.0
 */

#include <linux/module.h>    /* For MODULE_ macros */
#include <linux/kernel.h>    /* For kernel logging functions */
#include <linux/fs.h>        /* For file_operations, register_chrdev_region */
#include <linux/cdev.h>      /* For character device functions */
#include <linux/device.h>    /* For device_create/class_create */
#include <linux/uaccess.h>   /* For copy_to/from_user */
#include <linux/slab.h>      /* For kmalloc, kfree */
#include <linux/mutex.h>     /* For mutex operations */

/* Module information and constants */
#define DRIVER_NAME     "simple_dev"    /* Device name in /dev */
#define DRIVER_CLASS    "simple"        /* Device class name */
#define BUFFER_SIZE     PAGE_SIZE       /* Size of data buffer (4KB) */

/**
 * Device structure holding all driver state information.
 * This is better than using separate global variables.
 */
struct simple_dev {
    struct mutex lock;            /* Mutex to protect concurrent access */
    unsigned char *buffer;        /* Memory buffer to store data */
    size_t size;                  /* Current amount of data in buffer */
    dev_t devt;                   /* Device number (major+minor) */
    struct cdev cdev;             /* Character device structure */
    struct class *class;          /* Device class */
};

/* Global instance of our device */
static struct simple_dev device;

/**
 * @brief Handler for device open() operation
 *
 * Called when a process opens our device file.
 *
 * @param inode Pointer to inode structure of the device
 * @param file Pointer to file structure for this open instance
 * @return 0 on success
 */
static int mydrv_open(struct inode *inode, struct file *file)
{
    /* Store our device data in the file's private_data for later use */
    file->private_data = &device;
    
    /* Log the open operation */
    pr_info("simple_driver: Device opened\n");
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
static int mydrv_release(struct inode *inode, struct file *file)
{
    /* Log the close operation */
    pr_info("simple_driver: Device closed\n");
    return 0;
}

/**
 * @brief Handler for device read() operation
 *
 * Copies data from our kernel buffer to user space.
 *
 * @param file Pointer to file structure
 * @param buf User space buffer to copy data to
 * @param count Number of bytes to read
 * @param pos Current position in file
 * @return Number of bytes read, or negative error code
 */
static ssize_t mydrv_read(struct file *file, char __user *buf, 
                         size_t count, loff_t *pos)
{
    struct simple_dev *dev = file->private_data;
    ssize_t ret = 0;
    
    /* Acquire mutex to protect against concurrent access */
    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;  /* Return if interrupted by signal */
    
    /* Check if we're at end of data */
    if (*pos >= dev->size)
        goto out;  /* Nothing more to read */
    
    /* Adjust count if it would go past the end of data */
    if (*pos + count > dev->size)
        count = dev->size - *pos;
    
    /* Copy data from kernel space to user space buffer */
    if (copy_to_user(buf, dev->buffer + *pos, count)) {
        ret = -EFAULT;  /* Bad address error */
        goto out;
    }
    
    /* Update position and return bytes read */
    *pos += count;
    ret = count;
    
out:
    /* Always release the mutex before returning */
    mutex_unlock(&dev->lock);
    return ret;
}

/**
 * @brief Handler for device write() operation
 *
 * Copies data from user space to our kernel buffer.
 *
 * @param file Pointer to file structure
 * @param buf User space buffer to copy data from
 * @param count Number of bytes to write
 * @param pos Current position in file
 * @return Number of bytes written, or negative error code
 */
static ssize_t mydrv_write(struct file *file, const char __user *buf,
                          size_t count, loff_t *pos)
{
    struct simple_dev *dev = file->private_data;
    ssize_t ret = 0;
    
    /* Acquire mutex to protect against concurrent access */
    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;  /* Return if interrupted by signal */
    
    /* Check if we're at end of buffer */
    if (*pos >= BUFFER_SIZE) {
        ret = -ENOSPC;  /* No space left on device */
        goto out;
    }
    
    /* Adjust count if it would exceed buffer size */
    if (*pos + count > BUFFER_SIZE)
        count = BUFFER_SIZE - *pos;
    
    /* Copy data from user space buffer to kernel space */
    if (copy_from_user(dev->buffer + *pos, buf, count)) {
        ret = -EFAULT;  /* Bad address error */
        goto out;
    }
    
    /* Update position, data size, and return bytes written */
    *pos += count;
    dev->size = max(dev->size, (size_t)*pos);
    ret = count;
    
out:
    /* Always release the mutex before returning */
    mutex_unlock(&dev->lock);
    return ret;
}

/**
 * File operations structure defining the driver's capabilities.
 * This maps system calls to our handler functions.
 */
static const struct file_operations simple_fops = {
    .owner = THIS_MODULE,     /* Module that owns this structure */
    .open = mydrv_open,       /* Called on open() */
    .release = mydrv_release, /* Called on close() */
    .read = mydrv_read,       /* Called on read() */
    .write = mydrv_write,     /* Called on write() */
};

/**
 * @brief Initialize the module
 *
 * Called when the module is loaded. Sets up the device.
 *
 * @return 0 on success, negative error code on failure
 */
static int __init simple_init(void)
{
    int ret;
    struct device *dev_ret;
    
    /* Initialize mutex */
    mutex_init(&device.lock);
    
    /* Allocate memory buffer for our device */
    device.buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!device.buffer) {
        pr_err("simple_driver: Failed to allocate memory\n");
        return -ENOMEM;  /* Out of memory error */
    }
    
    /* Allocate a device number (major and minor) */
    ret = alloc_chrdev_region(&device.devt, 0, 1, DRIVER_NAME);
    if (ret < 0) {
        pr_err("simple_driver: Failed to allocate device number\n");
        goto fail_alloc_chrdev;
    }
    
    /* Create a device class */
    device.class = class_create(DRIVER_CLASS);
    if (IS_ERR(device.class)) {
        pr_err("simple_driver: Failed to create device class\n");
        ret = PTR_ERR(device.class);
        goto fail_class_create;
    }
    
    /* Create a device file in /dev */
    dev_ret = device_create(device.class, NULL, device.devt, NULL, DRIVER_NAME);
    if (IS_ERR(dev_ret)) {
        pr_err("simple_driver: Failed to create device file\n");
        ret = PTR_ERR(dev_ret);
        goto fail_device_create;
    }
    
    /* Initialize character device structure with our file operations */
    cdev_init(&device.cdev, &simple_fops);
    device.cdev.owner = THIS_MODULE;
    
    /* Add character device to the system */
    ret = cdev_add(&device.cdev, device.devt, 1);
    if (ret < 0) {
        pr_err("simple_driver: Failed to add character device\n");
        goto fail_cdev_add;
    }
    
    /* Log successful initialization with device numbers */
    pr_info("simple_driver: Initialized with major=%d, minor=%d\n",
           MAJOR(device.devt), MINOR(device.devt));
    
    return 0;

    /* Error handling with cleanup */
fail_cdev_add:
    device_destroy(device.class, device.devt);
fail_device_create:
    class_destroy(device.class);
fail_class_create:
    unregister_chrdev_region(device.devt, 1);
fail_alloc_chrdev:
    kfree(device.buffer);
    
    return ret;
}

/**
 * @brief Clean up the module
 *
 * Called when the module is unloaded. Releases all resources.
 */
static void __exit simple_exit(void)
{
    /* Cleanup in reverse order of initialization */
    
    /* Remove character device from system */
    cdev_del(&device.cdev);
    
    /* Remove device file */
    device_destroy(device.class, device.devt);
    
    /* Remove device class */
    class_destroy(device.class);
    
    /* Release device number */
    unregister_chrdev_region(device.devt, 1);
    
    /* Free the memory buffer */
    kfree(device.buffer);
    
    /* Log successful unloading */
    pr_info("simple_driver: Module unloaded\n");
}

/* Register the initialization and exit functions */
module_init(simple_init);
module_exit(simple_exit);

/* Module information */
MODULE_LICENSE("GPL");                      /* Module license (required) */
MODULE_AUTHOR("Developer Name");            /* Module author */
MODULE_DESCRIPTION("Simple character device driver"); /* Module description */
MODULE_VERSION("1.0");                      /* Module version */