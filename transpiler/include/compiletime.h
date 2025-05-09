#include <stdio.h>
#define BLOCK_SIZE 4064

char* calledIDs = NULL;
char* calledFunctions = NULL;
char* definedFunctions = NULL;
char* definedIDs = NULL;


int addToHeap(char* heap, char * newString, int newLen){
    for (int i = 0; i < BLOCK_SIZE; i++) {
        if (heap[i] == '\0' && i + 1 < BLOCK_SIZE) {
            if (heap[i + 1] == '\0') {
                int pos = 0;

                while (newString[pos] != '\0' && i < BLOCK_SIZE)
                {
                    
                }
                
            };
        };
    };
};