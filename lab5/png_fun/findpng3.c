#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <curl/curl.h>

#include <getopt.h>
#include <search.h>
#include <errno.h>

#include <time.h>
#include <sys/time.h>

#include "findpng3.h"
#include "frontier_stack.h"
#include "hashtable.h"

#define MAX_URLS 500
#define ECE_252_HEADER "X-Ece252-Fragment: "

#define CT_PNG  "image/png"
#define CT_HTML "text/html"
#define CT_PNG_LEN  9
#define CT_HTML_LEN 9

// GLOBAL VARIABLES
FRONTIER * urls_frontier;
char ** unique_pngs;    // this is a pointer to an array of pointers (where each item is a pointer to a char array (string)
char ** visited_urls; //for writing purposes
char ** hash_data;

char seed_url[256];
int max_png = 50;

int num_png_obtained = 0;
int num_urls_visited = 0;

int write_results(char * logfile_name){ //rewrite to append instead of write
    FILE *png_urls = fopen("png_urls.txt", "w+");
    if (!png_urls) {
        perror("fopen");
        fclose(png_urls);
        return -1;
    }
    int min_pngs = 0;
    if (num_png_obtained < max_png){
        min_pngs = num_png_obtained;
    } else {
        min_pngs = max_png;
    }

    if (min_pngs == 0){
        fclose(png_urls);
    }
    else {
        for (int i = 0; i < min_pngs; i++){
            //printf("%s\n", unique_pngs[i]);
            fprintf(png_urls, "%s\n", unique_pngs[i]);    // small but maybe wanna confirm if we can have a new line at the end of this file lol
        }
        fclose(png_urls);
    }

    if (logfile_name != NULL){
        FILE *logfile = fopen(logfile_name, "w+");
        for (int i = 0; i < num_urls_visited; i++){
            if (visited_urls[i] != NULL){        // this inefficient, easy workaround is just to create a seperate container (array) w/ an end index
                //printf("num %d, url %s\n", i, visited_urls[i]);
                fprintf(logfile, "%s\n", visited_urls[i]);  // variable, and add to it everytime we find a unique url. if this causes time influence @ the end we can do the workaround
            }
        }
        fclose(logfile);
    }

    for (int i = 0; i < num_png_obtained; i++){
        free(unique_pngs[i]);
    }
    for (int i = 0; i < num_urls_visited; i++){
        free(visited_urls[i]);
    }
    return 0;
}
    
int main(int argc, char * argv[]){
    // findpng3 -t 10 -m 20 -v log.txt http://ece252-1.uwaterloo.ca/lab4
    int c;
    char* logfile_name = NULL;
    

}