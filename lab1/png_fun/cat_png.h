/**
 * @file: cat_png.h
 * @brief: function definitions for concatenating pngs
 */

#pragma once

#include "png_utils/png_info.h"

data_IHDR_p read_ihdr(const char *fpath);
int concatenate_idat(const char *fpath, chunk_p *idat);
int concatenate_pngs(int png_count, char* argv[]);
int verify_png(const char *fpath);