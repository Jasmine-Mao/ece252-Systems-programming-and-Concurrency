/**
 * @file: cat_png.h
 * @brief: function definitions for concatenating pngs
 */

#pragma once

#include "png_utils/png_info.h"

/** @brief Write PNG data to all.png
 *
 *  @param png_to_write Source of png data to write.
 *  @param idat_data_size The size of the PNG's IDAT data.
 *  @return 0 if success, -1 otherwise
 */
//int write_png(struct simple_PNG *png_to_write, size_t idat_data_size);

/** @brief Concatenate PNG IDAT data
 *
 *  @param fpath The string of relative or absolute file path.
 *  @param idat Destination of concatenated IDAT data.
 *  @param current_idat_end Position of last written IDAT data within idat.
 *  @return updated current_idat_end value
 */
//size_t concatenate_idat(const char *fpath, U8 *idat, size_t current_idat_end);

/** @brief Perform PNG concatenation
 *  @param png_count Number of pngs to concatenate
 *  @param argv Arguments forwarded from main
 *  @return 0 if success, -1 otherwise
 */
//int concatenate_pngs(int png_count, char* argv[]);

void create_ihdr_chunk(ihdr_chunk_p dest);
void create_iend_chunk(chunk_p dest);

