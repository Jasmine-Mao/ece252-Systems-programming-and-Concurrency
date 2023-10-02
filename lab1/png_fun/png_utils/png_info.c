/**
 * @brief   functions for grabbing png information
 * Authored by Evelyn; modified by Jasmine.
 */

#include <stdio.h>
#include <stdlib.h>
#include "png_info.h"

int is_png(const char *fpath){
    FILE *f = fopen(fpath, "r");
    if (!f) {
        perror("fopen");
        return -1;
    }

    char *read_buffer = malloc(4);
    fread(read_buffer, sizeof(read_buffer), 1, f);

    // verify bytes 2-4 make up correct png signature
    if ((read_buffer[1] != 0x50) ||
        (read_buffer[2] != 0x4E) ||
        (read_buffer[3] != 0x47)) {
        return -1;
    }

    free(read_buffer);
    return 0;
}

int get_png_height(struct data_IHDR *buf){return buf->height;}

int get_png_width(struct data_IHDR *buf){return buf->width;}