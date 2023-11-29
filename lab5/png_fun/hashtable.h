#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <search.h>

char* url_to_key(char * url);
int ht_search_url(char * url);
int ht_add_url(char * url, char ** hash_data);
void ht_cleanup(char ** hash_data);