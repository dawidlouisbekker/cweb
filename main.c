#include "base/server.h"

const char* endpoint = "endpoint ";
const char* src_path = "./src";
int verbose = 1;

void handle_sigint(int sig) {
    printf("Caught SIGINT. Cleaning up...\n");
    exit(0);  // Triggers atexit()
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sigint);

    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);  // Available cores
    printf("Number of available CPU cores: %ld\n", num_cores);
    parse_args(argc, argv, verbose);
    parse_src(src_path);
    print_colored(COLOR_GREEN,"STARTING\n");
    run();
    return 0;
}