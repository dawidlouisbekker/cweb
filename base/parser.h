#include <stdio.h>

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// List of "heavy" headers
const char* heavy_headers[] = {
    "Authorization",
    "Cookie",
    "Content-Length",
    "Transfer-Encoding",
    "Expect",
    "Content-Type"
};
const int heavy_header_count = sizeof(heavy_headers) / sizeof(heavy_headers[0]);

typedef struct {
    bool is_get;
    bool has_heavy_headers;
} HeaderScanResult;

//GET REQUEST only for high perfomance like CDN.

HeaderScanResult fast_header_scan(const char* buffer, size_t length) {
    HeaderScanResult result = {0};
    result.is_get = strncmp(buffer, "GET ", 4) == 0;

    if (!result.is_get) {
        // Early exit: not a GET request
        result.has_heavy_headers = true;
        return result;
    }

    // Scan line-by-line until double CRLF
    const char* ptr = buffer;
    const char* end = buffer + length;

    while (ptr < end) {
        const char* line_end = strstr(ptr, "\r\n");
        if (!line_end) break;

        size_t line_len = line_end - ptr;
        if (line_len == 0) break; // Empty line — end of headers

        for (int i = 0; i < heavy_header_count; ++i) {
            if (strncmp(ptr, heavy_headers[i], strlen(heavy_headers[i])) == 0) {
                result.has_heavy_headers = true;
                return result; // Early exit — we found something heavy
            }
        }

        ptr = line_end + 2;
    }

    return result; // No heavy headers found, fast path possible
}
