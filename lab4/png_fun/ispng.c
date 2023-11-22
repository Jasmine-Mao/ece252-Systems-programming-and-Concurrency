/**
 * @brief   functions for grabbing png information
 * Authored by Evelyn; modified by Jasmine.
 */

#include <stdio.h>
#include <stdlib.h>
#include "findpng2.h"

/*
i think we should try running a memory leak test here too
*/

int is_png(const char *fpath){
    FILE *f = fopen(fpath, "r");
    if (!f) {
        perror("fopen");
        return -1;
    }

    char *read_buffer = malloc(4);
    fread(read_buffer, 4, 1, f);

    // verify bytes 2-4 make up correct png signature
    if ((read_buffer[1] != 0x50) ||
        (read_buffer[2] != 0x4E) ||
        (read_buffer[3] != 0x47)) {
        free(read_buffer);
        fclose(f);
        return -1;
    }
    
    free(read_buffer);
    fclose(f);
    return 0;
}
int get_png_height(struct data_IHDR *buf){return buf->height;}
int get_png_width(struct data_IHDR *buf){return buf->width;}
void set_png_height(struct data_IHDR *buf, int new_height){buf->height = new_height;}
void set_png_width(struct data_IHDR *buf, int new_width){buf->width = new_width;}



