#include "colors.h"
#include <unistd.h>

int get_available_threads() {
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    if (nprocs < 1) {
        perror("sysconf");
        return 1;
    }
    int num = (int)nprocs;
    printf("Threads aval: %s%d%s\n", BLUE, num, COL_RESET);
    return num;
}