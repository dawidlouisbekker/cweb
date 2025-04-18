#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#define DEBUG_LOG 0
#define NUM_TOKENS 1



typedef struct HTMLFile {
    const char* content;
};

/*
typedef struct WATFiles {
    const char* content;
}
*/

typedef struct CurrentFiles
{
    struct HTMLFile* htmlFiles;
};

//Strings require length for wasm.
typedef struct Element {
    const char* id;
    const char* funcName;
    struct Element* childNodes;
};

typedef struct PARSER {
    FILE* current_file;
    //line,  length, start
    void (*fn)(char*, int,int); // function pointer
} PARSER;
PARSER parser;

typedef enum {
    WRITE_TO_CRNT,
    ANALYSE_FURTHER,
} Parse_Mode;

Parse_Mode crntRetParse = ANALYSE_FURTHER;


const char return_end[] = { ')', ';' , '}' };



void parse_html_line(const char* line, int len) {
    char element[1024];
    int elm = 0;

    int pos = 0;
    for (int i = 0; i < len; i++) {
        if (!elm) {
            if (line[i] == '<') {
                elm = 1;
            }
            
        } else {
            if (line[i] == '>') {
                elm = 0;
                element[pos] = '\0';
                printf("Element Name:%s",element);
                return;
            }
        }
        element[pos] = line[i];
        pos++;

    };
};

void tokenize_return(char* line, int len, int start) {
    switch (crntRetParse) {
        case WRITE_TO_CRNT:
            // Look for end of return block
            if (strstr(line, ");")) {
                crntRetParse = ANALYSE_FURTHER;
                parser.fn = NULL; // Reset the function pointer
                return;
            }

            if (parser.current_file) {
                fputs(line, parser.current_file);
                fflush(parser.current_file);
            }

            break;
        case ANALYSE_FURTHER:
            
            if (strchr(line, '(')) {
                if (DEBUG_LOG) {
                    printf("\n Writing html\n");
                }
                parse_html_line(line,len);
                crntRetParse = WRITE_TO_CRNT;
            }
            break;
    }
}



const char* tokens[NUM_TOKENS] = { "return" };
const int lengths[] = { 6 };
void (*operations[])(char*, int, int) = { tokenize_return };

int addPath(char* path) {
    printf("\n\n--- Transpiling ---\n");
    printf("%s\n\n", path);
    FILE *file = fopen(path, "r");
    if (!file) {
        perror("Error opening source file");
        return 1;
    }

    // Replace "./src/" with "./build/"
    const char *srcPrefix = "./src/";
    const char *buildPrefix = "./build/";
    char buildPath[1024];

    if (strncmp(path, srcPrefix, strlen(srcPrefix)) == 0) {
        snprintf(buildPath, sizeof(buildPath), "%s%s", buildPrefix, path + strlen(srcPrefix));
    } else {
        fprintf(stderr, "Path doesn't start with ./src/: %s\n", path);
        fclose(file);
        return 1;
    }

    FILE *buildFile = fopen(buildPath, "w");
    parser.current_file = buildFile;

    if (!buildFile) {
        perror("Error opening build file");
        fclose(file);
        return 1;
    }

    char line[1024]; // buffer to hold each line, adjust size as needed
    while (fgets(line, sizeof(line), file)) {
        if (parser.fn == NULL) {
            char token[512] = "";
            int tknPos = 0;
            if (DEBUG_LOG) {
                printf("line: %s \n\n",line);
            }
            for (int i = 0; i < sizeof(line); i++ ) {
                token[i] = line[i];
                token[i+1] = '\0';
                int found = 0;
                switch (line[i])
                {
                    case ' ':
                        for (int j = 0; j < NUM_TOKENS; j++) {
                            if (strncmp(token, tokens[j], lengths[j]) == 0) {
                                parser.fn = operations[j];
                                parser.fn(line, strlen(line), tknPos);
                                found = 1;
                                break;
                            }
                        }
                        /* code */
                        break;
                    case '\n':
                    default:
                        break;
                }
                if (found) {
                    break;
                }
                
            };
        } else {
            if (DEBUG_LOG) {
                printf("\n Writing html line %s\n",line);
            }
            parser.fn(line, strlen(line), 0);;
        }

    };

    fclose(file);
    return EXIT_SUCCESS;
};
