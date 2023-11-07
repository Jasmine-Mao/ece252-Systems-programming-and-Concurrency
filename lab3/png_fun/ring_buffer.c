#include <stdio.h>
#include <stdlib.h>

#include "ring_buffer.h"

int ring_buffer_init(RING_BUFFER * ring_buf, int buffer_size){
    ring_buf->size = buffer_size;
    ring_buf->head = -1;
    ring_buf->tail = -1;

    // Initializes array/queue starting point to the apt address
    // (ring buffer starting address + offset for the struct size)
    ring_buf->queue = (DATA_BUF *)(ring_buf + sizeof(RING_BUFFER));
    return 0;
}

int ring_buffer_is_full(RING_BUFFER * ring_buf){
    return(((ring_buf->tail + 1) % ring_buf->size) == ring_buf->head);
}

int ring_buffer_is_empty(RING_BUFFER * ring_buf){
    return(ring_buf->head == -1);
}

// ENQUEUE PNG DATA
int ring_buffer_insert(RING_BUFFER * ring_buf, DATA_BUF * png_buf){
    if (ring_buffer_is_full(ring_buf)){
        printf("Attempted insert while ring buffer is full. Something has gone wrong! \n");
        return -1;
    }

    if (ring_buf->head == -1){
        ring_buf->head = 0;
    }

    ring_buf->tail = (ring_buf->tail + 1) % ring_buf->size;
    ring_buf->queue[ring_buf->tail] = *png_buf;

    return 0;
}

// DEQEUE PNG DATA
int ring_buffer_pop(RING_BUFFER * ring_buf, DATA_BUF * popped_dest){
    if (ring_buffer_is_empty(ring_buf)){
        printf("Attempted pop while ring buffer is empty. Something has gone wrong! \n");
        return -1;
    }

    // Copy head DATA_BUF instance into dest param
    *popped_dest = ring_buf->queue[ring_buf->head];

    if (ring_buf->head == ring_buf->tail){
        ring_buf->head = -1;
        ring_buf->tail = -1;
    }
    ring_buf->head = (ring_buf->head + 1) % ring_buf->size;

    return 0;
}

