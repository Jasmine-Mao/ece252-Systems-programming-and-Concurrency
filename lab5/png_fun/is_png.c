/**
 * @brief   functions for grabbing png information
 * Authored by Evelyn; modified by Jasmine.
 */

#include <stdio.h>
#include <stdlib.h>
#include "findpng2.h"

int is_png(char* buf){
    // FILE *f = fopen(fpath, "r");
    // if (!f) {
    //     perror("fopen");
    //     return -1;
    // }

    // char *read_buffer = malloc(4);
    // fread(read_buffer, 4, 1, f);
    // verify bytes 2-4 make up correct png signature
    if ((buf[1] != 0x50) ||
        (buf[2] != 0x4E) ||
        (buf[3] != 0x47)) {
        return -1;
    }
    return 0;
}
int get_png_height(struct data_IHDR *buf){return buf->height;}
int get_png_width(struct data_IHDR *buf){return buf->width;}
void set_png_height(struct data_IHDR *buf, int new_height){buf->height = new_height;}
void set_png_width(struct data_IHDR *buf, int new_width){buf->width = new_width;}