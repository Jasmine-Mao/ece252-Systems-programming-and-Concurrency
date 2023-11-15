#include <sys/types.h>
#include <unistd.h>

/**
 * @file: frontier_stack.h
 * @brief: defintions for a ring buffer data structure and its operations
 */

typedef struct frontier_stack {
    char ** stack;  // this is a pointer to an array of pointers (where each item is a pointer to a char array (string)
    int top;
} FRONTIER;

int frontiers_init(FRONTIER * frontier);
int frontiers_push(FRONTIER * frontier, char * incoming_url);
int frontiers_pop(FRONTIER * frontier, char * outgoing_url);