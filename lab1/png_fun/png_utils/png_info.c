#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include "png_info.h"

int is_png(const char *fpath){
    // ensure file is valid type (regular) 
    // NOTE @jasmine: if you would rather redo this part in find_png
    // feel free since we rlly just need to check header here,
    // i thought i might as well include it in this function
    struct stat type_buffer;
    if (lstat(fpath, &type_buffer) < 0) {
        perror("lstat error");
        return -1;
    } 
    if (!S_ISREG(type_buffer.st_mode)) {
        fprintf(stderr, "%s: Not a regular file", fpath);
        return -1;
    }
    
    FILE *f = fopen(fpath, "r");
    if (!f) {
        perror("fopen");
        return -1;
    }

    char * read_buffer = malloc(4);     // input buffer for reading 4 bytes

    fread(read_buffer, sizeof(read_buffer), 1, f);

    if (read_buffer[0] != 0x89 || 
        read_buffer[1] != 0x50 ||
        read_buffer[2] != 0x4E || 
        read_buffer[3] != 0x47) {
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

// REMOVE LATER, using to test png_info
int main(int argc, char *argv[]) 
{
    if (argc != 2){
        printf("WRONG");
        exit(1);
    }
    if (is_png(argv[1]) < 0){
        printf("%s : Not a PNG file", argv[1]);
        exit(1);
    }
    printf("yay it;s a png (print dimensions later)");
    exit(1);

}