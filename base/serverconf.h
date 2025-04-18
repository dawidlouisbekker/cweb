//#include "serverconf.h"
#include "data/endpoints.h"
#include "utils/hardware.h"
#include "responses.h"

#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

extern int addPath(char* path);

typedef enum {
    VERBOSE = 0,    // Print data
    PORT = 8081,
    NUM_ARGS        // This keeps track of the number of arguments
} Args;

// Array of strings corresponding to enum values
const char* arg_strings[] = {
    "verbose"  // for VERBOSE enum
};


// ENDPOINT SETUPS
void parse_src(const char *directory_path) {
    int dirLen = 0;


    DIR *dir = opendir(directory_path);
    if (dir == NULL) {
        perror("opendir failed");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        dirLen = strlen(entry->d_name);
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", directory_path, entry->d_name);
        struct stat path_stat;
        if (stat(full_path, &path_stat) == 0) {
            if (S_ISREG(path_stat.st_mode)) {
                
                const char *ext = strrchr(entry->d_name, '.');
                if (ext != NULL && strcmp(ext, ".cweb") == 0) {
                    printf("→ CWEB: %s\n",entry->d_name);
                    char real_path[4096];
                    snprintf(real_path, sizeof(real_path), "./src/%s", entry->d_name);
                    addPath(real_path);
                    printf("---------------------\n");
                } else {
                    printf("/%s\n", entry->d_name);
                }
                
            } else if (S_ISDIR(path_stat.st_mode)) {
                printf("/%s\n", entry->d_name);
            }
        } else {
            perror("stat failed");
        }
    };

    closedir(dir);
};




void parse_args(int argc, char* argv[], int verbose) {
    for (int i = 1; i < argc; i++) {
        for (int j = 0; j < NUM_ARGS; j++) {
            // Compare argument with the string
            if (strncmp(argv[i], arg_strings[j], strlen(arg_strings[j])) == 0) {
                printf("Matched: %s\n", arg_strings[j]);
                // Here you can handle specific argument logic based on the enum
                if (j == VERBOSE) {
                    verbose = 1;
                    printf("Verbose flag is set.\n");
                }
            }
        }
    }
};
