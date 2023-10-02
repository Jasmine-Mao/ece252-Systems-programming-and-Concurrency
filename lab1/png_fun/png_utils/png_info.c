/**
 * @brief   functions for grabbing png information
 * Authored by Evelyn; modified by Jasmine.
 */

// #include <sys/types.h>
// #include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include "png_info.h"

int is_png(const char *fpath){
    
    // struct stat type_buffer;                                     <-- remove later, going to use this logic to ensure the files arent broken when we use dis for
    // if (lstat(fpath, &type_buffer) < 0) {                            catpng, and so it can exit gracefully
    //     perror("lstat error");
    //     return -1;
    // } 
    // if (!S_ISREG(type_buffer.st_mode)) {
    //     fprintf(stderr, "%s: Not a regular file", fpath);
    //     return -1;
    // }

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
    return 0;
}

// int get_png_height(struct data_IHDR *buf){

// }

// int get_png_width(struct data_IHDR *buf){

// }

// int get_png_data_IHDR(struct data_IHDR *out, FILE *fp, long offset, int whence){

// }