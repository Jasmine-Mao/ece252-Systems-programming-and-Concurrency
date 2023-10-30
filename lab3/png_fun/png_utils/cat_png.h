/**
 * @file: cat_png.h
 * @brief: function definitions for concatenating pngs
 */

#ifndef CAT_PNG_H
#define CAT_PNG_H

#include "png_info.h"
#include <stdatomic.h>

#define PNG_WIDTH 400       /* width of PNGs on server */
#define PNG_HEIGHT 300      /* height of PNGs on server */
#define STRIP_HEIGHT 6      /* height of PNG strips on server */
#define INFLATED_DATA_SIZE (PNG_HEIGHT * (PNG_WIDTH * 4 + 1) )

/* Global container to store inflated idat data */
atomic_uchar idat_data[INFLATED_DATA_SIZE];

/** @brief Write PNG data to all.png
 *
 *  @param png_to_write Source of png data to write.
 *  @param idat_data_size The size of the PNG's IDAT data.
 *  @return 0 if success, -1 otherwise
 */
int write_png(struct simple_PNG *png_to_write, size_t idat_data_size);

/** @brief Concatenate PNG IDAT data
 *
 *  @param fpath The string of relative or absolute file path.
 *  @param idat Destination of concatenated IDAT data.
 *  @param current_idat_end Position of last written IDAT data within idat.
 *  @return updated current_idat_end value
 */
size_t concatenate_idat(const char *fpath, U8 *idat, size_t current_idat_end);

/** @brief Perform PNG concatenation
 *  @return 0 if success, -1 otherwise
 */
int concatenate_png();

/** @brief Create PNG IHDR chunk for a 400 x 300 image.
 *
 *  @param dest Destination of new IHDR chunk
 */
void create_ihdr_chunk(struct ihdr_chunk * ihdr_buf);

/** @brief Create PNG IEND chunk
 *
 *  @param dest Destination of new IEND chunk
 */
void create_iend_chunk(struct chunk * iend_buf);

#endif
