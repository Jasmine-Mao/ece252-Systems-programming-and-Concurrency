// from starter code
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <curl/curl.h>
#include <pthread.h>

#include <getopt.h>

/*#include <libxml/HTMLparser.h> //these aren't working for some reason
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/uri.h>*/
#include "findpng2.h"
#include "frontier_stack.h"

#define MAX_URLS 500

// GLOBAL VARIABLES
FRONTIER * urls_frontier;
char ** unique_pngs;    // this is a pointer to an array of pointers (where each item is a pointer to a char array (string)
int png_count = 0;

// TODO: @<JASMINE> curl callbacks!

// insert curl functions here :P

// TODO: @<name_here> Hash table operations (ht_search_url and ht_add_url, maybe also add a ht_init?)

int url_to_key(char * url){
    // parses url string into a numerical representation (sum of ascii values) to be used as the key
    return 0;
}

int ht_search_url(char * url){
    // gets key from url, and invokes hsearch() with said key
    // return 1 if the url exists in the hash table, 0 otherwise
    return 0;
}

int ht_add_url(char * url){
    // add a url to our hash table: get its key from its string url then set corresponding value (youre gonna have to check the hsearch(3) man page)
    // for the hash table stuff)
    return 0;
}

// TODO: @<JASMINE> thread routine
void *visit_url(void * arg){

    CURL *curl_handle = curl_easy_init();
    
    // while frontiers_counter is non zero or while our png counter < M <-- IMPORTANT: im not sure if this implementation is sound! 
    //                                                                      there may be some synchronization effort required so that things terminate gracefully. see foot note (lol)
    // pop from frontiers, this url should be unvisited 
    // curl stuff, up to you
    // ht_search_url()
    // push onto stack if needed, add a new hash table entry etc etc
    // for pngs, lets add to the unique_pngs array
    // decrement frontiers counter (as we have now visited the url)

    

    return NULL;
}

int write_results(char * logfile_name){
    FILE *png_urls = fopen("png_urls.txt", "w");
    if (!png_urls) {
        perror("fopen");
        fclose(png_urls);
        return -1;
    }
    
    if (png_count == 0){
        fclose(png_urls);
    }
    else {
        for (int i = 0; i < png_count; i++){
            fprintf(png_urls, "%s\n", unique_pngs[i]);    // small but maybe wanna confirm if we can have a new line at the end of this file lol
        }
        fclose(png_urls);
    }

    if (logfile_name != NULL){
        FILE *logfile = fopen(logfile_name, "w");
        for (int i = 0; i < MAX_URLS; i++){
            /*if (hashtable[i] != NULL){        // this inefficient, easy workaround is just to create a seperate container (array) w/ an end index
                fprintf(logfile, "%s\n", hashtable[i]);  // variable, and add to it everytime we find a unique url. if this causes time influence @ the end we can do the workaround
            }*/
        }
        fclose(logfile);
    }

    return 0;
}

// foot note: this is from the lab manual
/*  Another subtle difficulty is to know when to terminate the program. The pro-
    gram should terminate either when there are no more URLs in the URLs frontier
    or the user specified number of PNG URLs have been found. You may need some
    shared counters to keep track of information such as how many PNG URLs have
    been found and how many threads are waiting for a new URL   */
// foot note end

int main(int argc, char * argv[]){
    //./findpng2 –t=num –m=num –v=filename seed_url -- t, m, v are opt
    int c;
    int num_threads = 1;
    int num_pngs = 50;
    char* logfile_name = NULL;
    // default to 1 thread looking for 50 pngs; no logging

    if(argc < 2){
        fprintf(stderr, "Incorrect arguments!\n");
        return -1;
    }

    /*OPTION STUFF*/
    while((c = getopt(argc, argv, "t:m:v:")) != -1){
        switch (c)
        {
        case 't':
            num_threads = strtoul(optarg, NULL, 10);
            if(num_threads <= 0){
                printf("please enter valid thread number\n");
                return -1;
            }
            printf("arg passed for threads: %d\n", num_threads);
            break;
        
        case 'm':
            num_pngs = strtoul(optarg, NULL, 10);
            if(num_pngs <= 0){
                printf("please enter valid number of max images\n");
                return -1;
            }
            printf("arg passed for num images: %d\n", num_pngs);
            break;
        case 'v':
            //memcpy(log_file, optarg, strlen(optarg));
            printf("file to write log to: %s\n", optarg);
            logfile_name = optarg;
            // creates a file for logging; ***must be closed at the end of any operation using the log
            break;
        default:
            break;
        }        
    }



    // TODO: @<JASMINE> argument parsing & initialization
    // frontiers_init here, push the seed_url onto the stack as the first element
    // (i made a quick definiton for frontiers_init already)
    // hash table creation here
    // initialize unique_pngs array, just malloc(m * sizeof(char *))
    // initialize conditional variables, eg. frontiers_counter etc.

    urls_frontier = malloc(sizeof(FRONTIER));
    frontier_init(urls_frontier);

    unique_pngs = malloc(num_pngs * sizeof(char *));

    curl_global_init(CURL_GLOBAL_DEFAULT);

    // TODO: @EVELYN thread creation, joining and cleanup
    // if t == 1, just jump to visit_url [seed]
    // else, create threads, visit_url will be our thread routine
    // threads join, cleanup

    if (num_threads == 1){
        visit_url(NULL);
    }
    else{
        pthread_t threads[num_threads];
        int threads_res[num_threads];
        for (int i = 0; i < num_threads; i++){
            threads_res[i] = pthread_create(threads + i, NULL, visit_url, NULL);
        }

        for (int i = 0; i < num_threads; i++){
            if (threads_res[i] == 0){
                pthread_join(threads[i], NULL);
            }
        }
    }

    free(urls_frontier->stack);
    free(urls_frontier);

    // TODO: @EVELYN
    // write png_urls.txt
    // perform optional write to LOGFILE
    write_results(logfile_name);

    return 0;
}