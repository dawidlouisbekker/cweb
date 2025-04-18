#include "server.h"


#define MAX_ENDPOINTS 100

const char* endpoint = "endpoint ";
const char* src_path = "./src";
int verbose = 0;




int main(int argc, char *argv[]) {
    parse_args(argc, argv, verbose);
    parse_src(src_path);
    print_colored(COLOR_GREEN,"STARTING\n");
    //ST = Single Thread
    runST(verbose);
    return 0;
}