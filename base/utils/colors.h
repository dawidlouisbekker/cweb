#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define COL_RESET   "\x1b[0m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"

typedef enum {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_RESET,
    COLOR_COUNT  // helps to iterate or validate
} TerminalColor;

const char *color_codes[COLOR_COUNT] = {
    [COLOR_RED]    = "\x1b[31m",
    [COLOR_GREEN]  = "\x1b[32m",
    [COLOR_YELLOW] = "\x1b[33m",
    [COLOR_BLUE]   = "\x1b[34m",
    [COLOR_RESET]  = "\x1b[0m"
};

void print_colored(TerminalColor color, const char *text, ...) {
    va_list args;
    va_start(args, text);

    if (color < COLOR_COUNT) {
        printf("%s", color_codes[color]);
        vprintf(text, args);
        printf("%s", color_codes[COLOR_RESET]);
    } else {
        vprintf(text, args);
        printf("\n");
    }

    va_end(args);
}
