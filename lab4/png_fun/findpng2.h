// typedef struct recv_buf2 {
//     char *buf;       /* memory to hold a copy of received data */
//     size_t size;     /* size of valid data in buf in bytes*/
//     size_t max_size; /* max capacity of buf in bytes*/
//     int seq;         /* >=0 sequence number extracted from http header */
//                      /* <0 indicates an invalid seq number */
// } RECV_BUF;
/**
 * @brief  micros and structures for a simple PNG file 
 *
 * Copyright 2018-2020 Yiqing Huang
 *
 * This software may be freely redistributed under the terms of MIT License
 */
#pragma once

/******************************************************************************
 * INCLUDE HEADER FILES
 *****************************************************************************/
#include <stdio.h>

/******************************************************************************
 * DEFINED MACROS 
 *****************************************************************************/

#define PNG_SIG_SIZE    8 /* number of bytes of png image signature data */
#define CHUNK_LEN_SIZE  4 /* chunk length field size in bytes */          
#define CHUNK_TYPE_SIZE 4 /* chunk type field size in bytes */
#define CHUNK_CRC_SIZE  4 /* chunk CRC field size in bytes */
#define DATA_IHDR_SIZE 13 /* IHDR chunk data field size */

/******************************************************************************
 * STRUCTURES and TYPEDEFS 
 *****************************************************************************/
typedef unsigned char U8;
typedef unsigned int  U32;

/* note that there are 13 Bytes valid data, compiler will padd 3 bytes to make
   the structure 16 Bytes due to alignment. So do not use the size of this
   structure as the actual data size, use 13 Bytes (i.e DATA_IHDR_SIZE macro).
 */
typedef struct data_IHDR {// IHDR chunk data 
    U32 width;        /* width in pixels, big endian   */
    U32 height;       /* height in pixels, big endian  */
    U8  bit_depth;    /* num of bits per sample or per palette index.
                         valid values are: 1, 2, 4, 8, 16 */
    U8  color_type;   /* =0: Grayscale; =2: Truecolor; =3 Indexed-color
                         =4: Greyscale with alpha; =6: Truecolor with alpha */
    U8  compression;  /* only method 0 is defined for now */
    U8  filter;       /* only method 0 is defined for now */
    U8  interlace;    /* =0: no interlace; =1: Adam7 interlace */
} *data_IHDR_p;

typedef struct chunk {
    U32 length;  /* length of data in the chunk, host byte order */
    U8  type[4]; /* chunk type */
    U8  *p_data; /* pointer to location where the actual data are */
    U32 crc;     /* CRC field  */
} *chunk_p;

typedef struct ihdr_chunk {
    U32 length;
    U8  type[4];
    data_IHDR_p  *p_data;
    U32 crc;
} *ihdr_chunk_p; 

/* A simple PNG file format, three chunks only*/
typedef struct simple_PNG {
    ihdr_chunk_p p_IHDR;
    chunk_p p_IDAT;  /* only handles one IDAT chunk */  
    chunk_p p_IEND;
} *simple_PNG_p;

/** @brief Determine if a file is a png or not given file path
 *
 *  @param fpath The string of relative or absolute file path.
 *  @return 0 if the file is a png, -1 otherwise
 */
int is_png(char* buf);

/* Getters and setters for PNG dimensions*/
int get_png_height(struct data_IHDR *buf);
int get_png_width(struct data_IHDR *buf);
void set_png_height(struct data_IHDR *buf, int new_height);
void set_png_width(struct data_IHDR *buf, int new_width);