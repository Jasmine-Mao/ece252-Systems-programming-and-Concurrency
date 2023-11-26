#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "frontier_stack.h"

int frontier_init(FRONTIER * frontier){
    frontier->top = -1;
    frontier->stack = malloc(FRONTIER_MAX_SIZE * sizeof(char*));
    return 0;
}

int frontier_push(FRONTIER * frontier, char * incoming_url){
    if (frontier_is_full(frontier)){
        printf("Tried to push while frontiers is full! Something has gone terribly wrong!\n");
    }
    frontier->top++;
    frontier->stack[frontier->top] = strdup(incoming_url);
    return 0;
}

int frontier_pop(FRONTIER * frontier, char * outgoing_url){
    if (frontier_is_empty(frontier)){
        printf("Tried to pop while frontiers is empty!\n");
        return -1;
    }
    strcpy(outgoing_url, frontier->stack[frontier->top]);
    free(frontier->stack[frontier->top]);
    frontier->top--;
    return 0;
}

int frontier_is_empty(FRONTIER * frontier){
    return (frontier->top == -1);
}

int frontier_is_full(FRONTIER * frontier){
    return (frontier->top == FRONTIER_MAX_SIZE - 1);
}

void frontier_cleanup(FRONTIER* frontier){
    while(!frontier_is_empty(frontier)){
        char temp[256];
        frontier_pop(frontier, temp);
    }
}