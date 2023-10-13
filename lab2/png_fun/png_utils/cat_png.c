#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include "png_utils/zutil.h"
#include "png_utils/crc.h"

#include <arpa/inet.h>
#include "cat_png.h"

void create_ihdr_chunk(ihdr_chunk_p dest){
    // Chunk size is 12 + the size of the data pointer
    ihdr_chunk_p ihdr_chunk = malloc(sizeof(struct ihdr_chunk));
    ihdr_chunk->p_data = malloc(DATA_IHDR_SIZE);

    // Set length and type fields
    ihdr_chunk->length = htonl(DATA_IHDR_SIZE);
    const char* ihdr_type = "IHDR";
    memcpy(ihdr_chunk->type, ihdr_type, 4);

    // Set data fields
    ihdr_chunk->p_data->width = htonl(PNG_WIDTH);
    ihdr_chunk->p_data->width = htonl(PNG_HEIGHT);
    ihdr_chunk->p_data->bit_depth = 0x08; 
    ihdr_chunk->p_data->color_type = 0x06;
    ihdr_chunk->p_data->compression = 0;
    ihdr_chunk->p_data->filter = 0;  
    ihdr_chunk->p_data->interlace = 0;

    // Calculate and set CRC
    U8 ihdr_crc_buffer[DATA_IHDR_SIZE + 4];
    memcpy(ihdr_crc_buffer, "IHDR", 4);
    memcpy(ihdr_crc_buffer + 4, ihdr_chunk->p_data, DATA_IHDR_SIZE);
    U32 ihdr_crc = crc(ihdr_crc_buffer, DATA_IHDR_SIZE + 4);
    ihdr_chunk->crc = htonl(ihdr_crc);

    dest = ihdr_chunk;
}

void create_iend_chunk(chunk_p dest){
    chunk_p iend_chunk = malloc(sizeof(struct chunk));
    iend_chunk->p_data = NULL;

    // Set length and type fields
    iend_chunk->length = 0;
    const char* iend_type = "IEND";
    memcpy(iend_chunk->type, iend_type, 4);

    // Calculate and set CRC
    U32 iend_crc = crc(iend_chunk->type, 4);
    iend_chunk->crc = htonl(iend_crc);

    dest = iend_chunk;
}

// /*read, inflates, and concatenates IDAT data*/
// size_t concatenate_idat(const char *fpath, U8 *idat, size_t current_idat_end){
//     FILE *f = fopen(fpath, "rb");
//     if (fseek(f, 33, SEEK_SET) != 0){
//         perror("fseek");
//         fclose(f);
//         return -1;
//     }
//     int length;
//     if(fread(&length, sizeof(int), 1, f) !=1){
//         perror("fread");
//         fclose(f);
//         return -1;
//     }
//     length = ntohl(length);
//     if (fseek(f, 4, SEEK_CUR) != 0){
//         perror("fseek");
//         fclose(f);
//         return -1;
//     }
//     U8 *read_buffer = malloc(length);
//     if(fread(read_buffer, sizeof(U8), length, f) != length){
//         perror("fread");
//         free(read_buffer);
//         fclose(f);
//         return -1;
//     }
//     size_t inflated_size = 0;
//     /* inflate data in read buffer and store it in next avaliable memory in idat*/
//     if (mem_inf(idat + current_idat_end, &inflated_size, read_buffer, length) != 0){
//         perror("Error while inflating data");
//         free(read_buffer);
//         fclose(f);
//         return -1;
//     }
//     current_idat_end += inflated_size;
    
//     free(read_buffer);
//     fclose(f);

//     return current_idat_end;
// }

// int write_png(struct simple_PNG *png_to_write, size_t idat_data_size) {
//     FILE *output_png = fopen("all.png", "wb+");
//     if (!output_png) {
//         perror("fopen");
//         fclose(output_png);
//         return -1;
//     }

//     size_t output_size = 8 + IHDR_CHUNK_SIZE + idat_data_size + 12 + IEND_CHUNK_SIZE;
//     U8 * write_buffer = malloc(output_size);

//     /*Copy header to write buffer*/
//     const char png_header[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

//     /* Copy header IHDR chunk*/
//     memcpy(write_buffer, png_header, sizeof(png_header));
//     memcpy(write_buffer + 8, &(png_to_write->p_IHDR->length), CHUNK_LEN_SIZE);
//     memcpy(write_buffer + 12, png_to_write->p_IHDR->type, CHUNK_TYPE_SIZE);
//     memcpy(write_buffer + 16, png_to_write->p_IHDR->p_data, DATA_IHDR_SIZE);
//     memcpy(write_buffer + 29, &(png_to_write->p_IHDR->crc), CHUNK_CRC_SIZE);

//     /* Copy IDAT chunk*/
//     memcpy(write_buffer + 33, &(png_to_write->p_IDAT->length), CHUNK_LEN_SIZE);
//     memcpy(write_buffer + 37, png_to_write->p_IDAT->type, CHUNK_TYPE_SIZE);
//     memcpy(write_buffer + 41, png_to_write->p_IDAT->p_data, idat_data_size);
//     memcpy(write_buffer + idat_data_size + 41, &(png_to_write->p_IDAT->crc), CHUNK_CRC_SIZE);

//     /* Copy IEND chunk*/
//     memcpy(write_buffer + idat_data_size + 45, &(png_to_write->p_IEND->length), CHUNK_LEN_SIZE);
//     memcpy(write_buffer + idat_data_size + 49, png_to_write->p_IEND->type, CHUNK_TYPE_SIZE);
//     memcpy(write_buffer + idat_data_size + 53, &(png_to_write->p_IEND->crc), CHUNK_CRC_SIZE);

//     if (fwrite(write_buffer, 1, output_size, output_png) != output_size) {
//         perror("Error writing concatenated png");
//         free(write_buffer);
//         fclose(output_png);
//         return -1;
//     }

//     free(write_buffer);
//     fclose(output_png);
//     return 0;
// }

// int concatenate_pngs(int argc, char* argv[]){
//     simple_PNG_p png_all = malloc(sizeof(struct simple_PNG));

//     png_all->p_IDAT = NULL;
//     png_all->p_IEND = NULL;

//     ihdr_chunk_p ihdr_all = malloc(IHDR_CHUNK_SIZE);
//     ihdr_all->p_data = malloc(DATA_IHDR_SIZE);
//     read_ihdr(argv[1], ihdr_all->p_data); 
//     png_all->p_IHDR = ihdr_all;

//     int width_all = get_png_width(png_all->p_IHDR->p_data);

//     int height_all = 0;
//     int current_height = 0;
//     for (int i = 1; i < argc; i++){
//         current_height = read_height(argv[i]);
//         height_all += current_height;
//     }

//     set_png_height(png_all->p_IHDR->p_data, height_all);

//     U64 real_size = height_all * (width_all * 4 + 1);
//     U8 * idat_data = malloc(real_size);  /* size of uncompressed idat*/
//     size_t current_idat_end = 0;

//     for (int i = 1; i < argc; i++){
//         current_idat_end = concatenate_idat(argv[i], idat_data, current_idat_end);
//     }

//     /* deflate data*/
//     U8* deflated_idat = NULL;

//     U8 *def_buf = malloc(real_size); /*size of decompressed data*/
//     size_t def_actual = 0; /*var for updated size*/
    
//     /* IDAT Compression */
//     if (mem_def(def_buf, &def_actual, idat_data, real_size, Z_DEFAULT_COMPRESSION) != 0){
//         perror("Error while deflating data");
//         free(png_all);
//         free(ihdr_all->p_data);
//         free(ihdr_all);
//         free(idat_data);
//         free(def_buf);
//         return -1;
//     }
//     size_t idat_chunk_size = 4 + 4 + def_actual + 4;

//     deflated_idat = malloc(def_actual);
//     memcpy(deflated_idat, def_buf, def_actual);

//     chunk_p idat = malloc(idat_chunk_size);
//     png_all->p_IDAT = idat;
//     png_all->p_IDAT->p_data = deflated_idat;

//     /* Set height and width to big endian before copy*/
//     int big_end_height = htonl(get_png_height(png_all->p_IHDR->p_data));
//     int big_end_width = htonl(get_png_width(png_all->p_IHDR->p_data));
//     set_png_height(png_all->p_IHDR->p_data, big_end_height);
//     set_png_width(png_all->p_IHDR->p_data, big_end_width);

//     /* Store data in buffer to calculate crc*/
//     U8 ihdr_crc_buffer[DATA_IHDR_SIZE + 4];
//     memcpy(ihdr_crc_buffer, "IHDR", 4);
//     memcpy(ihdr_crc_buffer + 4, png_all->p_IHDR->p_data, DATA_IHDR_SIZE);

//     /* Calculate and set CRC for the IHDR chunk*/
//     U32 ihdr_crc = crc(ihdr_crc_buffer, DATA_IHDR_SIZE + 4);
//     png_all->p_IHDR->crc = htonl(ihdr_crc);
//     png_all->p_IHDR->length = htonl(DATA_IHDR_SIZE);

//     /* type name for IHDR */
//     png_all->p_IHDR->type[0] = 0x49;
//     png_all->p_IHDR->type[1] = 0x48;
//     png_all->p_IHDR->type[2] = 0x44;
//     png_all->p_IHDR->type[3] = 0x52;

//     /*Set crc for IDAT*/
//     unsigned char idat_crc_buffer[def_actual + 4];
//     memcpy(idat_crc_buffer, "IDAT", 4);
//     memcpy(idat_crc_buffer + 4, png_all->p_IDAT->p_data, def_actual);
//     U32 idat_crc = crc(idat_crc_buffer, def_actual + 4);
//     png_all->p_IDAT->crc = htonl(idat_crc);
//     png_all->p_IDAT->length = htonl(def_actual);

//     /* type name for IDAT*/
//     png_all->p_IDAT->type[0] = 0x49;
//     png_all->p_IDAT->type[1] = 0x44;
//     png_all->p_IDAT->type[2] = 0x41;
//     png_all->p_IDAT->type[3] = 0x54;

//     /*type name and allocation for IEND*/
//     png_all->p_IEND = malloc(sizeof(struct chunk));
//     png_all->p_IEND->p_data = NULL;
//     png_all->p_IEND->length = 0;
//     png_all->p_IEND->type[0] = 0x49;
//     png_all->p_IEND->type[1] = 0x45;
//     png_all->p_IEND->type[2] = 0x4E;
//     png_all->p_IEND->type[3] = 0x44;

//     /*set crc for IEND*/
//     U32 iend_crc = crc(png_all->p_IEND->type, 4);
//     iend_crc = htonl(iend_crc);

//     png_all->p_IEND->crc = iend_crc;

//     write_png(png_all, def_actual);
    
//     free(png_all->p_IEND);
//     free(png_all);
//     free(idat_data);
//     free(deflated_idat);
//     free(idat);
//     free(def_buf);
//     free(ihdr_all->p_data);
//     free(ihdr_all);
//     return 0;
// }
