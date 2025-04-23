#define _GNU_SOURCE
#include "shared.h"
#include "colorprint.h"
#include <stdlib.h>
#include <sched.h>

#include <sys/inotify.h>
#include <sys/mman.h>

#include <unistd.h>
#include <limits.h>
#include <linux/limits.h>
#include <string.h>
#include <stdarg.h>

#include <signal.h>
#include <execinfo.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + NAME_MAX + 1))

#define MAX_THREADS 2

#define MAX_CONF_LINES (MAX_THREADS * 2)
#define CONF_LINE_SIZE 64

#define THREAD_STACK_SIZE (1024 * 64)
#define CACHE_LINE_SIZE 64
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))


int safePrint(const char* str) {
    for (int i = 0; i < 1024; i++) {
        if (str[i] == '\0') {
            return 1;
        }
        printf("%c",str[i]);
    }
    return 0;

}

void printCWD() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %.50s\n", cwd);
    } else {
        perror("getcwd");
        return;
    }
}



typedef struct {
    const char* filename;
    const char* command;
    char padding[CACHE_LINE_SIZE - sizeof(int) - 2 * sizeof(void*)];
} __attribute__((aligned(CACHE_LINE_SIZE))) ThreadData;


ThreadData thread_data[MAX_THREADS];

char *stacks[MAX_THREADS];

int start_notify(void* arg)
{
    int index = *(int*)arg;
    //free(arg);  // optional cleanup, since you malloc'd it in spawn_watcher()

    ThreadData* data = &thread_data[index];
    printf("FILE: %s\n",data->filename);
    printf("COMMAND: %s\n",data->command);

    int fd = inotify_init1(IN_NONBLOCK);
    if (fd < 0) {
        perror("inotify_init");
        return -1;
    }

    int wd = inotify_add_watch(fd, data->filename, IN_MODIFY);
    if (wd < 0) {
        perror("Error adding file watcher");
        close(fd);
        return -1;
    }

    printf("Thread %d watching '%s' for changes...\n", index ,data->filename);
    // ✅ Use a safe separate buffer for inotify read
    char* buffer = malloc(EVENT_BUF_LEN);
    if (!buffer) {
        perror("malloc");
        close(fd);
        return -1;
    }
    int i = 0;
    while (1) {
        int length = read(fd, buffer, EVENT_BUF_LEN);
        if (length < 0) {
            usleep(100000);
            continue;
        }

        i = 0;
        while (i < length) {
            struct inotify_event* event = (struct inotify_event*)&buffer[i];
            if (event->mask & IN_MODIFY) {
                printf("File '%s' was modified!\n", data->filename);
                int status = system(data->command);
                if (status == -1) {
                    perror("Error executing command");
                    break;
                } else {
                    printf("Update \x1b[32msuccess\x1b[0m\n");
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }

    // Not reached, but for completeness
    free(buffer);
    inotify_rm_watch(fd, wd);
    close(fd);
    return 0;
}

volatile int lock = 0;
void acquire_lock(volatile int* lock_addr) {
    while (__sync_lock_test_and_set(lock_addr, 1)) {
        // Busy wait (spin)
    }
}

void release_lock(volatile int* lock_addr) {
    __sync_lock_release(lock_addr); // same as *lock_addr = 0;
}

char* previous_lines[MAX_CONF_LINES];

void load_file(const char* filename, char* lines[], int* count) {
    FILE* file = fopen(filename, "r");
    if (!file) return;

    char buffer[CONF_LINE_SIZE];
    int i = 0;
    while (fgets(buffer, CONF_LINE_SIZE, file) && i < MAX_CONF_LINES) {
        lines[i] = strdup(buffer);
        i++;
    }
    *count = i;
    return;
}



void compare_and_print_changes(const char* filename) {
    char* new_lines[MAX_CONF_LINES] = {0};
    int new_count = 0;
    load_file(filename, new_lines, &new_count);

    for (int i = 0; i < new_count; i++) {
        if (!previous_lines[i] || strcmp(previous_lines[i], new_lines[i]) != 0) {
            printf("Line %d changed: %s", i, new_lines[i]);
        }
    }

    // free old lines and update
    for (int i = 0; i < MAX_CONF_LINES; i++) {
        if (previous_lines[i]) free(previous_lines[i]);
        previous_lines[i] = new_lines[i];  // save current as previous
    }
}


void spawn_watcher(int index) {
    stacks[index] = malloc(THREAD_STACK_SIZE);
    if (!stacks[index]) return;

    int* arg = malloc(sizeof(int));
    *arg = index;
    pid_t tid = clone(
        start_notify,
        stacks[index] + THREAD_STACK_SIZE,
        CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD,
        arg
    );
    if (tid < 0) {
        perror("clone");
        return;
    }

    return;
}

void trackConf(const char* filename) {

    char watchfile[128];
    char command[128];
    int numThreads = 0;

    FILE* conf = fopen(filename,"r");
    if (!conf) {
        perror("Failed to open file");
        return;
    }
    while (fgets(watchfile, 128, conf)) {
        if (fgets(command, 128, conf)) {
            watchfile[strcspn(watchfile, "\n")] = 0;
            command[strcspn(command, "\n")] = 0;
    
            if (numThreads >= MAX_THREADS) {
                fprintf(stderr, "Too many threads! Max is %d\n", MAX_THREADS);
                break;
            }
    
            // Zero out the entire struct to clean up padding
            //memset(&thread_data[numThreads], 0, sizeof(ThreadData));
    
            // Assign values after zeroing
            thread_data[numThreads].filename = strdup(watchfile);
            thread_data[numThreads].command = strdup(command);
    
            spawn_watcher(numThreads);
            numThreads++;
        }
    }
    int fd = inotify_init1(IN_NONBLOCK);
    if (fd < 0) {
        perror("inotify_init");
        return;
    }

    int wd = inotify_add_watch(fd, filename, IN_MODIFY);
    if (wd < 0) {
        perror("Error adding file watcher");
        return;
    }

    printf("Watching %s for changes...\n",filename);
    int i = 0;
    char buffer[BUF_LEN];
    while (1) {
        int length = read(fd, buffer, BUF_LEN);
        if (length < 0) {
            usleep(100000); // sleep for 100ms to reduce CPU usage
            continue;
        }
        i = 0;
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *) &buffer[i];
            if (event->mask & IN_MODIFY) {
                
                compare_and_print_changes(filename);
            }
            i += EVENT_SIZE + event->len;
        }

    }
}

void segfault_handler(int sig) {
    void *array[10];
    size_t size = backtrace(array, 10);
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

int main(int argc, char* argv[]) {
    signal(SIGSEGV, segfault_handler);
    printCWD();

    char* dirName = NULL;
    char* filename = NULL;
    char* command = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i],"-d") == 0 && i + 1 < argc && i + 1 < NAME_MAX) {
            dirName = argv[i+1];
            break; 
        } else if (strcmp(argv[i],"-f") == 0 && i + 1 < argc && i + 1 < NAME_MAX) {
            filename = argv[i+1];
            break; 
        } else if (strcmp(argv[i],"-cmd") == 0 && i + 1 < argc && i + 1 < NAME_MAX) {
            filename = argv[i+1];
            break; 
        } else if (strcmp(argv[i],"-cnf") == 0 && i + 1 < argc && i + 1 < NAME_MAX) {
            filename = argv[i+1];
            trackConf(filename);
        };
    };



    if (dirName == NULL) {
        printf("No specified directory.\n");
    }
    if (filename == NULL) {
        printf("No specified file.\n");
    }
    return 0;
}
