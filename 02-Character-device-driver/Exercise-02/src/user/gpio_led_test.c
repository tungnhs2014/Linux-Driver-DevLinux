/**
 * @file gpio_led_test.c 
 * @brief Test application for GPIO LED driver
 * 
 * This application demonstrates how to use the GPIO LED driver
 * by opening the device file and writing commands to control the LED.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <errno.h>

 /* Constants */
 #define DEVICE_PATH     "/dev/gpio_led"   /* Path to the device file */
 #define BUFFER_SIZE     64                /* Size of our read buffer */

 /**
 * @brief Print usage instructions
 * 
 * @param program_name Name of the program
 */
 static void print_usage(const char *program_name) {
     printf("Usage: %s COMMAND\n\n", program_name);
     printf("Commands:\n");
     printf("  on       Turn the LED on\n");
     printf("  off      Turn the LED off\n");
     printf("  status   Read the current LED status\n");
     printf("\nExample: %s on\n", program_name);
 }
 
 /**
 * @brief Turn the LED ON
 * @param fd File descriptor for the device
 * @return 0 on success, -1 on error
 */
 static int led_on(int fd) {
    const char *cmd = "1";
    ssize_t bytes;

    printf("Turning LED ON...\n");

    /* Write the command to turn on the LED */
    bytes = write(fd, cmd, strlen(cmd));
    if (bytes < 0) {
        perror("Error writing to device");
        return -1;
    }

    printf("Command sent successfully\n");
    return 0;
 }

 /**
 * @brief Turn the LED OFF
 * @param fd File descriptor for the device
 * @return 0 on success, -1 on error
 */
 static int led_off(int fd) {
    const char *cmd = "0";
    ssize_t bytes;

    printf("Turning LED OFF...\n");

    /* Write the command to turn off  the LED */
    bytes = write(fd, cmd, strlen(cmd));
    if (bytes < 0) {
        perror("Error writing to device");
        return -1;
    }

    printf("Command sent successfully\n");
    return 0;
 }

 /**
 * @brief Read the current LED status
 * @param fd File descriptor for the device
 * @return 0 on success, -1 on error
 */
 static int read_status(int fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes;

    printf("Reading LED status...\n");

    /* Read status from the device */
    bytes = read(fd, buffer, BUFFER_SIZE -1);
    if (bytes < 0) {
        perror("Error reading from device");
        return -1;
    }

    /* Null-terminate the string and print what we read */
    buffer[bytes] = '\0';
    printf("Status: %s", buffer);

    return 0;
 }

 int main(int argc, char *argv[]) {
    int fd;
    int ret = EXIT_SUCCESS;

    /* Check for correct number of arguments */
    if (argc != 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    /* Open the device for reading and writing */
    printf("Opening %s...\n", DEVICE_PATH);

    fd = open(DEVICE_PATH, O_RDWR);
    /* Check if device open successfully */
    if (fd < 0) {
        perror("Error opening device");
        return EXIT_FAILURE;
    }

    /* Process command */
    if (strcmp(argv[1], "on") == 0) {
        if (led_on(fd) < 0) {
            ret = EXIT_FAILURE;
        }
    }
    else if (strcmp(argv[1], "off") == 0) {
        if (led_off(fd) < 0) {
            ret = EXIT_FAILURE;
        }
    }
    else if (strcmp(argv[1], "status") == 0) {
        if (read_status(fd) < 0) {
            ret = EXIT_FAILURE;
        }
    }
    else {
        printf("Unknown command: %s\n", argv[1]);
        print_usage(argv[0]);
        ret = EXIT_FAILURE;
    }

    /* Close the device */
    printf("Closing device\n");
    close(fd);

    return ret;
 }
 