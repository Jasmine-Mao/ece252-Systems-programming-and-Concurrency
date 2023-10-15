#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <arpa/inet.h>
#include "zutil.h"
#include "crc.h"
#include "cat_png.h"

void create_ihdr_chunk(struct ihdr_chunk *ihdr_buf){
    data_IHDR_p data = malloc(DATA_IHDR_SIZE);

    // Set length and type fields
    ihdr_buf->length = (U32)htonl(DATA_IHDR_SIZE);
    const char* ihdr_type = "IHDR";
    memcpy(ihdr_buf->type, ihdr_type, 4);

    // Set data fields
    data->width = htonl(PNG_WIDTH);
    data->height = htonl(PNG_HEIGHT);
    data->bit_depth = 0x08; 
    data->color_type = 0x06;
    data->compression = 0;
    data->filter = 0;  
    data->interlace = 0;
    ihdr_buf->p_data = data;

    // Calculate and set CRC
    U8 ihdr_crc_buffer[DATA_IHDR_SIZE + 4];
    memcpy(ihdr_crc_buffer, "IHDR", 4);
    memcpy(ihdr_crc_buffer + 4, ihdr_buf->p_data, DATA_IHDR_SIZE);
    U32 ihdr_crc = crc(ihdr_crc_buffer, DATA_IHDR_SIZE + 4);
    ihdr_buf->crc = htonl(ihdr_crc);

}

void create_iend_chunk(struct chunk * iend_buf){
    iend_buf->p_data = NULL;

    // Set length and type fields
    iend_buf->length = 0;
    const char* iend_type = "IEND";
    memcpy(iend_buf->type, iend_type, 4);

    // Calculate and set CRC
    U32 iend_crc = crc(iend_buf->type, 4);
    iend_buf->crc = htonl(iend_crc);
}

int write_png(struct simple_PNG *png_to_write, size_t idat_data_size) {
    FILE *output_png = fopen("all.png", "wb+");
    if (!output_png) {
        perror("fopen");
        fclose(output_png);
        return -1;
    }

    size_t output_size = 8 + IHDR_CHUNK_SIZE + idat_data_size + 12 + IEND_CHUNK_SIZE;
    U8 * write_buffer = malloc(output_size);

    // Copy header to write buffer
    const char png_header[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

    // Copy header IHDR chunk
    memcpy(write_buffer, png_header, sizeof(png_header));
    memcpy(write_buffer + 8, &(png_to_write->p_IHDR->length), CHUNK_LEN_SIZE);
    memcpy(write_buffer + 12, &png_to_write->p_IHDR->type, CHUNK_TYPE_SIZE);
    memcpy(write_buffer + 16, png_to_write->p_IHDR->p_data, DATA_IHDR_SIZE);
    memcpy(write_buffer + 29, &(png_to_write->p_IHDR->crc), CHUNK_CRC_SIZE);

    // Copy IDAT chunk
    memcpy(write_buffer + 33, &(png_to_write->p_IDAT->length), CHUNK_LEN_SIZE);
    memcpy(write_buffer + 37, png_to_write->p_IDAT->type, CHUNK_TYPE_SIZE);
    memcpy(write_buffer + 41, png_to_write->p_IDAT->p_data, idat_data_size);
    memcpy(write_buffer + idat_data_size + 41, &(png_to_write->p_IDAT->crc), CHUNK_CRC_SIZE);

    // Copy IEND chunk
    memcpy(write_buffer + idat_data_size + 45, &(png_to_write->p_IEND->length), CHUNK_LEN_SIZE);
    memcpy(write_buffer + idat_data_size + 49, png_to_write->p_IEND->type, CHUNK_TYPE_SIZE);
    memcpy(write_buffer + idat_data_size + 53, &(png_to_write->p_IEND->crc), CHUNK_CRC_SIZE);

    if (fwrite(write_buffer, 1, output_size, output_png) != output_size) {
        perror("Error writing concatenated png");
        free(write_buffer);
        fclose(output_png);
        return -1;
    }

    free(write_buffer);
    fclose(output_png);
    return 0;
}

int concatenate_png(){
    simple_PNG_p png_all = malloc(sizeof(struct simple_PNG));
    png_all->p_IHDR = malloc(IHDR_CHUNK_SIZE);
    png_all->p_IEND = malloc(sizeof(struct chunk));
    // IHDR and IEND chunks are populated with constants
    create_ihdr_chunk(png_all->p_IHDR);
    create_iend_chunk(png_all->p_IEND);

    // IDAT Compression
    U8 *def_buf = malloc(INFLATED_DATA_SIZE);
    size_t def_actual = 0;  // size of compressed data stored here
    if (mem_def(def_buf, &def_actual, idat_data, INFLATED_DATA_SIZE, Z_DEFAULT_COMPRESSION) != 0){
        perror("Error while deflating data");
        free(png_all->p_IHDR->p_data);
        free(png_all->p_IHDR);
        free(png_all->p_IEND);
        free(png_all);
        free(def_buf);
        return -1;
    }
    size_t idat_chunk_size = def_actual + CHUNK_LEN_SIZE + CHUNK_TYPE_SIZE + CHUNK_CRC_SIZE;

    // Trim to size of compressed data
    U8* deflated_idat = malloc(def_actual);
    memcpy(deflated_idat, def_buf, def_actual);
    free(def_buf);

    chunk_p idat = malloc(idat_chunk_size);
    png_all->p_IDAT = idat;
    png_all->p_IDAT->p_data = deflated_idat;

    // Set IDAT length and type fields
    png_all->p_IDAT->length = htonl(def_actual);
    const char* ihdr_type = "IDAT";
    memcpy(png_all->p_IDAT->type, ihdr_type, 4);

    // Set IDAT CRC
    unsigned char idat_crc_buffer[def_actual + 4];
    memcpy(idat_crc_buffer, "IDAT", 4);
    memcpy(idat_crc_buffer + 4, png_all->p_IDAT->p_data, def_actual);
    U32 idat_crc = crc(idat_crc_buffer, def_actual + 4);
    png_all->p_IDAT->crc = htonl(idat_crc);

    int ret;
    ret = write_png(png_all, def_actual);
    
    free(png_all->p_IHDR->p_data);
    free(png_all->p_IHDR);
    free(png_all->p_IDAT->p_data);
    free(png_all->p_IDAT);
    free(png_all->p_IEND);
    free(png_all);

    if (ret < 0 ){
        return -1;
    }
    return 0;
}
