#include <stdio.h>
// Function to print text in green
void printGreen(const char *text) {
    printf("\033[32m%s\033[0m\n", text);
}

// Function to print text in blue
void printBlue(const char *text) {
    printf("\033[34m%s\033[0m\n", text);
}

// Function to print text in yellow
void printYellow(const char *text) {
    printf("\033[33m%s\033[0m\n", text);
}

// Function to print text in red
void printRed(const char *text) {
    printf("\033[31m%s\033[0m\n", text);
}