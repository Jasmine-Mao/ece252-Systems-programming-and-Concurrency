#include <sys/types.h>
#include <unistd.h>

/**
 * @file: ring_buffer.h
 * @brief: defintions for a ring buffer data structure and its operations
 */

#define MAX_PNG_SIZE 10000

typedef struct data_buf{
    u_int8_t png_data[MAX_PNG_SIZE];
    size_t size;
    size_t max_size;
    int seq;
} DATA_BUF;

typedef struct ring_buffer {
    DATA_BUF * queue;
    int head;
    int tail;
    int capacity;
} RING_BUFFER;

typedef struct img_buf{
    U8* buf;
    size_t size;
    int write_success;
    int seq;
} IMG_BUF;

int ring_buffer_init(RING_BUFFER * ring_buf, int buffer_size);
int ring_buffer_is_full(RING_BUFFER * ring_buf);
int ring_buffer_is_empty(RING_BUFFER * ring_buf);
int ring_buffer_insert(RING_BUFFER * ring_buf, DATA_BUF * png_buf);
int ring_buffer_pop(RING_BUFFER * ring_buf, DATA_BUF * popped_dest);