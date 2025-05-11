/**
 * main.c - Basic Linux kernel module example
 *
 * This module demonstrates the essential components of a Linux kernel module
 * with thorough explanations of each part.
 *
 */

/* Required kernel header files */
#include <linux/init.h>      /* Defines module init/exit macros */
#include <linux/module.h>    /* Core module functionality */
#include <linux/kernel.h>    /* Provides kernel logging functions */
#include <linux/fs.h>        /* File operations structure */
#include <linux/mutex.h>     /* For mutex operations */

/**
 * Module metadata - this information appears when using modinfo
 * The LICENSE declaration is particularly important as it affects
 * which kernel symbols your module can access.
 */
MODULE_LICENSE("GPL");                   /* Must be GPL for full symbol access */
MODULE_AUTHOR("Developer Name");         /* Who wrote this module */
MODULE_DESCRIPTION("Basic kernel module example with explanations");
MODULE_VERSION("1.0");                   /* Module version number */

/* Module parameters allow configuration without recompiling */
static char *device_name = "mydevice";   /* Default parameter value */
/* This creates a parameter visible at /sys/module/simple_module/parameters/device_name */
module_param(device_name, charp, 0644);  /* Type: char pointer, Mode: 0644 */
MODULE_PARM_DESC(device_name, "Name of the device (default: mydevice)");

/* Module's private data structure */
static struct {
    struct mutex lock;               /* Protects access to this structure */
    unsigned long counter;           /* Example counter to demonstrate data */
    bool initialized;                /* Track if module is fully initialized */
} module_data;

/**
 * simple_init - Module initialization function
 *
 * Called when module is loaded with insmod. Performs all setup operations.
 * The __init marker tells the kernel this function is only needed during 
 * initialization and its memory can be freed afterward.
 *
 * Return: 0 on success, negative error code on failure
 */
static int __init simple_init(void)
{
    /* Initialize module data */
    mutex_init(&module_data.lock);
    module_data.counter = 0;
    
    /* Log a message to the kernel ring buffer (view with dmesg) */
    pr_info("Simple module: Initialized with device name: %s\n", device_name);
    
    /* Demonstration of mutex usage for thread-safe operations */
    mutex_lock(&module_data.lock);
    module_data.initialized = true;
    module_data.counter++;
    mutex_unlock(&module_data.lock);
    
    pr_info("Simple module: Counter value: %lu\n", module_data.counter);
    
    /* Return 0 to indicate successful initialization */
    return 0;
}

/**
 * simple_exit - Module cleanup function
 *
 * Called when module is unloaded with rmmod. Responsible for releasing
 * all resources allocated by simple_init to prevent memory leaks.
 * The __exit marker means this function is only needed during unloading.
 */
static void __exit simple_exit(void)
{
    /* Clean up - in a real module, you would free all allocated resources here */
    mutex_lock(&module_data.lock);
    module_data.initialized = false;
    /* Final counter value for demonstration */
    pr_info("Simple module: Final counter value: %lu\n", module_data.counter);
    mutex_unlock(&module_data.lock);
    
    pr_info("Simple module: Unloaded successfully\n");
}

/* 
 * Register the init and exit functions - essential for the module to work.
 * These macros create special ELF sections that the module loader recognizes.
 */
module_init(simple_init);
module_exit(simple_exit);