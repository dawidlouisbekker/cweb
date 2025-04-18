#include "base/serverconf.h"
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#define PORT 8081
#define BUFFER_SIZE 4096
#define DEBUG_LOG 0
const int PATH_SIZE = 1024;

const char* favCon = "/favicon.ico";
int equal_str(const char * str1,const char * str2, size_t MAX_LEN) {
    return strncmp(str1, str2, MAX_LEN);
};


void handleEndpoint(char* path, int client_socket) {
    printf("Handling %s\n", path);

    char real_path[4096];  // Large enough buffer
    snprintf(real_path, sizeof(real_path), "./build%s", path);
    
    if (DEBUG_LOG) {
        printf("Real Path: %s\n", real_path);
    }
    const char *ext = strrchr(path, '.');
    const char *content_type = "application/octet-stream";
    if (ext) {
        if (strcmp(ext, ".wasm") == 0) wasm_response(client_socket, real_path);
    }

    if (strncmp(favCon, path, PATH_SIZE) == 0) {
        printf("Found image\n");
        image_response(client_socket, real_path);
    } else {
        html_response(client_socket, real_path);
        printf("Images not found\n");
    }
}

void parse_http_request(const char *request, int client_socket) {
    char method[8] = {0};
    char path[1024] = {0};

    // Find the first line (before the first \r\n)
    const char *line_end = strstr(request, "\r\n");
    if (!line_end) {
        printf("Invalid HTTP request: no CRLF\n");
        return;
    }

    // Copy the first line into a temporary buffer
    size_t line_length = line_end - request;
    if (line_length >= 2048) {
        printf("First line too long\n");
        return;
    }

    char first_line[2048];
    strncpy(first_line, request, line_length);
    first_line[line_length] = '\0';

    // Now safely parse method and path using sscanf
    if (sscanf(first_line, "%7s %1023s", method, path) != 2) {

        printf("Malformed HTTP request line\n");
        return;
    }
    handleEndpoint(path, client_socket);
    printf("HTTP Method: %s\n", method);
    printf("Endpoint: %s\n", path);
};

//ST = Single Thread
void startSTServer(int server_fd, struct sockaddr_in* address_ptr, int* addrlen_ptr, int verbose) {
    int new_socket;
    char buffer[BUFFER_SIZE] = {0};
    while (1) {

        if ((new_socket = accept(server_fd, (struct sockaddr *)address_ptr, (socklen_t*)addrlen_ptr)) < 0) {
            perror("Accept failed");
            continue;
        }

        int bytes_read = read(new_socket, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';  // Null-terminate the string
            //if (verbose == 1) {
                //printf("raw: %s",buffer);
            //}
            printf("Received request.");
            parse_http_request(buffer,new_socket);
        }

        // Optionally send a response
        const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
        send(new_socket, response, strlen(response), 0);
        close(new_socket);
    }
}
//ST = Single Thread
void runST(int verbose) {
    
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    

    // 1. Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Bind to port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, addrlen) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Listen
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", PORT);

    // 4. Accept and handle connection
    startSTServer(server_fd,&address,&addrlen,verbose);
    close(server_fd);
    return;
}
