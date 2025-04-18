#include "colors.h"
#include <time.h>

void print_current_date() {
    time_t t;
    struct tm *tm_info;

    time(&t);  // Get current time
    tm_info = localtime(&t);  // Convert to local time

    char date_str[100];
    strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S", tm_info);  // Format date
    printf("Current Date and Time: %s\n", date_str);
}
