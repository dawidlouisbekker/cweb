#include "endpoints.h"
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

const char* HTTP_METHOD_GET     = "GET";
const char* HTTP_METHOD_POST    = "POST";
const char* HTTP_METHOD_PUT     = "PUT";
const char* HTTP_METHOD_DELETE  = "DELETE";
const char* HTTP_METHOD_PATCH   = "PATCH";
const char* HTTP_METHOD_HEAD    = "HEAD";
const char* HTTP_METHOD_OPTIONS = "OPTIONS";
const char* HTTP_METHOD_TRACE   = "TRACE";
const char* HTTP_METHOD_CONNECT = "CONNECT";

void* memmap;

//MAX_ENDPOINTS



int is_hex_digit(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'A' && c <= 'F') ||
           (c >= 'a' && c <= 'f');
}

int is_unreserved(char c) {
    return (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == '/');
}

int is_valid_url(const char *url) {
    size_t len = strlen(url);
    for (size_t i = 0; i < len; ++i) {
        if (url[i] == '%') {
            // Check that we have two valid hex digits after %
            if (i + 2 >= len || !is_hex_digit(url[i + 1]) || !is_hex_digit(url[i + 2])) {
                return 0; // Invalid percent encoding
            }
            i += 2; // Skip the next two hex digits
        } else if (!is_unreserved(url[i])) {
            return 0; // Invalid character
        }
    }
    return 1;
}


int needs_encoding(char c) {
    // Characters that need to be percent-encoded
    return !(isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || 
             c == '/' || c == ':' || c == '?' || c == '#' || c == '[' || 
             c == ']' || c == '@' || c == '!' || c == '$' || c == '&' || 
             c == '\'' || c == '(' || c == ')' || c == '*' || c == '+' || 
             c == ',' || c == ';' || c == '=');
}

char *percent_encode(const char *input) {
    size_t len = strlen(input);
    char *encoded = malloc(len * 3 + 1);  // Maximum space needed (for percent-encoding)
    size_t j = 0;

    if (encoded == NULL) {
        perror("malloc failed");
        return NULL;
    }

    for (size_t i = 0; i < len; ++i) {
        if (needs_encoding(input[i])) {
            // Encode the character
            sprintf(encoded + j, "%%%02X", (unsigned char)input[i]);
            j += 3;  // 3 characters for percent-encoded character
        } else {
            // Copy the character as it is
            encoded[j++] = input[i];
        }
    }

    encoded[j] = '\0';  // Null-terminate the string
    return encoded;
}

struct endpoint_data* add_endpoint(const char* path) {
    if (endpoint_count >= 100) {
        fprintf(stderr, "Maximum endpoints reached\n");
        return NULL;
    }

    struct endpoint_data* new_ep = &mmap_region[endpoint_count++];
    strncpy(new_ep->path, path, sizeof(new_ep->path));
    new_ep->methods[0] = '\0'; // default
    new_ep->next = NULL;

    return new_ep;
}


void* allocate_memory(size_t size) {
    // Use mmap to allocate a large region of memory
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap failed");
        return NULL;
    }
    return ptr;
}

void create_mmap() {
    
}