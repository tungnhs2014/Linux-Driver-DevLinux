## Linux Character Device Driver

This project implements a minimal Linux character device driver and a user-space test application to demonstrate basic driver functionality.

### Project Structure

```
.
├── Makefile               # Main Makefile for building everything
├── README.md              # This documentation file
└── src
    ├── kernel
    │   └── simple_driver.c  # Kernel module source code
    └── user
        └── test_app.c       # User-space test application
```

### Features

This character device driver:
- Creates a device file (`/dev/simple_dev`)
- Allocates a memory buffer to store data
- Implements read/write operations
- Uses mutexes for synchronization between concurrent accesses

### Requirements

- Linux kernel headers (matching your running kernel version)
- GCC compiler
- Make utility
- Root permissions for loading the kernel module

### Building the Project

The project includes a single Makefile that handles building both the kernel module and test application:

```bash
# Build both the kernel module and test application
make

# Build only the kernel module
make kernel_module

# Build only the test application
make user_app
```

### Using the Driver

#### Loading the Module

```bash
make load
```

After loading the module, you need to set the appropriate permissions for the device file (since by default only root can access it):

```bash
sudo chmod 666 /dev/simple_dev
```

#### Testing the Driver

Run the test application, optionally with a custom message:

```bash
./src/user/test_app
```

Or with a custom message:

```bash
./src/user/test_app "Hello, custom message!"
```

Alternatively, you can run the application with root permissions without changing device permissions:

```bash
sudo ./src/user/test_app
```

#### Unloading the Module

```bash
make unload
```

### Cleaning Up

Remove all compiled files:

```bash
make clean
```

### Implementation Notes

#### Kernel Module (simple_driver.c)

- The driver creates a character device and a device file in `/dev`
- It allocates a memory buffer to store data written from user-space
- Read/write operations are properly synchronized using a mutex
- Careful error handling ensures resources are properly cleaned up
- Function names avoid conflicts with existing kernel functions (prefix `sdev_`)

#### Test Application (test_app.c)

- Opens the device file
- Writes a message to the device
- Closes and reopens the device (to reset the file position)
- Reads back the data to verify it was stored correctly
- Closes the device file
- Handles errors properly with descriptive messages

#### Common Issues

1. **Permission Denied**: If you get "Permission denied" when using the test application, you need to either:
   - Run the test application as root (`sudo ./src/user/test_app`)
   - Change permissions on the device file (`sudo chmod 666 /dev/simple_dev`)

2. **Module Not Found**: If the kernel module isn't found, check if it's loaded:
   ```bash
   lsmod | grep simple_driver
   ```

3. **Build Errors**: If you encounter build errors:
   - Make sure you have the kernel headers installed
   - Check that your kernel headers match your running kernel

### License

This project is licensed under the GPLv2 license.