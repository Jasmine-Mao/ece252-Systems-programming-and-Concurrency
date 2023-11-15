// from starter code
#include <stdio.h>
#include <stdlib.h>
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

// GLOBAL VARIABLES
FRONTIER * urls_frontier;
char ** unique_pngs;    // this is a pointer to an array of pointers (where each item is a pointer to a char array (string)

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
int visit_url(void * arg){

    
    // while frontiers_counter is non zero or while our png counter < M <-- IMPORTANT: im not sure if this implementation is sound! 
    //                                                                      there may be some synchronization effort required so that things terminate gracefully. see foot note (lol)
    // pop from frontiers, this url should be unvisited 

    // curl stuff, up to you

    // ht_search_url()
    // push onto stack if needed, add a new hash table entry etc etc
    // for pngs, lets add to the unique_pngs array

    // decrement frontiers counter (as we have now visited the url)

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
    FILE* log_file;
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
            log_file = fopen(optarg, "w+");
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


    // TODO: @<name_here> thread creation, joining and cleanup
    // if t == 1, just jump to visit_url [seed]
    // else, create threads, visit_url will be our thread routine
    // threads join, cleanup

    // TODO: @<name_here>
    // write png_urls.txt
    // perform optional write to LOGFILE

    return 0;
}