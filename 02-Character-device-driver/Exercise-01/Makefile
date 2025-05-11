# Makefile for character device driver project

# Module and application names
MODULE_NAME := simple_driver
APP_NAME := simple_test

# Kernel source directory (for module compilation)
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build

# Compiler settings
CC := gcc
CFLAGS := -Wall -Wextra -O2

# Default target: build both module and test application
all: module app

# Build the kernel module
module:
	@echo "Building kernel module..."
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

# Build the test application
app:
	@echo "Building test application..."
	$(CC) $(CFLAGS) src/user/test_app.c -o $(APP_NAME)

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -f $(APP_NAME)

# Load the kernel module
load:
	@echo "Loading kernel module..."
	sudo insmod $(MODULE_NAME).ko

# Unload the kernel module
unload:
	@echo "Unloading kernel module..."
	sudo rmmod $(MODULE_NAME)

# Run the test application
test: app
	@echo "Running test application..."
	sudo ./$(APP_NAME)

# View recent kernel log messages
log:
	@echo "Showing kernel log messages..."
	sudo dmesg | tail -20

# Define kernel module object files
# This tells the kernel build system which source files to compile
obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-objs := src/kernel/simple_driver.o

# Declare targets that don't represent files
.PHONY: all module app clean load unload test log