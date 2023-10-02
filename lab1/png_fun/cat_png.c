#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include "png_utils/png_info.h"
#include "png_utils/zutil.h"
#include "png_utils/crc.h"

#include <arpa/inet.h>
#include "cat_png.h"

/*memory leaks (valgrind)

*/

// One-time initialization of non-height IHDR values
data_IHDR_p read_ihdr(const char *fpath){ /*memory leak here -- pointer is returned, how do we free?
 also valgrind ./catpng starter/images/uweng.png --leak-check=full and valgrind ./catpng starter/images/uweng.png give different error counts*/
    FILE *f = fopen(fpath, "rb");
    if (!f) {
        perror("fopen");
        return NULL;
    }

    data_IHDR_p initial = malloc(13);
    fseek(f, 16, SEEK_SET);
    /*header 8 + ihdr len 4 + ihdr type 4 + ihdr data width 4 = 20*/

    if(fread(&(initial->width), sizeof(int), 1, f) !=1){
        perror("Error reading width");
        fclose(f);
        return NULL;
    };
    initial->width = ntohl(initial->width);

    if(fread(&(initial->height), sizeof(int), 1, f) !=1){
        perror("Error reading height");
        fclose(f);
        return NULL;
    };
    initial->height = ntohl(initial->height);

    if(fread(&(initial->bit_depth), sizeof(char), 1, f) !=1){
        perror("Error reading bit_depth");
        fclose(f);
        return NULL;
    };
    //initial->bit_depth = ntohl(initial->bit_depth);
    if(fread(&(initial->color_type), sizeof(char), 1, f) !=1){
        perror("Error reading color type");
        fclose(f);
        return NULL;
    };

    if(fread(&(initial->compression), sizeof(char), 1, f) !=1){
        perror("Error reading compression");
        fclose(f);
        return NULL;
    };

    if(fread(&(initial->filter), sizeof(char), 1, f) !=1){
        perror("Error reading filter");
        fclose(f);
        return NULL;
    };

    if(fread(&(initial->interlace), sizeof(char), 1, f) !=1){
        perror("Error reading interlace");
        fclose(f);
        return NULL;
    };   

    //printf("%d, %d, %d, %d, %d, %d, %d\n", initial->width, initial->height, initial->bit_depth, initial->color_type, initial->compression, initial->filter, nitial->interlace);

    return initial; // temp
    //return NULL;
}

int read_height(const char *fpath){
    int height = 0;
    FILE *f = fopen(fpath, "rb");
    if (!f) {
        perror("fopen");
        return -1;
    }
    
    fseek(f, 20, SEEK_SET);
    /*header 8 + ihdr len 4 + ihdr type 4 + ihdr data width 4 = 20*/

    if(fread(&height, sizeof(int), 1, f) !=1){
        perror("Error reading height");
        fclose(f);
        return -1;
    };
    height = ntohl(height);
    return height;
}

int concatenate_idat(const char *fpath, chunk_p *idat){
    return 0;
}

int concatenate_pngs(int argc, char* argv[]){
    simple_PNG_p all_png = malloc(sizeof(struct simple_PNG));
    all_png->p_IDAT = NULL;
    all_png->p_IEND = NULL;
    all_png->p_IHDR = read_ihdr(argv[1]);

    int k, j = 0;
    for (int i = 1; i < argc; i++){
        k = read_height(argv[i]);
        j += read_height(argv[i]);
        //printf("%d total: %d \n", k, j);
    }
    free(all_png); /*there was a memory leak here idk if this was the right soln*/
    return 0;
}

int verify_png(const char *fpath){ /*verifying a correct file path to avoid segmentation faults prior to calling is_png */
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
    
    concatenate_pngs(argc, argv);

    for (int i = 0; i < argc; ++i) { 
        free(argv[i]); 
    }
    free(argv);
    return 0;
}