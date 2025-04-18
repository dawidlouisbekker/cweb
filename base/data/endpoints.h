#include <unistd.h>

/* Documentation
  Read first directory. / is replaced with int which specifies the length of a path/file name
*/

#define REGION_SIZE (sizeof(struct endpoint_data) * MAX_ENDPOINTS)



struct endpoint_data {
    char path[256];
    char methods[256]; // store method as string directly
    struct endpoint_data* next;
};

// Global pointer to the beginning of the mmap region
struct endpoint_data* mmap_region = NULL;
size_t endpoint_count = 0;

void set_max_endpoints(int max_endpoints);

struct endpoint_data* add_endpoint(const char* path);

//int create_mmap();