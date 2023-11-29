#pragma once

#include <sys/types.h>
#include <unistd.h>

#define FRONTIER_MAX_SIZE 500

/**
 * @file: frontier_stack.h
 * @brief: defintions for a stack data structure and its operations
 */

typedef struct frontier_stack {
    char ** stack;  // this is a pointer to an array of pointers (where each item is a pointer to a char array (string)
    int top;
} FRONTIER;

int frontier_init(FRONTIER * frontier);
int frontier_push(FRONTIER * frontier, char * incoming_url);
int frontier_pop(FRONTIER * frontier, char * outgoing_url);
int frontier_is_empty(FRONTIER * frontier);
int frontier_is_full(FRONTIER * frontier);
void frontier_cleanup(FRONTIER* frontier);