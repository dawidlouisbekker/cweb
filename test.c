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
#define REQUESTS_PER_THREAD 10
#define HOST "127.0.0.1"

#define PORT 8081



void *send_requests(void *arg) {
    for (int i = 0; i < REQUESTS_PER_THREAD; i++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("Socket creation failed");
            continue;
        }

        struct hostent *server = gethostbyname(HOST);
        if (server == NULL) {
            fprintf(stderr, "No such host\n");
            close(sock);
            continue;
        }

        struct sockaddr_in server_addr = {0};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Connection failed");
            close(sock);
            continue;
        }

        const char *request = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
        send(sock, request, strlen(request), 0);

        char buffer[1024];
        while (recv(sock, buffer, sizeof(buffer), 0) > 0); // Discard response

        close(sock);
    }
    return NULL;
}

int main() {

    pthread_t threads[NUM_THREADS];
    struct timeval start, end;

    gettimeofday(&start, NULL);

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, send_requests, NULL);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&end, NULL);

    double elapsed = (end.tv_sec - start.tv_sec) * 1000.0;
    elapsed += (end.tv_usec - start.tv_usec) / 1000.0;

    printf("All requests completed in %.2f ms\n", elapsed);
    return 0;
}
