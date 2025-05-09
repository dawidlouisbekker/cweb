#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "base/utils/colors.h"

#define NUM_THREADS 10
#define REQUESTS_PER_THREAD 100
#define HOST "127.0.0.1"
#define PORT 8082

void *send_requests(void *arg) {
    for (int i = 0; i < REQUESTS_PER_THREAD; i++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("Socket creation failed");
            continue;
        }

        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        int status = getaddrinfo(HOST, NULL, &hints, &res);
        if (status != 0) {
            fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
            close(sock);
            continue;
        }

        struct sockaddr_in *server_addr = (struct sockaddr_in *)res->ai_addr;
        server_addr->sin_port = htons(PORT);

        if (connect(sock, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0) {
            perror("Connection failed");
            freeaddrinfo(res);  // Don't forget to free addrinfo struct
            close(sock);
            continue;
        }

        const char *request = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
        if (send(sock, request, strlen(request), 0) < 0) {
            perror("Send failed");
        }

        // Optionally receive the response
        // char buffer[1024];
        // while (recv(sock, buffer, sizeof(buffer), 0) > 0);

        freeaddrinfo(res);
        close(sock);
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    struct timeval start, end;

    gettimeofday(&start, NULL);

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, send_requests, NULL) != 0) {
            perror("Thread creation failed");
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Thread join failed");
            return 1;
        }
    }

    gettimeofday(&end, NULL);

    double elapsed = (end.tv_sec - start.tv_sec) * 1000.0;
    elapsed += (end.tv_usec - start.tv_usec) / 1000.0;

    printf("All requests completed in %.2f ms\n", elapsed);
    return 0;
}
