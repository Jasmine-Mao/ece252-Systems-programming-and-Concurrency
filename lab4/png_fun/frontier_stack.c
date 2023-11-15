#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "frontier_stack.h"

int frontiers_init(FRONTIER * frontier){
    frontier->top = -1;
    frontier->stack = malloc(500* sizeof(char*));
    return 0;
}

// TODO: @<name_here> stack push and pop operations
int frontiers_push(FRONTIER * stack, char * incoming_url);
int frontiers_pop(FRONTIER * stack, char * outgoing_url);