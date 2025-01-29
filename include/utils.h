#ifndef UTILS_H
#define UTILS_H

#define MAX_NAME_LENGTH 50   

// Helper function prototypes
int getValidIntInput();
float getValidFloatInput();
void getValidStringInput(char *input, int max_len);
int getValidDateInput(char *date, int max_len);

#endif
