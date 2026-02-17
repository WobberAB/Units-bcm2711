#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include "fileUtils.h"
#include "config.h"


// Helper function to validate file paths (prevent directory traversal attacks)
static int isValidPath(const char *filename) {
    if (filename == NULL || strlen(filename) == 0) {
        return 0;
    }
    // Check for directory traversal attempts
    if (strstr(filename, "..") != NULL) {
        return 0;
    }
    // Ensure path doesn't start with / (only relative paths allowed)
    // Or enforce a specific base directory
/*    if (filename[0] == '/') {
        // You might want to allow only specific absolute paths
        // For now, rejecting all absolute paths except whitelisted ones
        return 0;
    }*/
    return 1;
}

int configDirExists(void){
    DIR* dir = opendir("/etc/units/bcm2711");
    if (dir) {
        /* Directory exists. */
        closedir(dir);
        return 1;
    } else if (ENOENT == errno) {
        /* Directory does not exist. */
        return 0;
    } else {
        /* opendir() failed for some other reason. */
        return 0;
    }
}

// FIXED: Added bounds checking, format string safety, and path validation
void fileToString(char *h, size_t h_size, const char *filename){
    char buff[100];

    if(filename == NULL || h == NULL || h_size == 0){
        openlog("units-bcm2711 config", LOG_PID, LOG_USER);
        syslog(LOG_ERR, "Invalid parameters passed to fileToString");
        closelog();
        return;
    }

    // Validate path to prevent directory traversal
    if (!isValidPath(filename)) {
        openlog("units-bcm2711 config", LOG_PID, LOG_USER);
        syslog(LOG_ERR, "Invalid file path detected: %s", filename);
        closelog();
        return;
    }

    FILE* ptr_file = fopen(filename, "r");
    if (ptr_file == NULL) {
        openlog("units-bcm2711 config", LOG_PID, LOG_USER);
        syslog(LOG_ERR, "File %s was not found or cannot be opened", filename);
        closelog();
        return;
    }

    // Use fgets instead of fscanf for safer input
    if (fgets(buff, sizeof(buff), ptr_file) != NULL) {
        // Remove trailing newline if present
        buff[strcspn(buff, "\n")] = '\0';

        // Safe string concatenation with bounds checking
        size_t current_len = strlen(h);
        size_t buff_len = strlen(buff);

        if (current_len + buff_len + 1 <= h_size) {
            strncat(h, buff, h_size - current_len - 1);
        } else {
            openlog("units-bcm2711 config", LOG_PID, LOG_USER);
            syslog(LOG_ERR, "Buffer overflow prevented in fileToString");
            closelog();
        }
    }

    fclose(ptr_file);
}

// FIXED: Added bounds checking and path validation
double fileTodouble(const char *filename){
    if(filename == NULL){
        return ERRNO;
    }

    // Validate path to prevent directory traversal
    if (!isValidPath(filename)) {
        openlog("units-bcm2711 config", LOG_PID, LOG_USER);
        syslog(LOG_ERR, "Invalid file path detected: %s", filename);
        closelog();
        return ERRNO;
    }

    char buff[64];  // Increased size to handle larger numbers
    FILE* ptr_file = fopen(filename, "r");

    if (ptr_file == NULL) {
        openlog("units-bcm2711 config", LOG_PID, LOG_USER);
        syslog(LOG_ERR, "File %s was not found", filename);
        closelog();
        return ERRNO;
    }

    // Use fgets instead of fscanf for safer input
    if (fgets(buff, sizeof(buff), ptr_file) != NULL) {
        fclose(ptr_file);

        // Validate that the string contains a valid number
        char *endptr;
        errno = 0;
        double result = strtod(buff, &endptr);

        // Check for conversion errors
        if (errno == ERANGE || endptr == buff) {
            openlog("units-bcm2711 config", LOG_PID, LOG_USER);
            syslog(LOG_ERR, "Invalid number format in file %s", filename);
            closelog();
            return ERRNO;
        }

        return result;
    }

    fclose(ptr_file);
    return ERRNO;
}

// FIXED: Added bounds checking and path validation
int fileToInt(const char *filename){
    if(filename == NULL){
        return ERRNO;
    }

    // Validate path to prevent directory traversal
    if (!isValidPath(filename)) {
        openlog("units-bcm2711 config", LOG_PID, LOG_USER);
        syslog(LOG_ERR, "Invalid file path detected: %s", filename);
        closelog();
        return ERRNO;
    }

    char buff[32];  // Increased size to handle larger integers
    FILE* ptr_file = fopen(filename, "r");

    if (ptr_file == NULL) {
        openlog("units-bcm2711 config", LOG_PID, LOG_USER);
        syslog(LOG_ERR, "File %s was not found", filename);
        closelog();
        return ERRNO;
    }

    // Use fgets instead of fscanf for safer input
    if (fgets(buff, sizeof(buff), ptr_file) != NULL) {
        fclose(ptr_file);

        // Validate that the string contains a valid integer
        char *endptr;
        errno = 0;
        long result = strtol(buff, &endptr, 10);
        // Check for conversion errors and overflow
        if (errno == ERANGE || endptr == buff || result > INT_MAX || result < INT_MIN) {
            openlog("units-bcm2711 config", LOG_PID, LOG_USER);
            syslog(LOG_ERR, "Invalid integer format or overflow in file %s", filename);
            closelog();
            return ERRNO;
        }
        return (int)result;
    }
    fclose(ptr_file);
    return ERRNO;
}


