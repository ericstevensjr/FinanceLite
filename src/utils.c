#define _XOPEN_SOURCE 700
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Helper function to get a valid integer input from the user
int getValidIntInput() {
    int value;
    while (scanf("%d", &value) != 1) {
        printf("Error: Invalid input. Please enter a valid integer.\n");
        while (getchar() != '\n');  // Clear buffer
    }
    return value;
}

// Helper function to get a valid float input from the user
float getValidFloatInput() {
    float value;
    while (scanf("%f", &value) != 1 || value <= 0) {
        printf("Error: Invalid input. Please enter a valid positive number.\n");
        while (getchar() != '\n');  // Clear buffer
    }
    return value;
}

// Helper function to get a valid non-empty string input
void getValidStringInput(char *input, int max_len) {
    while (1) {
        fgets(input, max_len, stdin);
        input[strcspn(input, "\n")] = 0;  // Remove trailing newline
        if (strlen(input) == 0) {
            printf("Error: Input cannot be empty. Please enter a valid string.\n");
        } else {
            break;
        }
    }
}

// Helper function to validate date input (YYYY-MM-DD)
int getValidDateInput(char *date, int max_len) {
    while (1) {
        fgets(date, max_len, stdin);
        date[strcspn(date, "\n")] = 0;  // Remove trailing newline
        
        struct tm tm;
        if (strptime(date, "%Y-%m-%d", &tm) == NULL) {
            printf("Error: Invalid date format. Please enter a date in the format YYYY-MM-DD.\n");
        } else {
            return 1; // Valid date
        }
    }
}
