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
    int size;
} RING_BUFFER;