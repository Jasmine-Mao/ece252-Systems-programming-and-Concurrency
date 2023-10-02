#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include "png_utils/zutil.h"
#include "png_utils/crc.h"

#include <arpa/inet.h>
#include "cat_png.h"

#define IHDR_CHUNK_SIZE 25

// One-time initialization of non-height IHDR values
data_IHDR_p read_ihdr(const char *fpath){ /*memory leak here -- pointer is returned, how do we free?
 also valgrind ./catpng starter/images/uweng.png --leak-check=full and valgrind ./catpng starter/images/uweng.png give different error counts*/
    FILE *f = fopen(fpath, "rb");
    if (!f) {
        perror("fopen");
        return NULL;
    }

    data_IHDR_p data_reading = malloc(DATA_IHDR_SIZE);
    if (fseek(f, 16, SEEK_SET) != 0){
        perror("fseek");
        free(data_reading);
        fclose(f);
        exit(1);
    }

    /*header 8 + ihdr len 4 + ihdr type 4 + ihdr data width 4 = 20*/
    if(fread(&(data_reading->width), sizeof(int), 1, f) !=1){
        perror("Error reading width");
        fclose(f);
        exit(1);
    };

    data_reading->width = ntohl(data_reading->width);

    data_reading->height = 0;
    data_reading->bit_depth = 1;
    data_reading->color_type = 6;
    data_reading->compression = 0;
    data_reading->filter = 0;
    data_reading->interlace = 0;

    printf("Width: %d\n", data_reading->width);

    return data_reading;
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

    ihdr_chunk_p all_ihdr = malloc(IHDR_CHUNK_SIZE);
    all_ihdr -> p_data = read_ihdr(argv[1]);
    all_png->p_IHDR = all_ihdr;

    int all_height = 0;
    for (int i = 1; i < argc; i++){
        all_height += read_height(argv[i]);
    }

    all_png->p_IHDR->p_data->height = all_height;

    free(all_ihdr);
    free(all_png);
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

    return 0;
}