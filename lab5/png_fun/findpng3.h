#pragma once

#include <libxml/HTMLparser.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/uri.h>

typedef struct data_buf {
    char* buf;
    size_t size;
    size_t max_size;
} DATA_BUF;

int write_results(char * logfile_name);
int is_png(char* buf);