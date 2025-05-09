#define _POSIX_C_SOURCE 199309L
#include "base/endpoints.h"
#include <signal.h>


void handle_sigint(int sig) {
    printf("\nReceived SIGINT. Shutting down...\n");
    exit(1);
};


void get_unique_ascii() {
    int ascii[MAX_ASCII_PLUS_ONE];
    for (int i = 0; i < MAX_ASCII_PLUS_ONE; i++) ascii[i] = i;

    srand(time(NULL));  // Seed once

    // Fisher-Yates shuffle
    for (int i = 127; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = ascii[i];
        ascii[i] = ascii[j];
        ascii[j] = temp;
    };

    printf("[");
    for (int i = 0; i < MAX_ASCII_PLUS_ONE; i++) {
        num_with_start_ascii[i] = ascii[i];
        //printf("ASCII %d: %c\n", ascii[i], ascii[i]);
        printf("%d,",ascii[i]);
    };
    printf("]\n");
}



int main(){
    signal(SIGINT, handle_sigint);
    printf("char %d\n", (int)sizeof(char));
    printf("int %d\n",(int)sizeof(int));
    printf("size_t %d\n",(int)sizeof(size_t));
    int result;
    if ((result = load_endpoints()) > 0) {
        printf("CREATING STACK\n");
        create_stack();
        //print_raw_stack();
        print_stack();
        print_endp_memadrr();
    };
    char request[MAX_LINE_SIZE];
    uint8_t* addr;
    char* url;
    size_t* len;
    size_t req_len;
    int trav_nodes = -1;
    long seconds;
    long nanoseconds;
    struct timespec start, end;
    while (1)
    {
start:
        trav_nodes = -1;
        seconds = 0;
        nanoseconds = 0;
        
        printf("\nEnter url: ");
        if (fgets(request, MAX_LINE_SIZE -1, stdin) > 0) {
            clock_gettime(CLOCK_MONOTONIC, &start);  // Start timing
            
    
            request[strlen(request) - 1] = '\0';
            //printf("Request: %s\n",request);
            if (endp_ptrs[(size_t)request[1]] != NULL && (num_with_start_ascii[(size_t)request[1]] !=0)) {
                addr = endp_ptrs[(size_t)request[1]]; 
                do 
                 {
loopstart:
                    trav_nodes++;
                    if (num_with_start_ascii[(size_t)request[1]] <= trav_nodes) {
                        printf("%s 404 NOT FOUND %s %s\n",color_codes[COLOR_RED],request,color_codes[COLOR_RESET]);
                        clock_gettime(CLOCK_MONOTONIC, &end);
                        seconds = end.tv_sec - start.tv_sec;
                        nanoseconds = end.tv_nsec - start.tv_nsec;
                        if (nanoseconds < 0) {
                            seconds--;
                            nanoseconds += 1e9;
                        }
                
                        printf("Time taken: %ld.%09ld seconds\n", seconds, nanoseconds);
                        goto start;
                    };
                    len = (size_t *)addr;
                    addr += sizeof(size_t *);
                    url = (char *)addr;
                    req_len = strlen(&request[2]);
                    printf("store len: %ld\n",*len);
                    printf("req len: %ld\n",req_len);
                    printf("address: %p\n",addr);
                    
                    if (*len != req_len) {
                        addr += *len + 8;
                        len = (size_t *)addr;
                        printf("len: %ld\n",*len);
                        printf("address: %p\n",addr);
                        goto loopstart;
                    };
                    
                } while (memcmp(&request[2], url, *len) != 0);

                printf("%sACCEPT%s: %s\n",color_codes[COLOR_GREEN],color_codes[COLOR_RESET],request);
                clock_gettime(CLOCK_MONOTONIC, &end);

                seconds = end.tv_sec - start.tv_sec;
                nanoseconds = end.tv_nsec - start.tv_nsec;
                if (nanoseconds < 0) {
                    seconds--;
                    nanoseconds += 1e9;
                }
        
                printf("Time taken: %ld.%09ld seconds\n", seconds, nanoseconds);
                goto start;

            } else {
                printf("%s 404 NOT FOUND %s %s\n",color_codes[COLOR_RED],request,color_codes[COLOR_RESET]);
            };

    
            clock_gettime(CLOCK_MONOTONIC, &end);  // End timing
    
            long seconds = end.tv_sec - start.tv_sec;
            long nanoseconds = end.tv_nsec - start.tv_nsec;
            if (nanoseconds < 0) {
                seconds--;
                nanoseconds += 1e9;
            }
    
            printf("Time taken: %ld.%09ld seconds\n", seconds, nanoseconds);
        }
        
    }
    
    //int result = get_unique_sequnce_numbers();
    //get_unique_ascii();
    return 1;
};