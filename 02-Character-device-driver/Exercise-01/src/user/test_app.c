/**
 * @file test_app.c
 * @brief Test application for simple character device driver
 *
 * This application demonstrates how to:
 * - Open the character device
 * - Write data to the device
 * - Read data back from the device
 * - Close the device
 *
 * @author TungNHS
 * @license GPL-2.0
 */

#include <stdio.h>      /* For printf, perror */
#include <stdlib.h>     /* For EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h>     /* For strlen */
#include <unistd.h>     /* For close */
#include <fcntl.h>      /* For open, O_RDWR */
#include <errno.h>      /* For errno */

/* Constants */
#define DEVICE_PATH     "/dev/simple_dev"   /* Path to the device file */
#define BUFFER_SIZE     1024                /* Size of our read buffer */

/**
 * @brief Write a message to the device
 *
 * This function writes a string to the character device.
 *
 * @param fd File descriptor of the opened device
 * @param message String message to write
 * @return 0 on success, -1 on error
 */
static int write_device(int fd, const char *message)
{
    ssize_t bytes;
    size_t length = strlen(message);
    
    /* Print what we're about to do */
    printf("Writing message: %s\n", message);
    
    /* Write the message to the device */
    bytes = write(fd, message, length);
    
    /* Check for errors */
    if (bytes < 0) {
        perror("Error writing to device");
        return -1;
    }
    
    /* Log how many bytes were written */
    printf("Wrote %zd bytes\n", bytes);
    return 0;
}

/**
 * @brief Read data from the device
 *
 * This function reads data from the character device.
 *
 * @param fd File descriptor of the opened device
 * @return 0 on success, -1 on error
 */
static int read_device(int fd)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes;
    
    /* Go to the beginning of the device */
    lseek(fd, 0, SEEK_SET);
    
    /* Print what we're about to do */
    printf("Reading from device...\n");
    
    /* Read data from the device into our buffer */
    bytes = read(fd, buffer, BUFFER_SIZE - 1);
    
    /* Check for errors */
    if (bytes < 0) {
        perror("Error reading from device");
        return -1;
    }
    
    /* Null-terminate the string and print what we read */
    buffer[bytes] = '\0';
    printf("Read %zd bytes: %s\n", bytes, buffer);
    return 0;
}

/**
 * @brief Main function
 *
 * Opens the device, writes data to it, reads it back, then closes the device.
 *
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error
 */
int main(int argc, char *argv[])
{
    int fd;
    const char *message = "Hello from user space!";
    
    /* Use custom message if provided as command line argument */
    if (argc > 1)
        message = argv[1];
    
    /* Open the device for reading and writing */
    printf("Opening %s...\n", DEVICE_PATH);
    fd = open(DEVICE_PATH, O_RDWR);
    
    /* Check if device opened successfully */
    if (fd < 0) {
        perror("Error opening device");
        printf("Make sure the simple_driver module is loaded\n");
        return EXIT_FAILURE;
    }
    
    /* Write to the device */
    if (write_device(fd, message) < 0) {
        close(fd);
        return EXIT_FAILURE;
    }
    
    /* Read from the device */
    if (read_device(fd) < 0) {
        close(fd);
        return EXIT_FAILURE;
    }
    
    /* Close the device */
    printf("Closing device\n");
    close(fd);
    
    return EXIT_SUCCESS;
}