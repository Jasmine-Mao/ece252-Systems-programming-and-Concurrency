/**
 * @file: cat_png.h
 * @brief: function definitions for concatenating pngs
 */

#pragma once

#include "png_utils/png_info.h"

/** @brief Read in PNG IHDR data fields
 *
 *  @param fpath The string of relative or absolute file path.
 *  @param data_reading The location where the IHDR data is stored.
 *  @return 0 if success, -1 otherwise
 */
int read_ihdr(const char *fpath, data_IHDR_p data_reading);

/** @brief Read PNG height
 *
 *  @param fpath The string of relative or absolute file path.
 *  @return 0 if success, -1 otherwise
 */
int read_height(const char *fpath);

/** @brief Write PNG data to all.png
 *
 *  @param png_to_write Source of png data to write.
 *  @param idat_data_size The size of the PNG's IDAT data.
 *  @return 0 if success, -1 otherwise
 */
int write_png(struct simple_PNG *png_to_write, size_t idat_data_size);

/** @brief Concatenate PNG IDAT data
 *
 *  @param png_to_write Source of png data to write.
 *  @param idat_data_size The size of the PNG's IDAT data.
 *  @return 0 if success, -1 otherwise
 */
size_t concatenate_idat(const char *fpath, U8 *idat, size_t current_idat_end);

/** @brief Perform PNG concatenation
 *  @param fpath The string of relative or absolute file path.
 *  @param idat Destination of concatenated IDAT data.
 *  @param current_idat_end Position of last written IDAT data within idat.
 *  @return 0 if success, -1 otherwise
 */
int concatenate_pngs(int png_count, char* argv[]);

/** @brief Verify if a file is a valid PNG.
 *
 *  @param fpath The string of relative or absolute file path.
 *  @return 0 if png is valid, -1 otherwise
 */
int verify_png(const char *fpath);