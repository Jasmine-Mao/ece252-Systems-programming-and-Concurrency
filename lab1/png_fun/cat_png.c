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
#define IEND_CHUNK_SIZE 12

// One-time initialization of non-height IHDR values
data_IHDR_p read_ihdr(const char *fpath){
 /*also valgrind ./catpng starter/images/uweng.png --leak-check=full and valgrind ./catpng starter/images/uweng.png give different error counts*/
    FILE *f = fopen(fpath, "rb");
    if (!f) {
        perror("fopen");
        fclose(f);
        exit(1);
    }

    if (fseek(f, 16, SEEK_SET) != 0){
        perror("fseek");
        fclose(f);
        exit(1);
    }

    data_IHDR_p data_reading = malloc(DATA_IHDR_SIZE);

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

size_t concatenate_idat(const char *fpath, U8 *idat, size_t current_idat_end){
    FILE *f = fopen(fpath, "rb");
    if (fseek(f, 33, SEEK_SET) != 0){
        perror("fseek");
        fclose(f);
        exit(1);
    }
    int length;
    if(fread(&length, sizeof(int), 1, f) !=1){
        perror("fread");
        fclose(f);
        exit(1);
    }
    length = ntohl(length);
    if (fseek(f, 4, SEEK_CUR) != 0){
        perror("fseek");
        fclose(f);
        exit(1);
    }
    U8 *read_buffer = malloc(length);
    if(fread(read_buffer, sizeof(U8), length, f) != length){
        perror("fread");
        free(read_buffer);
        fclose(f);
        exit(1);
    }
    size_t inflated_size = 0;
    // inflate data in read buffer and store it in next avaliable memory in idat
    if (mem_inf(idat + current_idat_end, &inflated_size, read_buffer, length) != 0){
        perror("Error while inflating data");
        free(read_buffer);
        fclose(f);
        exit(1);
    }
    current_idat_end += inflated_size;
    
    free(read_buffer);
    fclose(f);

    return current_idat_end;
}

int write_png(struct simple_PNG *png_to_write, size_t idat_chunk_size){
    FILE *output_png = fopen("all.png", "w+b");
    if (!output_png) {
        perror("fopen");
        return -1;
    }
    const char png_header[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    fwrite(png_header, 1, sizeof(png_header), output_png);
    fwrite(png_to_write->p_IHDR, 1, IHDR_CHUNK_SIZE, output_png);
    fwrite(png_to_write->p_IDAT, 1, idat_chunk_size, output_png);
    fwrite(png_to_write->p_IEND, 1, IEND_CHUNK_SIZE, output_png);

    fclose(output_png);
    return 0;
}

int concatenate_pngs(int argc, char* argv[]){
    simple_PNG_p png_all = malloc(sizeof(struct simple_PNG));

    png_all->p_IDAT = NULL;
    png_all->p_IEND = NULL;

    ihdr_chunk_p ihdr_all = malloc(IHDR_CHUNK_SIZE);
    ihdr_all -> p_data = read_ihdr(argv[1]);
    png_all->p_IHDR = ihdr_all;

    int width_all = get_png_width(png_all->p_IHDR->p_data);
    int height_all = 0;
    int current_height = 0;
    for (int i = 1; i < argc; i++){
        current_height = read_height(argv[i]);
        height_all += current_height;
    }

    set_png_height(png_all->p_IHDR->p_data, height_all);

    U64 real_size = height_all * (width_all * 4 + 1);
    U8 * idat_data = malloc(real_size);  // size of uncompressed idat
    size_t current_idat_end = 0;

    for (int i = 1; i < argc; i++){
        current_idat_end = concatenate_idat(argv[i], idat_data, current_idat_end);
    }

    // deflate data
    U8* deflated_idat = NULL;

    U8 *def_buf = malloc(real_size); //size of decompressed data
    size_t def_actual = 0; //var for updated size
    
    if (mem_def(def_buf, &def_actual, idat_data, real_size, Z_DEFAULT_COMPRESSION) != 0){
        perror("Error while deflating data");
        exit(1);
    }
    size_t idat_chunk_size = 4 + 4 + def_actual + 4;

    deflated_idat = malloc(def_actual);
    memcpy(deflated_idat, def_buf, def_actual);

    chunk_p idat = malloc(idat_chunk_size);
    png_all->p_IDAT = idat;
    png_all->p_IDAT->p_data = deflated_idat;


    // validate crc for ihdr chunk
    U32 ihdr_crc = crc((unsigned char*)&(png_all->p_IHDR->p_data), DATA_IHDR_SIZE);
    //for ihdr: crc(,sizeof(ihdr_chunk))
    png_all->p_IHDR->crc = ihdr_crc;
    png_all->p_IHDR->length = htonl(DATA_IHDR_SIZE - 8);


    // validate crc for idat chunk
    U32 idat_crc = crc(def_buf, def_actual);
    png_all->p_IDAT->crc = idat_crc;
    //printf("crc_val = %u\n", idat_crc);
    /* how to apply crc to chunk*/

    write_png(png_all, idat_chunk_size);

    free(idat_data);
    free(deflated_idat);
    free(idat);
    free(def_buf);
    free(ihdr_all);
    free(png_all);
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