# Basic Makefile for Linux kernel module

# Add warning flags
EXTRA_CFLAGS += -Wall

# Module name (must match the .c file name without extension)
obj-m := src/main.o

# Kernel build directory - this works on most systems
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build

# Default target - builds the module
all:
	make -C $(KERNEL_DIR) M=$(PWD) modules

# Cleans build files
clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

.PHONY: all clean