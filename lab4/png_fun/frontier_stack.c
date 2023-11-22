#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "frontier_stack.h"

int frontier_init(FRONTIER * frontier){
    frontier->top = -1;
    frontier->stack = malloc(FRONTIER_MAX_SIZE* sizeof(char*));
    return 0;
}

int frontier_push(FRONTIER * frontier, char * incoming_url){
    if (frontier_is_full(frontier)){
        printf("Tried to push while frontiers is full! Something has gone terribly wrong!\n");
    }
    frontier->top++;
    frontier->stack[frontier->top] = strdup(incoming_url);
    printf("push %d\n", frontier->top);
    if (frontier->top > 4){
        printf("index 4 %s\n", frontier->stack[4]);
    }
    return 0;
}

char* frontier_pop(FRONTIER * frontier){
    char * outgoing_url = NULL;
    if (frontier_is_empty(frontier)){
        printf("Tried to pop while frontiers is empty! Something has gone wrong!\n");
    }
    outgoing_url = strdup(frontier->stack[frontier->top]);
    printf("pop %s\n", frontier->stack[frontier->top]);
    printf("pop1 %s\n", outgoing_url);
    frontier->top--;
    return outgoing_url;
}

int frontier_is_empty(FRONTIER * frontier){
    return (frontier->top == -1);
}

int frontier_is_full(FRONTIER * frontier){
    return (frontier->top == FRONTIER_MAX_SIZE - 1);
}

