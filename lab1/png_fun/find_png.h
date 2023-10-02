/**
 * @file: find_png.h
 * @brief: function definition to locate all pngs in a directory
 */

#pragma once

#include <dirent.h>

int find_png(DIR *directory, char filePath[], int numPNGs);