#include "main.h"
#include "compiletime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/resource.h>

#define DEBUG_LOG 0
#define NUM_TOKENS 1


//extern int scan_heap(char* heap, int size);

// Function to print text in green

void setGreen() {
    printf("\033[32m");
}

void setBlue() {
    printf("\033[34m");
}

void setYellow() {
    printf("\033[33m");
}

void setRed() {
    printf("\033[31m");
}

void setCyan() {
    printf("\033[36m");
}

void colorReset() {
    printf("\033[0m");
}

void printGreen(const char *text) {
    printf("\033[32m%s\033[0m\n", text);
}

// Function to print text in blue
void printBlue(const char *text) {
    printf("\033[34m%s\033[0m\n", text);
}

// Function to print text in yellow
void printYellow(const char *text) {
    printf("\033[33m%s\033[0m\n", text);
}

// Function to print text in red
void printRed(const char *text) {
    printf("\033[31m%s\033[0m\n", text);
}

void printCyan(const char *text) {
    printf("\033[36m%s\033[0m\n", text);
}
/*
typedef struct HTMLFile {
    const char* content;
};


typedef struct WATFiles {
    const char* content;
}


typedef struct CurrentFiles
{
    struct HTMLFile* htmlFiles;
};
*/

void queryResources(){
    struct rlimit rl;
    // Get the stack size limit
    if (getrlimit(RLIMIT_STACK, &rl) == 0) {
        printf("Stack size: %ld bytes\n", rl.rlim_cur);
    } else {
        perror("getrlimit failed");
    }
}



//Strings require length for wasm.
typedef struct {
    const char* id;
    const char* funcName;
    struct Element* childNodes;
} Element;

typedef struct {
    const char* name;
} WEBCPAGE;

WEBCPAGE* pages = NULL; 

typedef struct {
    FILE* current_file;
    //line,  length, start
    void (*fn)(char*, int,int); // function pointer
} PARSER;

PARSER parser;

typedef enum {
    WRITE_TO_CRNT,
    PARSE_CODE,
    ANALYSE_FURTHER,
} Parse_Mode;

Parse_Mode crntRetParse = ANALYSE_FURTHER;





int isatoz(char ch){
    if (ch > 96 && ch < 123) {
        return 1;
    } 
    return 0;
};

void to_lower_case_string(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = tolower(str[i]);  // Convert each character to lowercase
    }
}

// Function to check if the parsed element is a valid HTML element
int is_html_element(const char* element) {
    switch (element[0]) {
        case 'a': if (strcmp(element, "a") == 0) return 1; break;
        case 'b': if (strcmp(element, "b") == 0) return 1; break;
        case 'd': if (strcmp(element, "div") == 0) return 1; break;
        case 'h': 
            if (strcmp(element, "h1") == 0 || strcmp(element, "h2") == 0 || 
                strcmp(element, "h3") == 0 || strcmp(element, "h4") == 0 || 
                strcmp(element, "h5") == 0 || strcmp(element, "h6") == 0) return 1; break;
        case 'p': if (strcmp(element, "p") == 0) return 1; break;
        case 'u': if (strcmp(element, "ul") == 0 || strcmp(element, "li") == 0) return 1; break;
        case 'i': if (strcmp(element, "img") == 0) return 1; break;
        case 's': if (strcmp(element, "span") == 0) return 1; break;
        case 't': if (strcmp(element, "table") == 0 || strcmp(element, "tr") == 0 || strcmp(element, "td") == 0) return 1; break;
        case 'f': if (strcmp(element, "form") == 0) return 1; break;
        // Add more HTML elements as needed
    }
    return 0;
}


typedef enum {
    ELM,
    PROPS,
    PROPVALUE,
    PROPVALUESTART,
    EVENTHANDLER,
    ANALYSE_HTML,
} HMTL_MODE;
/// Returns for changing parsing mode
const char return_end[] = { ')', ';' , '}' };

//On is for on<Event Handler>;
const char* htmlprops[] = {
    "class", "style", "title", "data-", "lang", "dir", "accesskey", "on", 
    "id", "name", "value", "type", "placeholder", "required", "checked", "disabled",
    "readonly", "action", "method", "href", "src", "alt", "target", "rel", "colspan", 
    "rowspan", "maxlength", "minlength", "step", "pattern", "rows", "cols", "autoplay",
    "loop", "muted", "controls", "aria-label", "aria-hidden", "role"
};
//Length of props
const size_t length[] = {
    5, 5, 5, 5, 4, 3, 9, 2,
    2, 4, 5, 4, 10, 8, 7, 8,
    6, 3, 3, 4, 3, 3, 6, 5,
    3, 3, 3, 3, 4, 5, 4, 4, 
    3, 3, 6, 9, 4, 4           
};

// Updated function to handle more props
int getPropType(const char* text, int maxLen) {
    for (int i = 0; i < sizeof(htmlprops) / sizeof(htmlprops[0]); i++) {
        // Compare the length of the substring against the known length
        if (maxLen < length[i]) {
            continue;
        }

        // Compare the first 'length[i]' characters from 'text' with the current property
        if (strncmp(htmlprops[i], text, length[i]) == 0) {
            printf("prop: ");
            setCyan();
            printf("%s\n", htmlprops[i]);
            colorReset();
            if (htmlprops[i] == "on") {
                printf("Event handler\n");
            };
            // Add to node structure
            return 1;
        }
    }
    return 0;
}


void parse_html_line(const char* line, int len) {
    char token[1024];
    HMTL_MODE mode = ANALYSE_HTML;
    int pos = 0;
    for (int i = 0; i < len; i++) {
        switch (mode)
        {
        case ELM:
            if (isatoz(line[i])){
                token[pos] = line[i];
                if (is_html_element(token)) {
                    printf("Element: ");
                    setYellow();
                    printf("%s\n",token);
                    colorReset();
                    mode = PROPS;
                    printGreen("Adding to Node.");
                }
                pos++;
            }
            break;
            case PROPS: {
                if (isalpha(line[i])) {
                    // Pass the memory address of the line pointer and the remaining length
                    if (getPropType(line + i, len - i) != 0) {
                        mode = PROPVALUESTART;
                    }
                }

                if (line[i] == '>') {
                    mode = ANALYSE_HTML;
                }
                break;
            }
            case PROPVALUESTART: {
                if (line[i] == '"') {
                    mode = PROPVALUE;
                    pos = 0;
                };
                break;
            }
            case PROPVALUE: {
                if (line[i] != '"') {
                    token[pos] = line[i];
                    pos++;
                } else {
                    printf("Prop Value: ");
                    setCyan();
                    printf("%s\n",token);
                    colorReset();
                    mode=PROPS;
                    pos=0;
                }
                break;
            }
        case ANALYSE_HTML:
            switch (line[i])
            {
            case '<':
                mode = ELM;
                pos = 0;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        };
    }
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
                parse_html_line(line,len);
                fputs(line, parser.current_file);
                fflush(parser.current_file);
            }

            break;
        case ANALYSE_FURTHER:
            
            if (strchr(line, '(')) {
                if (DEBUG_LOG) {
                    printf("\n Writing html\n");
                }
                crntRetParse = WRITE_TO_CRNT;
            }
            break;
    }

}



const char* tokens[NUM_TOKENS] = { "return" };
const int lengths[] = { 6 };
void (*operations[])(char*, int, int) = { tokenize_return };

int addPath(char* path) {
    
    FILE *file = fopen(path, "r");
    if (!file) {
        perror("Error opening source file");
        printf("%s\n", path);
        return 1;
    }

    // Replace "./src/" with "./build/"
    const char *srcPrefix = "../src/";
    const char *buildPrefix = "../build/";
    char buildPath[1024];
    if (strncmp(path, srcPrefix, strlen(srcPrefix)) == 0) {
        snprintf(buildPath, sizeof(buildPath), "%s%s", buildPrefix, path + strlen(srcPrefix));
    } else {
        fprintf(stderr, "Path doesn't start with ../src/: %s\n", path);
        fclose(file);
        return 1;
    }
    printf("Transpiling: %s\n",buildPath);
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
                printf("line: %s \n",line);
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
                printf("Writing html line %s\n",line);
            }
            parser.fn(line, strlen(line), 0);;
        }

    };

    fclose(file);
    return EXIT_SUCCESS;
};

void listFilesRecursively(const char *basePath) {
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    // Unable to open directory
    if (!dir) {
        printf("Failed to open directory: %s\n", basePath);
        return;
    }

    while ((dp = readdir(dir)) != NULL) {
        // Skip the special entries "." and ".."
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", basePath, dp->d_name);
        struct stat statbuf;
        if (stat(path, &statbuf) == 0) {
            // Recursively call for directories
            if (S_ISDIR(statbuf.st_mode)) {
                listFilesRecursively(path);
            } else {
                const char *ext = strrchr(dp->d_name, '.');
                if (ext != NULL && strcmp(ext, ".cweb") == 0) {
                    printf("======= %s =======\n",dp->d_name);
                    char real_path[4096];
                    snprintf(real_path, sizeof(real_path), "%s/%s",basePath,dp->d_name);
                    addPath(real_path);
                    printf("=================\n");
                } else {
                    printf("/%s\n", dp->d_name);
                }
            }

        }
    }
    closedir(dir);
}


void qryStructures() {
   // printf("CacheLineBlockSize: %d\n",sizeof(CacheLineBlock));
}

void allocateEventhandlerHeap(){
    
    char* calledIDs = (char *)malloc(BLOCK_SIZE);
    char* calledFunctions = (char *)malloc(BLOCK_SIZE);
    char* definedFunctions = (char *)malloc(BLOCK_SIZE);
    char* definedIDs = (char *)malloc(BLOCK_SIZE);
    //printf("scanning heap\n");
};

void dealocateEventHandlerHeap(){
    free(calledIDs);
    free(calledFunctions);
    free(definedFunctions);
    free(definedIDs);
};

/*
int main(int argc, char* argv[]) {
    queryResources();
    allocateEventhandlerHeap();
    //qryStructures();
    char* dirName = NULL;
    // Check if the -d flag is provided
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            dirName = argv[i + 1]; // Get the directory name after -d
            break;
        }
    }

    if (dirName) {
        printf("Directory specified: %s\n", dirName);
        listFilesRecursively(dirName);
    } else {
        printf("Usage: %s -d <directory_name>\n", argv[0]);
        printf("Using default ../src\n");
        dirName = "../src";
    }
    listFilesRecursively(dirName);
    dealocateEventHandlerHeap();
    return 0;
}*/

