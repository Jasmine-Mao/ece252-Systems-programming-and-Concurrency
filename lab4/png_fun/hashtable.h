#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

char* url_to_key(char * url);
int ht_search_url(char * url);
int ht_add_url(char * url);