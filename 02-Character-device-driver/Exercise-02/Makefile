# Makefile for GPIO LED driver project

# Directories structure
SRC_DIR := src
KERNEL_SRC_DIR := $(SRC_DIR)/kernel
USER_SRC_DIR := $(SRC_DIR)/user
BUILD_DIR := build

# Source files
KERNEL_SRC := $(KERNEL_SRC_DIR)/gpio_led_driver.c
USER_SRC := $(USER_SRC_DIR)/gpio_led_test.c

# Output files
MODULE_NAME := gpio_led_driver
APP_NAME := gpio_led_test

# Kernel module info
KERNEL_DIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Complier options 
CC := gcc
CFLAGS := -Wall -Wextra -g

# Default target
all: module app

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Build the kernel module
module: $(BUILD_DIR)
	@echo "Building kernel module..."
	cp $(KERNEL_SRC) $(BUILD_DIR)/$(MODULE_NAME).c
	@echo "obj-m := $(MODULE_NAME).o" > $(BUILD_DIR)/Makefile
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD)/$(BUILD_DIR) modules
	@cp $(BUILD_DIR)/*.ko ./

# Build the user application
app: $(BUILD_DIR)
	@echo "Building user application..."
	$(CC) $(CFLAGS) $(USER_SRC) -o $(BUILD_DIR)/$(APP_NAME)
	cp $(BUILD_DIR)/$(APP_NAME) ./

# Install the module
load:
	@echo "Loading module..."
	@sudo insmod $(MODULE_NAME).ko

# Set permissions for the device file
perms:
	@echo "Setting permissions for /dev/gpio_led..."
	@sudo chmod 666 /dev/gpio_led

# Unload the module
unload:
	@echo "Unloading module..."
	@-sudo rmmod $(MODULE_NAME) || true

# Test LED directly from command line
test_on:
	@echo "Turning LED ON..."
	@echo "1" > /dev/gpio_led

test_off:
	@echo "Turning LED OFF..."
	@echo "0" > /dev/gpio_led

test_status:
	@echo "Reading LED status..."
	@cat /dev/gpio_led

# Clean up build files
clean:
	@echo "Cleaning up..."
	rm -rf $(BUILD_DIR)
	rm -f *.ko $(APP_NAME)


.PHONY: all module app load perms unload test_on test_off test_status \
	test_app_on test_app_off test_app_status clean

# Help target
help:
	@echo "GPIO LED Driver Makefile Help"
	@echo "============================"
	@echo "Available targets:"
	@echo "  all         : Build both kernel module and user application"
	@echo "  module      : Build only the kernel module"
	@echo "  app         : Build only the user application"
	@echo "  load        : Load the kernel module"
	@echo "  perms       : Set permissions for the device file"
	@echo "  unload      : Unload the kernel module"
	@echo "  test_on     : Turn LED ON using command line"
	@echo "  test_off    : Turn LED OFF using command line"
	@echo "  test_status : Read LED status using command line"
	@echo "  test_app_on     : Turn LED ON using application"
	@echo "  test_app_off    : Turn LED OFF using application"
	@echo "  test_app_status : Read LED status using application"
	@echo "  test        : Run a full test sequence"
	@echo "  clean       : Clean up build files"
	@echo "  help        : Display this help message"