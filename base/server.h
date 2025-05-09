#define _GNU_SOURCE
#include "serverconf.h"
#include "parser.h"

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include <stdio.h>
#include <string.h>




#define PORT 8082
#define THREAD_COUNT 8
#define BACKLOG ((SOMAXCONN / THREAD_COUNT))


#define DEBUG_LOG 0

#define QUEUE_DEPTH 64
#define BUFFER_SIZE 4096

#define CLIENT_HANDLER_THREADS 6
#define THREAD_STACK_SIZE 4096

int server_fds[THREAD_COUNT];

const int PATH_SIZE = 1024;

int allocatedStack = 0;

char* STACKS[THREAD_COUNT];
int thread_ids[THREAD_COUNT];

__attribute__((destructor))
void my_destructor() {
    printf("Cleaning up before shutdown...\n");
    for (int i = 0; i < THREAD_COUNT; i++) {
        /*
        if (STACKS[i]) {
            free(STACKS[i]);
            STACKS[i] = NULL;
            printf("Freed thread %d stack memory\n", i);
        };*/

        if (server_fds[i] != -1) {
            close(server_fds[i]);
            printf("Closed thread %d socket.\n",i);
        };
    }
    return;
};

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
};

void parse_http_request(char *request, int client_socket) {
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
    };
    char first_line[2048];
    strncpy(first_line, request, line_length);
    first_line[line_length] = '\0';

    if (sscanf(first_line, "%7s %1023s", method, path) != 2) {
        printf("Malformed HTTP request line\n");
        return;
    }
    handleEndpoint(path, client_socket);
    printf("HTTP Method: %s\n", method);
    printf("Endpoint: %s\n", path);
};


extern void parse_header(char* header);

int server_thread(void* arg) {
    int thread_id = *(int*)arg;

    server_fds[thread_id] = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fds[thread_id] == -1) {
        perror("socket failed");
        pthread_exit(NULL);
        return -1;
    };

    int opt = 1;
    if (setsockopt(server_fds[thread_id], SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEPORT) failed");
        close(server_fds[thread_id]);
        server_fds[thread_id] = -1;
        return -1;
    };

    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(PORT)
    };
    //printf("THREAD %d: BINDING.\n",thread_id);
    if (bind(server_fds[thread_id], (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fds[thread_id]);
        server_fds[thread_id] = -1;
        return -1;
    }

    if (listen(server_fds[thread_id], BACKLOG) < 0) {
        perror("listen failed");
        close(server_fds[thread_id]);
        server_fds[thread_id] = -1;
        return -1;
    }
    //printf("THREAD %d: lISTENING.\n",thread_id);
    //printf("Thread %d listening on port %d\n", thread_id, 8082);
    int client_fd = -1;
    char ip_str[INET_ADDRSTRLEN];
    char buffer[THREAD_STACK_SIZE / 2];
    int bytes_received = -1;
    const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 11\r\n\r\nHello World";
    //char* line;
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        //printf("THREAD %d: GOING TO ACCEPT.\n", thread_id);
        client_fd = accept(server_fds[thread_id], (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        };
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
        //printf("Thread %d accepted connection from %s:%d\n", thread_id, ip_str, ntohs(client_addr.sin_port));
        //printf("CONNECTED\n");
        while ((bytes_received = recv(client_fd, buffer, (THREAD_STACK_SIZE / 2) - 1, 0)) > 0) {
            inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
            printf("Thread %d accepted connection from %s:%d\n", thread_id, ip_str, ntohs(client_addr.sin_port));
        
            buffer[bytes_received] = '\0';  // Null-terminate the buffer
            //parse_header(buffer);
            printf("Thread %d received message of %d bytes: \n%s\n", thread_id, bytes_received, buffer);
            //char* line = strtok(buffer, "\r\n");  // Split on \r\n
           /// while (line != NULL) {
            //    printf("Line: %s\n", line);
            //    line = strtok(NULL, "\r\n");  // Next line
           // }
            //
            send(client_fd, response, strlen(response), 0);
        }
        close(client_fd);  // Always close the client socket after handling it
    };
    server_fds[thread_id] = -1;
    close(server_fds[thread_id]);
    return 0;
}

//ST = Single Thread
void run() {
    for (int i = 0; i < THREAD_COUNT; ++i) {
        // Allocate stack for each thread
        STACKS[i] = (char *)malloc(THREAD_STACK_SIZE);
        if (!STACKS[i]) {
            perror("Failed to allocate stack");
            exit(1);
        };
        server_fds[i] = -1;
        int* id = malloc(sizeof(int));
        *id = i;
        if (clone(server_thread,
                  STACKS[i] + THREAD_STACK_SIZE,  // Stack grows downward
                  CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD,
                  id) == -1) {
            perror("clone failed");
            exit(1);
        }
    }
    // Wait forever (threads are within the same process)
    while (1) pause();
};

//ST = Single Thread
void startServer(int server_fd, struct sockaddr_in* address_ptr, int* addrlen_ptr, int verbose) {
    int new_socket;
    char buffer[BUFFER_SIZE] = {0};
    HeaderScanResult result;
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)address_ptr, (socklen_t*)addrlen_ptr)) < 0) {
            perror("Accept failed");
            continue;
        }
        int bytes_read = read(new_socket, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';  // Null-terminate the string
            //if (verbose == 1) {
            printf("----------\n%s",buffer);
            result = fast_header_scan(buffer, new_socket);
            printf("%d",result.has_heavy_headers);
            printf("Received request.");
            parse_http_request(buffer,new_socket);
        }

        // Optionally send a response
        const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
        send(new_socket, response, strlen(response), 0);
        close(new_socket);
    }
}

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
    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", PORT);

    // 4. Accept and handle connection
    startServer(server_fd,&address,&addrlen,verbose);
    
    return;
}
