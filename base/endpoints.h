#pragma once
#include <string.h>
#include "utils/colors.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/*
    TODO:

    Null terminate the endpoints in the endpoint_stack. Make them as short as possible to be uniquely identified.
*/

extern void endp_qry(uint8_t* stack, char* qry);

#define MAX_LINE_SIZE 64
#define MAX_ENDPOINTS 100

//127 possible ascii values
#define MAX_ASCII 127
#define MAX_ASCII_PLUS_ONE ((MAX_ASCII + 1))

size_t num_endpoints = 0;


//index at MAX_ASCII points to the starting ascii char's decimal value so only 'pi' in '/api' is at index MAX_ENDPOINTS .
char* endpoints[MAX_ASCII][MAX_ENDPOINTS];

size_t num_with_start_ascii[MAX_ASCII_PLUS_ONE];

int clear_endpoint_buffer = 0;

size_t num_endp_ptrs = 0;
uint8_t* endp_ptrs[MAX_ASCII_PLUS_ONE];

size_t endpoint_stack_size = -1;
void* endpoint_stack;


typedef enum {
    READ_ENDPOINT = 0,
    READ_METHODS = 1,
    READ_REQUIRED = 2,
    EXIT = 3,
} MODE;


char* endpoint_function(char *) {
    return "Hello";
};


void clear_endpointbuffer(){
    if (clear_endpoint_buffer == 1) {
        for (size_t ascii = 0; ascii < MAX_ASCII_PLUS_ONE; ascii++) {
            //printf("Moving to %c\n",(int)ascii);
            for (size_t i = 0; i < num_with_start_ascii[ascii]; i++) {
                if (endpoints[ascii][i] != NULL) {
                    printf("Freeing endpoint[%ld][%ld]: %s\n", ascii, i, endpoints[ascii][i]);
                    free(endpoints[ascii][i]);
                    endpoints[ascii][i] = NULL;  // Good practice
                };
            };
        };
        clear_endpoint_buffer = 0;
    } else {
        printf("Endpoint Buffer already cleared or was never created.\n");
    };
    return;
};


void __attribute__((destructor)) cleanup() {
    printf("\n%sCLEARING RESOURCES%s\n",color_codes[COLOR_YELLOW], color_codes[COLOR_RESET]);
    if (endpoint_stack_size != -1 && endpoint_stack != NULL) {
        free(endpoint_stack);
        endpoint_stack = NULL;
        printf("%sCLEARED STACK%s\n",color_codes[COLOR_GREEN], color_codes[COLOR_RESET]);
    };
    //if (endp_ptrs != NULL) {
    //    free(endp_ptrs);
    //    printf("%sCLEARED ENDPOINT POINTERS%s\n",color_codes[COLOR_GREEN], color_codes[COLOR_RESET]);

    //}

    clear_endpointbuffer();
    printf("%sFINISHED.\nEXITING...%s\n",color_codes[COLOR_YELLOW], color_codes[COLOR_RESET]);
};

size_t calculate_stack_size() {
    size_t total_size = 0;
    for (size_t i = 0; i < MAX_ASCII_PLUS_ONE; i++) {
        for (size_t j = 0; j < num_with_start_ascii[i]; j++) {
            size_t len = strlen(endpoints[i][j]);
            total_size += sizeof(size_t); // for length
            total_size += len;            // for string
            total_size += sizeof(void*);  // for function pointer
        }
    }
    return total_size;
};

void print_raw_stack() {
    char* ptr = (char*)endpoint_stack;
    printf("Endpoint stack size: %ld\n", endpoint_stack_size);
    char* end = ptr + endpoint_stack_size;

    size_t index = 0;
    while (ptr < end) {
        unsigned char byte = (unsigned char)*ptr;

        // Print printable ASCII or dot
        if (byte >= 32 && byte <= 126) {
            printf("%c", byte);
        } else {
            printf("\n%02zx: %02x '.'\n", index, byte);
        }

        ptr++;
        index++;
    }
}

void print_endp_memadrr() {
    for (int i = 0; i < MAX_ASCII_PLUS_ONE; i++) {
        if (endp_ptrs[i]) { // Optional: only print non-null pointers
            printf("%d '%c' : %p \n", i,(char)i, endp_ptrs[i]);
        }
    }
}

void print_stack() {
    char* ptr = (char*)endpoint_stack;
    printf("--- PRINTING STACK ---\n");
    printf("start Ptr: %p\n", ptr);
    printf("Endpoint stack size: %ld\n",endpoint_stack_size);
    char* end = ptr + endpoint_stack_size;

    size_t entry_count = 0;
    while (ptr < end) {
        // 1. Read length
        size_t len;
        memcpy(&len, ptr, sizeof(size_t));
        ptr += sizeof(size_t);
        //printf("ptr: %p\n",ptr);
        // 2. Read string
        char buffer[MAX_LINE_SIZE]; // Assuming strings < 256 chars
        if (len >= sizeof(buffer)) len = sizeof(buffer);
        memcpy(buffer, ptr, len);
        buffer[len] = '\0';
        ptr += len;

        // 3. Read function pointer
        void* func_ptr;
        memcpy(&func_ptr, ptr, sizeof(void*));
        ptr += sizeof(void*);

        // Print decoded entry
        printf("Entry %zu: \"%s\",\nFunction Ptr: %p\n", entry_count++, buffer, func_ptr);
    };
};

void create_stack(){
    endpoint_stack_size = calculate_stack_size();
    if (endpoint_function <= 0) {
        perror("Stack size");
        clear_endpointbuffer();
    };
    printf("REQUIRED STACK SIZE: %ld.\n",endpoint_stack_size);

    endpoint_stack = malloc(endpoint_stack_size); // Or manually manage a buffer


    char* ptr = (char*)endpoint_stack;

    printf("start Ptr: %p\n", ptr);

    for (size_t i = 0; i < MAX_ASCII_PLUS_ONE; i++) {
        for (size_t j = 0; j < num_with_start_ascii[i]; j++) {
            if (j == 0) {
                endp_ptrs[i] = (uint8_t*)ptr;
                num_endp_ptrs++;
            };
            size_t len = strlen(endpoints[i][j]);
            printf("len: %ld\n",len);
            memcpy(ptr, &len, sizeof(size_t));
            ptr += sizeof(size_t);
            //printf("ptr: %p\n",ptr);

            const char* str = endpoints[i][j];
            printf("Entry: '%s'\n",str);
            memcpy(ptr, str, len); // store "hello"
            ptr += len;
            char* (*my_func)(char*) = endpoint_function; //This will by created by the user.
            printf("Func size: %ld\n", sizeof(my_func));
            memcpy(ptr, &my_func, sizeof(my_func));
            ptr += sizeof(my_func);
          /*  if (strcmp("", endpoints[i][j]) == 0) {  // strcmp returns 0 when strings are equal
                printf("Memory address of endpoints[%ld][%ld]: %p\n", i, j, my_func);
            }
            if (strcmp("pi", endpoints[i][j]) == 0) {  // strcmp returns 0 when strings are equal
                printf("Memory address of endpoints[%ld][%ld]: %p\n", i, j, my_func);
            } 
            */
        };
    };
    return;
};
    /*
    char* ptr = (char*)endpoint_stack;
    size_t len = 5;
    memcpy(ptr, &len, sizeof(size_t));
    ptr += sizeof(size_t);
    
    const char* str = "hello";
    memcpy(ptr, str, len); // store "hello"
    ptr += len;
    char* (*my_func)(char*) = endpoint_function; //This will by created by the user.
    memcpy(ptr, &my_func, sizeof(void*));
    */


int get_unique_sequnce_numbers() {
    for (int i = 0; i < MAX_ASCII_PLUS_ONE; i++){
        srand(time(NULL));              // Seed the random number generator
        int rand_ascii = rand() % 128;  // Generate number between 0 and 127
        printf("Random ASCII value: %d\n", rand_ascii);
        printf("Character: %c\n", rand_ascii);
        num_with_start_ascii[i] = rand_ascii;
    };
};


void process_endpoints(){

};  

int load_endpoints() {
    
    for (size_t i = 0; i < MAX_ASCII_PLUS_ONE; i++){
        num_with_start_ascii[i] = 0;
        endp_ptrs[i] = NULL;
    };

    MODE mode = READ_ENDPOINT;
    FILE* conf = fopen("./cweb.conf", "r");
    if (!conf) {
        perror("Error opening file");
        return -1;
    }

    int ch;
    char* read_result = { "" };
    char line[MAX_LINE_SIZE];
    size_t len = 0;
    while (mode != EXIT && read_result > 0)
    {
        switch (mode)
        {
            case READ_ENDPOINT:
                if ((read_result = fgets(line,MAX_LINE_SIZE - 1,conf)) > 0) {
                    if (line[0] == '/') {
                        len = strlen(line);
                        
                        line[len - 1] = '\0'; 
                        if (line[len - 2] == ' ') {
                            printf("%s",color_codes[COLOR_RED]);
                            printf("WARNING: ");
                            printf("%s",color_codes[COLOR_RESET]);
                            printf("'%s' has space at the end.\n",line);
                        };
                        printf("Adding endpoint: '%s' \n",line);
                        //endpoints['e'][0] = &line[2]; // line is stack-allocated
                        //strdup uses malloc internally so it must be freed.

                        endpoints[line[1]][num_with_start_ascii[line[1]]] = strdup(&line[2]);
                        if (clear_endpoint_buffer == 0) {
                            clear_endpoint_buffer = 1;
                        };
                        num_endpoints++;
                        num_with_start_ascii[line[1]]++;
                        mode = READ_METHODS;
                    };
                } else {
                    mode = EXIT;
                }
                break;
            case READ_METHODS:
                if ((read_result = fgets(line,MAX_LINE_SIZE - 1,conf)) > 0) {
                    if (line[0] == '}') {
                        mode = READ_ENDPOINT;
                    } else if (line[0] == '{')
                    {
                        break;
                    };

                } else {
                    mode = EXIT;
                }
                break;
            default:
                break;
        };
    }
    
    printf("Total endpoints: %ld\n",num_endpoints);
    /*
    while ((ch = fgetc(conf)) != EOF) {
        switch (ch)
        {
        case '\n':
            break;
        
        default:
            break;
        }
        putchar(ch);
    };*/

    fclose(conf);
    return 1;
}


//0x555aa5060898


//0x555aa50608a8

//0x555aa50608a0