#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void status_response(int client_socket, int code) {
    const char *status_text;
    switch (code) {
        case 200: status_text = "OK"; break;
        case 400: status_text = "Bad Request"; break;
        case 401: status_text = "Unauthorized"; break;
        case 403: status_text = "Forbidden"; break;
        case 404: status_text = "Not Found"; break;
        case 500: status_text = "Internal Server Error"; break;
        default: status_text = "Unknown"; break;
    }

    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.1 %d %s\r\n"
             "Content-Length: 0\r\n"
             "Connection: close\r\n\r\n",
             code, status_text);

    send(client_socket, header, strlen(header), 0);
}

static char *read_file(const char *filepath, long *filesize) {
    FILE *file = fopen(filepath, "rb");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    *filesize = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(*filesize);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, *filesize, file);
    fclose(file);
    return buffer;
}

void send_favicon_response(int client_socket, const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        const char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, not_found, strlen(not_found), 0);
        return;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    // Allocate buffer for image
    char *image_data = (char *)malloc(filesize);
    if (!image_data) {
        fclose(file);
        const char *error_response = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, error_response, strlen(error_response), 0);
        return;
    }

    fread(image_data, 1, filesize, file);
    fclose(file);

    // Build and send the HTTP response
    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: image/x-icon\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n\r\n", filesize);

    send(client_socket, header, strlen(header), 0);
    send(client_socket, image_data, filesize, 0);

    free(image_data);
}

void wasm_response(int client_socket, const char *filepath) {
    // Open the WASM file
    printf("Servering wasm.");
    long filesize;
    char *data = read_file(filepath, &filesize);
    if (!data < 0) {
        status_response(client_socket, 404);
        close(client_socket);
        return;
    }

    // Prepare the response headers


    // Allocate a buffer for the headers with file size substitution
    char header_buffer[256];
    snprintf(header_buffer, sizeof(header_buffer),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/wasm\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Cross-Origin-Resource-Policy: same-origin\r\n"
        "Cross-Origin-Embedder-Policy: require-corp\r\n"
        "Content-Length: %ld\r\n"
        "Cache-Control: no-store\r\n"
        "\r\n", filesize);
    send(client_socket, header_buffer, strlen(header_buffer), 0);
    send(client_socket, data, filesize, 0);
    // Clean up
    free(data);
}

void image_response(int client_socket, const char *filepath) {
    long filesize;
    char *data = read_file(filepath, &filesize);

    if (!data) {
        status_response(client_socket, 404);
        return;
    }

    // Guessing MIME type based on extension (very basic)
    const char *ext = strrchr(filepath, '.');
    const char *content_type = "application/octet-stream";
    if (ext) {
        if (strcmp(ext, ".ico") == 0) content_type = "image/x-icon";
        else if (strcmp(ext, ".png") == 0) content_type = "image/png";
        else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) content_type = "image/jpeg";
        else if (strcmp(ext, ".gif") == 0) content_type = "image/gif";
        else if (strcmp(ext, ".svg") == 0) content_type = "image/svg";
    }

    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n\r\n",
             content_type, filesize);

    send(client_socket, header, strlen(header), 0);
    send(client_socket, data, filesize, 0);

    free(data);
}

void html_response(int client_socket, const char *filepath) {
    long filesize;
    char *data = read_file(filepath, &filesize);

    if (!data) {
        status_response(client_socket, 404);
        return;
    }

    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n\r\n", filesize);

    send(client_socket, header, strlen(header), 0);
    send(client_socket, data, filesize, 0);

    free(data);
}

void json_response(int client_socket, const char *json) {
    size_t length = strlen(json);
    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n\r\n", length);

    send(client_socket, header, strlen(header), 0);
    send(client_socket, json, length, 0);
}


