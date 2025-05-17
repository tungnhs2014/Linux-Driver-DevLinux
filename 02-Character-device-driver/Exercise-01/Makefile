# Makefile for Simple Linux Kernel Driver Project

# Source directories and file paths
SRC_DIR := src
KERNEL_SRC := $(SRC_DIR)/kernel/simple_driver.c
USER_SRC := $(SRC_DIR)/user/test_app.c

# Output file names
MODULE_NAME := simple_driver
TEST_APP := test_app

# Compiler and flags for test application
CC := gcc
CFLAGS := -Wall -Wextra -g -O2

# Kernel source directory
KERNEL_SOURCE := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
KERNEL_BUILD_DIR := $(SRC_DIR)/kernel

# Default target: build both kernel module and test application
all: kernel_module user_app

# Create kernel Makefile and build the module
kernel_module: $(KERNEL_SRC)
	@echo "=== Building kernel module ==="
	@echo "obj-m := $(MODULE_NAME).o" > $(KERNEL_BUILD_DIR)/Makefile
	$(MAKE) -C $(KERNEL_SOURCE) M=$(PWD)/$(KERNEL_BUILD_DIR) modules

# Build test application
user_app: $(USER_SRC)
	@echo "=== Building test application ==="
	$(CC) $(CFLAGS) -o $(SRC_DIR)/user/$(TEST_APP) $<

# Install kernel module
load:
	@echo "=== Loading kernel module ==="
	sudo insmod $(KERNEL_BUILD_DIR)/$(MODULE_NAME).ko

# Remove kernel module
unload:
	@echo "=== Unloading kernel module ==="
	sudo rmmod $(MODULE_NAME) || true

# Clean build artifacts
clean:
	@echo "=== Cleaning build files ==="
	@if [ -f $(KERNEL_BUILD_DIR)/Makefile ]; then \
		$(MAKE) -C $(KERNEL_SOURCE) M=$(PWD)/$(KERNEL_BUILD_DIR) clean; \
	fi
	rm -f $(KERNEL_BUILD_DIR)/Makefile
	rm -f $(SRC_DIR)/user/$(TEST_APP)

# Display help information
help:
	@echo "Available commands:"
	@echo "  make           - Build both kernel module and test application"
	@echo "  make kernel_module - Only build kernel module"
	@echo "  make user_app  - Only build test application"
	@echo "  make load      - Load kernel module"
	@echo "  make unload    - Unload kernel module"
	@echo "  make clean     - Clean build files"
	@echo "  make help      - Display this help"

# Define targets that don't correspond to file names
.PHONY: all kernel_module user_app load unload clean help