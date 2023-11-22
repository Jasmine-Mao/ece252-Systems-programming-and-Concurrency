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
int png_count = 0;
char seed_url[256];

pthread_mutex_t ht_lock;

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
// char* url_to_key(char * url);
// int ht_search_url(char * url);
// int ht_add_url(char * url);

// for storing data fetched from the curl calls

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
        // PTR PASSED IS THE DATA_BUF WHICH WILL HOLD ALL THE CURLED STUFF
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
    // expression is the name of files in a directory
    //      they can be names of nodes, /, //, ., .., etc

    // context: "The context path refers to the location relative to the server's address which represents the name of the web application"
    // new copntexr function needs to have the context freed after, which is below
    xmlXPathFreeContext(context);
    if (result == NULL) {
        // BAD, DON'T WANT TO GET HERE
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
    // finds the links in a doc; based off the base url
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
    // gets the html file that we need to actually look at
    // creates a text file with the html code, links are taken from it and just output raw
    result = getnodeset (doc, xpath);
    if (result) {
        // IF THE GET NODE SET CALL WAS SUCCESSFUL
        nodeset = result->nodesetval;
        for (i=0; i < nodeset->nodeNr; i++) {
            href = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode, 1);
            if ( follow_relative_links ) { // if we say yes, we want to follow relative links, go here --> i think we will want to follow these relative paths too
                xmlChar *old = href;
                href = xmlBuildURI(href, (xmlChar *) base_url); // stitches together the relative url to make a full, absolute path
                xmlFree(old);
            }
            if ( href != NULL && !strncmp((const char *)href, "http", 4) ) {
                pthread_mutex_lock(&ht_lock);
                // lock when we read and write to the ht
                if(ht_search_url((char*)href) == 0){
                    // get here if the link IS NOT IN THE HASH TABLE
                    printf("this is a unique url: %s\n", href);
                    ht_add_url((char*)href);
                    // add url to the frontier
                    frontier_push(urls_frontier, (char*)href);
                }
                else{
                    printf("ALREADY IN THE HT: %s \n", href);
                }
                pthread_mutex_unlock(&ht_lock);

                // printf("href: %s\n", href);
                // ADD TO THE HASHTABLE


                // here is where the url printing actually happens
                // makes sure that the href passed is an http, then prints it out
                // this can be modified to add to the frontier buf/stack/queue however we end up implementing it
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
    // COME HERE IF THE THING WE GOT FROM A URL IS ANOTHER HTML PAGE
    // char fname[256];
    int follow_relative_link = 1;
    char *url = NULL; 
    // pid_t pid =getpid();

    curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &url);
    return find_http(recv_buf->buf, recv_buf->size, follow_relative_link, url); 
    // MODIFIED TO RETURN THE SUCCESS STATUS OF THE FIND_HTTP CALL

        // this is where we get all the links
        // can modify this to be called elsewhere by threads
    // sprintf(fname, "./output_%d.html", pid);
    //     // outputs the html stuff to a file
    // return write_file(fname, p_recv_buf->buf, p_recv_buf->size);
    }

int process_png(CURL *curl_handle, DATA_BUF *recv_buf) {
    // pid_t pid =getpid();
    // char fname[256];
    char *eurl = NULL;          /* effective URL */
    curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &eurl);
        // gets the effective url from the curl handle
        // effective URL is the last used url
            // see: https://curl.se/libcurl/c/curl_easy_getinfo.html
        // puts the last used url into the eurl pointer
            // if there was a last used url print out the below text
    if ( eurl != NULL /*&& IS_PNG*/) {
        // here is is a png and effective url is not null
        printf("The PNG url is: %s\n", eurl);
        // add link to hash table
        // write to the log
    }
    return 0;
    // return 0 regardless of whatever was passed was a png or not
    // can modify this later to return some error code if the thing passed was not a url
}

int process_data(CURL *curl_handle, DATA_BUF *recv_buf) {
    CURLcode res;
    // char fname[256];
    // pid_t pid =getpid();
    long response_code;

    res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
    // see for response codes: https://everything.curl.dev/http/response
    if ( res == CURLE_OK ) {
	    printf("Response code: %ld\n", response_code);
    }

    if ( response_code >= 400 ) { 
        // 4xx: the client asked for something the server could not or would not deliver
        // 5xx: there is a problem in the server
            // basically errors
    	fprintf(stderr, "Error.\n");
        return 1;
    }

    char *ct = NULL;
    res = curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &ct);
        // return content type header
        // should be text/html if we got to an html text page i think?
    if ( res == CURLE_OK && ct != NULL ) {
    	printf("Content-Type: %s, len=%ld\n", ct, strlen(ct));
    } else {
        fprintf(stderr, "Failed obtain Content-Type\n");
        return 2;
    }

    if ( strstr(ct, CT_HTML) ) {    // finds first occurence of CT_HTML in ct; returns a pointer to the first occurence in ct if successful
        printf("FOUND AN HTML PAGE!!\n");
        return process_html(curl_handle, recv_buf);
        // this is if the thing we have passed gets an html page --> grab all the stuff from the html page and pass them to the hash table
    } else if ( strstr(ct, CT_PNG) ) {
        printf("FOUND A PNG!!\n");
        //return process_png(curl_handle, p_recv_buf);
        // this is a png
    } 
    // else {
    //     sprintf(fname, "./output_%d", pid);
    // }

    // return write_file(fname, p_recv_buf->buf, p_recv_buf->size);
    return 0;
}

// TODO: @Iman Hash table operations (ht_search_url and ht_add_url, maybe also add a ht_init?)
    // parses url string into a numerical representation (sum of ascii values) to be used as the key
    /*
    for length of url
        get url character at index
        convert to ascii
        add to sum
    divide by number of characters
    mod 10 and store
    add one and mod 10 and store
    
    */

// TODO: @<JASMINE> thread routine
void *visit_url(void * arg){
    DATA_BUF buf;
    buf.max_size = 1048576; // WHAT SHOULD THE MAX SIZE BE?
    buf.buf = malloc(1048576);
    buf.size = 0;
    buf.seq = -1;
    while(/*some condition*/){
        // frontier_pop()

        CURL* curl_handle - easy_handle_init(buf, /*URL THAT WE POPPED OFF THE FRONTIER*/)
        if(curl_handle == 0){
            break;
        }
        process_data(curl_handle, buf);
        curl_easy_cleanup(curl_handle);
    }
    
    // while frontiers_counter is non zero or while our png counter < M <-- IMPORTANT: im not sure if this implementation is sound! 
    //                                                                      there may be some synchronization effort required so that things terminate gracefully. see foot note (lol)
    // pop from frontiers, this url should be unvisited 
    // curl stuff, up to you
    // ht_search_url()
    // add url to visited_urls (table we're going to use to write into logfile [not the same as hashtable])
    // push onto stack if needed, add a new hash table entry etc etc
    // for pngs, lets add to the unique_pngs array
    // decrement frontiers counter (as we have now visited the url)

    

    return NULL;
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
    int num_pngs = 50;
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

    unique_pngs = malloc(num_pngs * sizeof(char *));

    pthread_mutex_init(&ht_lock, NULL);

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

    printf("done\n");

    return 0;
}