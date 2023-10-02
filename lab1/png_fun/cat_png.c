#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include "cat_png.h"


/*
goal: given the paths to several png files, return a png file titled all.png that is a vertical concatenation of the given pngs
procedure:
1, locating the given pngs
2. verifying that they are pngs (is_png)
3. open pngs (fopen)


3. reading the pngs and their data
a. inflate w zlib
b. identify ihdr, idat, iend
4. creating a new png file; writing to the png file (appending the new pngs one by one to the file)
5. compress and create one idat
*/
data_IHDR_p read_ihdr(const char *fpath){
    return NULL; // temp
}

int concatenate_pngs(int argc, char* argv){
    simple_PNG_p all_png = malloc(sizeof(struct simple_PNG));
    all_png->p_IHDR = NULL;
    all_png->p_IDAT = NULL;
    all_png->p_IEND = NULL;

    for (int i = 1; i < argc; i++){
        if (all_png->p_IHDR == NULL){
            all_png->p_IHDR = read_ihdr(argv[i]);
        }
    }

    return 0;
}

int verify_png(const char *fpath){
    struct stat type_buffer;
    if (lstat(fpath, &type_buffer) < 0) {
        perror("lstat error");
        return -1;
    } 
    if (!S_ISREG(type_buffer.st_mode)) {
        fprintf(stderr, "%s: Not a regular file\n", fpath);
        return -1;
    }
    return(is_png(fpath));
}

int main(int argc, char* argv[]){
    if (argc < 2){
        fprintf(stderr, "Missing argument\n");
        exit(1);
    }
    for (int i = 1; i < argc; ++i) {
        if (verify_png(argv[i]) < 0){
            fprintf(stderr, "%s: File is not a PNG\n", argv[i]);
            exit(1);
        }
    }

    return 0;
}