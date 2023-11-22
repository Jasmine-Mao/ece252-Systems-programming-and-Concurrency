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
#include <search.h>
#include <errno.h>
#include <stdatomic.h>

#include <libxml/HTMLparser.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/uri.h>
#include "findpng2.h"
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
char** visited_urls; //for writing purposes
atomic_int png_count = 0;
char seed_url[256];
int max_png = 50;

pthread_mutex_t ht_lock;
pthread_mutex_t frontier_lock;

typedef struct data_buf {
    char* buf;
    size_t size;
    size_t max_size;
    int seq;
} DATA_BUF;

// FUNCTION DECLERATIONS
size_t header_write_callback(char* recv, size_t size, size_t nmemb, void* user_data);
size_t data_write_callback(char* recv, size_t size, size_t nmemb, void* user_data);
CURL *easy_handle_init(DATA_BUF *ptr, const char *url);
htmlDocPtr mem_getdoc(char *buf, int size, const char *url);
xmlXPathObjectPtr getnodeset (xmlDocPtr doc, xmlChar *xpath);
int find_http(char *buf, int size, int follow_relative_links, const char *base_url);
int process_html(CURL *curl_handle, DATA_BUF *recv_buf);
int process_png(CURL *curl_handle, DATA_BUF *recv_buf);
int process_data(CURL *curl_handle, DATA_BUF *recv_buf);

// TODO: @<JASMINE> curl callbacks!

// insert curl functions here :P

size_t header_write_callback(char* recv, size_t size, size_t nmemb, void* user_data){
    size_t real_size = size * nmemb;
    DATA_BUF* data_fetched = user_data;

    if(real_size > strlen(ECE_252_HEADER) &&  strncmp(recv, ECE_252_HEADER, strlen(ECE_252_HEADER))){
        data_fetched->seq = atoi(recv + strlen(ECE_252_HEADER));
    }
    return real_size;
}

size_t data_write_callback(char* recv, size_t size, size_t nmemb, void* user_data){
    size_t real_size = size * nmemb;

    DATA_BUF* data_fetched = user_data;

    memcpy(data_fetched->buf + data_fetched->size, recv, real_size);
    data_fetched->size += real_size;
    // PRINTING HERE WOULD JUST SPIT OUT EVERYTHING IN THE HTML DOC
    return real_size;

    // taken directly from paster2
}

CURL *easy_handle_init(DATA_BUF *ptr, const char *url)
{
    CURL *curl_handle = NULL;

    if ( ptr == NULL || url == NULL) {
        return NULL;
    }

    /* init a curl session */
    curl_handle = curl_easy_init();

    if (curl_handle == NULL) {
        fprintf(stderr, "curl_easy_init: returned NULL\n");
        return NULL;
    }

    /* specify URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, data_write_callback); 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)ptr);

    curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, header_write_callback); 
    curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void *)ptr);

    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "ece252 lab4 crawler");

    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_UNRESTRICTED_AUTH, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl_handle, CURLOPT_ACCEPT_ENCODING, "");

    curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(curl_handle, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPAUTH, CURLAUTH_ANY);

    return curl_handle;
}

htmlDocPtr mem_getdoc(char *buf, int size, const char *url)
{
    int opts = HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | \
               HTML_PARSE_NOWARNING | HTML_PARSE_NONET;
    htmlDocPtr doc = htmlReadMemory(buf, size, url, NULL, opts);
    
    if ( doc == NULL ) {
        fprintf(stderr, "Document not parsed successfully.\n");
        return NULL;
    }
    return doc;
}

xmlXPathObjectPtr getnodeset (xmlDocPtr doc, xmlChar *xpath)
{
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;

    context = xmlXPathNewContext(doc);
    // sets the context of the xml path to the document just retrieved from the link
    if (context == NULL) {
        printf("Error in xmlXPathNewContext\n");
        return NULL;
    }
    result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);
    if (result == NULL) {
        printf("Error in xmlXPathEvalExpression\n");
        return NULL;
    }
    if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
        xmlXPathFreeObject(result);
        printf("No result\n");
        return NULL;
    }
    return result;
}

int find_http(char *buf, int size, int follow_relative_links, const char *base_url) {
    int i;
    htmlDocPtr doc;
    xmlChar *xpath = (xmlChar*) "//a/@href";
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    xmlChar *href;
		
    if (buf == NULL) {
        return 1;
    }

    doc = mem_getdoc(buf, size, base_url);
    result = getnodeset (doc, xpath);
    if (result) {
        nodeset = result->nodesetval;
        for (i=0; i < nodeset->nodeNr; i++) {
            href = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode, 1);
            if ( follow_relative_links ) {
                xmlChar *old = href;
                href = xmlBuildURI(href, (xmlChar *) base_url); 
                xmlFree(old);
            }
            if ( href != NULL && !strncmp((const char *)href, "http", 4) ) {
                pthread_mutex_lock(&ht_lock);
                if(ht_search_url((char*)href) == 0){
                    printf("this is a unique url: %s\n", (char*)href);
                    ht_add_url((char*)href);
                    frontier_push(urls_frontier, (char*)href);
                }
                else{
                    printf("ALREADY IN THE HT: %s \n", href);
                }
                pthread_mutex_unlock(&ht_lock);
            }
            xmlFree(href);
        }
        xmlXPathFreeObject (result);
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return 0;
}

int process_html(CURL *curl_handle, DATA_BUF *recv_buf) {
    int follow_relative_link = 1;
    char *url = NULL; 

    curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &url);
    return find_http(recv_buf->buf, recv_buf->size, follow_relative_link, url); 
    }

int process_png(CURL *curl_handle, DATA_BUF *recv_buf) {
    char *eurl = NULL;
    curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &eurl);
    
    if ((eurl != NULL) && (ht_search_url(eurl) == 0) && (recv_buf->seq != -1)) {
        // here is erul is not null AND the url is not already in the ht AND the thing passed is a png
        pthread_mutex_lock(&ht_lock);
        ht_add_url(eurl);
        pthread_mutex_unlock(&ht_lock);
        // ^add the eurl to the ht
        printf("The PNG url is: %s\n", eurl);
        png_count++;
    }
    return 0;
}

int process_data(CURL *curl_handle, DATA_BUF *recv_buf) {
    CURLcode res;
    // char fname[256];
    // pid_t pid =getpid();
    long response_code;

    res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
    if ( res == CURLE_OK ) {
	    printf("Response code: %ld\n", response_code);
    }

    if ( response_code >= 400 ) {
    	fprintf(stderr, "Error.\n");
        return 1;
    }

    char *ct = NULL;
    res = curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &ct);
    if ( res == CURLE_OK && ct != NULL ) {
    	printf("Content-Type: %s, len=%ld\n", ct, strlen(ct));
    } else {
        fprintf(stderr, "Failed obtain Content-Type\n");
        return 2;
    }

    if ( strstr(ct, CT_HTML) ) { 
        printf("FOUND AN HTML PAGE!!\n");
        return process_html(curl_handle, recv_buf);
    } else if ( strstr(ct, CT_PNG) ) {
        printf("FOUND A PNG!!\n");
        return process_png(curl_handle, recv_buf);
    }
    return 0;
}

// TODO: @<JASMINE> thread routine
void *visit_url(void * arg){
    DATA_BUF buf;
    buf.max_size = 1048576; // WHAT SHOULD THE MAX SIZE BE?
    buf.buf = malloc(1048576);
    buf.size = 0;
    buf.seq = -1;
    //while(){
        char *temp = NULL;
        pthread_mutex_lock(&frontier_lock);
        temp = frontier_pop(urls_frontier);
        pthread_mutex_unlock(&frontier_lock);

        printf("POPPED %s\n", temp);
        // CURL* curl_handle = easy_handle_init(&buf, temp);

        // if(curl_handle == NULL){
        //     return -1;
        // }
        // if(process_data(curl_handle, &buf) == 0){
        //     // successful return

        //     // add to the visited url array
        // }
        //printf("%s\n", temp);
        // FRONTIER LOCK
        // frontier_pop()
        // FRONTIER UNLOCK
        // add to visited url array

        // CURL* curl_handle - easy_handle_init(buf, /*URL THAT WE POPPED OFF THE FRONTIER*/)
        // if(curl_handle == 0){
        //     break;
        // }
        // process_data(curl_handle, buf);
    //}

    
    // while frontiers_counter is non zero or while our png counter < M <-- IMPORTANT: im not sure if this implementation is sound! 
    //                                                                      there may be some synchronization effort required so that things terminate gracefully. see foot note (lol)
    // pop from frontiers, this url should be unvisited 
    // curl stuff, up to you
    // ht_search_url()
    // add url to visited_urls (table we're going to use to write into logfile [not the same as hashtable])
    // push onto stack if needed, add a new hash table entry etc etc
    // for pngs, lets add to the unique_pngs array
    // decrement frontiers counter (as we have now visited the url)

    

    return 0;
}

int write_results(char * logfile_name){ //rewrite to append instead of write
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
            if (visited_urls[i] != NULL){        // this inefficient, easy workaround is just to create a seperate container (array) w/ an end index
                fprintf(logfile, "%s\n", visited_urls[i]);  // variable, and add to it everytime we find a unique url. if this causes time influence @ the end we can do the workaround
            }
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
    curl_global_init(CURL_GLOBAL_DEFAULT);
    int c;
    int num_threads = 1;
    char* logfile_name = NULL;
    
    // default to 1 thread looking for 50 pngs; no logging

    if(argc < 2){
        fprintf(stderr, "Incorrect arguments!\n");
        return -1;
    }

    if (hcreate(MAX_URLS) == 0){
        printf("Visited URLs table could not be made!\n");
        return errno;
    }

    visited_urls = malloc(sizeof(char*)*500);

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
            max_png = strtoul(optarg, NULL, 10);
            if(max_png <= 0){
                printf("please enter valid number of max images\n");
                return -1;
            }
            printf("arg passed for num images: %d\n", max_png);
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
    strcpy(seed_url, argv[argc-1]);
    printf("INITIAL URL %s\n", seed_url);
    // gets seed url from args passed
    // this will be the url to be searched through recursively

    // TODO: @<JASMINE> argument parsing & initialization
    // frontiers_init here, push the seed_url onto the stack as the first element
    // (i made a quick definiton for frontiers_init already)
    // hash table creation here
    // initialize unique_pngs array, just malloc(m * sizeof(char *))
    // initialize conditional variables, eg. frontiers_counter etc.

    urls_frontier = malloc(sizeof(FRONTIER));
    frontier_init(urls_frontier);

    unique_pngs = malloc(max_png * sizeof(char *));
    // list of png links

    pthread_mutex_init(&ht_lock, NULL);
    pthread_mutex_init(&frontier_lock, NULL);
    // make a unique png mutex
    // INIT ALL MUTEXES

    // MAIN STARTS THE INITIAL CURL WITH THE SEED URL



    // TODO: @EVELYN thread creation, joining and cleanup
    // if t == 1, just jump to visit_url [seed]
    // else, create threads, visit_url will be our thread routine
    // threads join, cleanup
     
    DATA_BUF buf;
    buf.max_size = 1048576; // WHAT SHOULD THE MAX SIZE BE?
    buf.buf = malloc(1048576);
    buf.size = 0;
    buf.seq = -1;
    CURL* curl_handle = easy_handle_init(&buf, seed_url);

    if(curl_handle == NULL){
        printf("curl failed L\n");
        return -1;
    }

    CURLcode res = curl_easy_perform(curl_handle);
    // perform all the curl stuff specified in the easy handle init
    // MAIN SHOULD PUT ALL THE FETCHED URL'S FROM THE SEED INTO THE HASH, THEN INTO THE FRONTIER
    

    if(res == CURLE_OK){
        printf("successfully curled. starting threads...\n");
        process_data(curl_handle, &buf);
        process_data(curl_handle, &buf);
        visit_url(NULL);

        // if (num_threads == 1){
        //     visit_url(NULL);
        // }
        // else{
        //     pthread_t threads[num_threads];
        //     int threads_res[num_threads];
        //     for (int i = 0; i < num_threads; i++){
        //         threads_res[i] = pthread_create(threads + i, NULL, visit_url, NULL);
        //     }
        //     // C O N D I T I O N A L  F O R  K I L L I N G  T H R E A D S

        //     for (int i = 0; i < num_threads; i++){
        //         if (threads_res[i] == 0){
        //             pthread_join(threads[i], NULL);
        //         }
        //     }
        // }
    }
    else{
        return -1;
    }

    curl_easy_cleanup(curl_handle);

    free(urls_frontier->stack);
    free(urls_frontier);
    
    hdestroy();
    write_results(logfile_name);
    free(unique_pngs);

    free(visited_urls);

    pthread_mutex_destroy(&ht_lock);
    pthread_mutex_destroy(&frontier_lock);

    printf("done\n");

    return 0;
}