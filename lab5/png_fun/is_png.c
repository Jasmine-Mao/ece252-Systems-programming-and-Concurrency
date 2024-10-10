/**
 * @brief   functions for grabbing png information
 * Authored by Evelyn; modified by Jasmine.
 */

#include <stdio.h>
#include <stdlib.h>
#include "findpng3.h"

int is_png(char* buf){
    if ((buf[1] != 0x50) ||
        (buf[2] != 0x4E) ||
        (buf[3] != 0x47)) {
        return -1;
    }
    return 0;
}