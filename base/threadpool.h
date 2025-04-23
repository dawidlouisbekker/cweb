#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>

#define STACK_SIZE (1024 * 1024)
#define THREAD_COUNT 4
#define CORE_COUNT 4

int createThread(int core) {
    void* stack = malloc(STACK_SIZE);
    if (!stack) {
        perror("malloc");
        exit(1);
    }

    // clone() flags: CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD
    int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;

    if (clone(thread_fn, stack + STACK_SIZE, flags, &core) == -1) {
        perror("clone");
        return -1;
    }

    // Optionally wait or keep main alive
    while (1); // Hold main thread

    return 0;
}

int thread_fn(void* arg) {
    int core_id = *(int*)arg;

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    pid_t tid = syscall(SYS_gettid);  // get thread ID for affinity

    if (sched_setaffinity(tid, sizeof(cpu_set_t), &cpuset) != 0) {
        perror("sched_setaffinity");
        exit(1);
    }

    printf("Thread running on core %d\n", core_id);

    // Do actual work here...
    while (1); // Just loop to hold CPU

    return 0;
}