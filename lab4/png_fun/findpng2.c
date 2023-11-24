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

char seed_url[256];
int max_png = 50;
int num_threads = 1;

int THREADS_STOP = 0;

atomic_int num_urls_visited = 0;
atomic_int num_threads_waiting = 0;
atomic_int num_png_obtained = 0;

pthread_mutex_t ht_lock;
pthread_mutex_t frontier_lock;
pthread_mutex_t threads_lock;
pthread_mutex_t add_png_lock;
pthread_mutex_t add_url_lock;
pthread_mutex_t frontier_empty_lock;

pthread_mutex_t thread_stop_lock;

pthread_cond_t kill_threads_cond;
pthread_cond_t frontier_cond;

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
        // fprintf(stderr, "Document not parsed successfully.\n");
        return NULL;
    }
    return doc;
}

xmlXPathObjectPtr getnodeset (xmlDocPtr doc, xmlChar *xpath)
{
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;

    context = xmlXPathNewContext(doc);
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
        // printf("No result\n");
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
            if(THREADS_STOP == 1){
                xmlXPathFreeObject (result);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return 0;
            }
            href = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode, 1);
            if ( follow_relative_links ) {
                xmlChar *old = href;
                href = xmlBuildURI(href, (xmlChar *) base_url); 
                xmlFree(old);
            }
            if ( href != NULL && !strncmp((const char *)href, "http", 4) && ht_search_url((char*)href) == 0) {
                printf("this is a unique url: %s\n", (char*)href);

                pthread_mutex_lock(&ht_lock);
                // lock the ht to write to it
                ht_add_url((char*)href);
                pthread_mutex_unlock(&ht_lock);

                pthread_mutex_lock(&frontier_lock);
                // lock the frontier when trying to write
                frontier_push(urls_frontier, (char*)href);
                pthread_mutex_unlock(&frontier_lock);

                pthread_cond_broadcast(&frontier_cond);
                // broadcast to all threads waiting that there is stuff in the frontier now
                
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
    
    if ((eurl != NULL) && is_png(recv_buf->buf) == 0) {
        pthread_mutex_lock(&ht_lock);
        // lock the ht when adding a new url
        ht_add_url(eurl);
        pthread_mutex_unlock(&ht_lock);

        pthread_mutex_lock(&add_png_lock);
        unique_pngs[num_png_obtained] = strdup(eurl);
        printf("UNIQUE PNG FOUND! %s\n", unique_pngs[num_png_obtained]);
        num_png_obtained++;
        pthread_mutex_unlock(&add_png_lock);

        if(num_png_obtained == max_png){
            printf("MAX PNGS FOUND\n");
            pthread_cond_broadcast(&kill_threads_cond);
            pthread_cond_broadcast(&frontier_cond);
            return 0;
        }
    }
    return 0;
}

int process_data(CURL *curl_handle, DATA_BUF *recv_buf) {
    CURLcode res;
    long response_code;

    res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
    if ( res == CURLE_OK ) {
	    // printf("Response code: %ld\n", response_code);
    }

    if ( response_code >= 400 ) {
    	// fprintf(stderr, "Error.\n");
        return 1;
    }

    char *ct = NULL;
    res = curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &ct);
    if ( res == CURLE_OK && ct != NULL ) {
    	// printf("Content-Type: %s, len=%ld\n", ct, strlen(ct));
    } else {
        fprintf(stderr, "Failed obtain Content-Type\n");
        return 2;
    }

    if ( strstr(ct, CT_HTML) ) { 
        // printf("FOUND AN HTML PAGE!!\n");
        return process_html(curl_handle, recv_buf);
    } else if ( strstr(ct, CT_PNG) ) {
        // printf("FOUND A PNG!!\n");
        return process_png(curl_handle, recv_buf);
    }
    return 0;
}

// TODO: @<JASMINE> thread routine
void *visit_url(void * arg){    
    while(THREADS_STOP == 0){
        DATA_BUF buf;
        buf.max_size = 1048576; // WHAT SHOULD THE MAX SIZE BE?
        buf.buf = malloc(buf.max_size);
        buf.size = 0;
        buf.seq = -1;

        if(frontier_is_empty(urls_frontier)){ 
            printf("FRONTIER EMPTY!!\n");
            num_threads_waiting++;
            printf("NUM WAITING = %d\n", num_threads_waiting);
            if(num_threads_waiting == num_threads){
                printf("KILL\n");
                pthread_cond_signal(&kill_threads_cond);
                pthread_cond_broadcast(&frontier_cond);
                // here if everything else is stuck waiting for the frontier cond to be satisfied
                free(buf.buf);
                return 0;
            }

            pthread_mutex_lock(&frontier_empty_lock);
            pthread_cond_wait(&frontier_cond, &frontier_empty_lock);
            
            printf("STUFF IN THE FRONTIER AGAIN!\n");
            if(THREADS_STOP == 1){
                printf("time to die :D\n");
                pthread_mutex_unlock(&frontier_empty_lock);
                pthread_cond_broadcast(&frontier_cond);
                free(buf.buf);
                return 0;
            }
            pthread_mutex_unlock(&frontier_empty_lock);

            num_threads_waiting--;
        }

        if(THREADS_STOP != 1){
            pthread_mutex_lock(&frontier_lock);
            char temp[256];
            frontier_pop(urls_frontier, temp);
            pthread_mutex_unlock(&frontier_lock);
            if(temp != NULL){
                CURL* curl_handle = easy_handle_init(&buf, temp);

                if(curl_handle == NULL){
                    printf("CURL FAILED\n");
                    pthread_mutex_lock(&frontier_lock);
                    frontier_push(urls_frontier, temp);
                    pthread_mutex_unlock(&frontier_lock);
                    free(buf.buf);
                    return 0;
                }

                CURLcode res = curl_easy_perform(curl_handle);

                if(res == CURLE_OK){
                    process_data(curl_handle, &buf);
                    pthread_mutex_lock(&add_url_lock);
                    visited_urls[num_urls_visited] = temp;
                    num_urls_visited++;
                    pthread_mutex_unlock(&add_url_lock);
                }
                curl_easy_cleanup(curl_handle);
            }
        }
        free(buf.buf);
    }
    return 0;
}

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
            printf("%s\n", unique_pngs[i]);
            fprintf(png_urls, "%s\n", unique_pngs[i]);    // small but maybe wanna confirm if we can have a new line at the end of this file lol
        }
        fclose(png_urls);
    }

    if (logfile_name != NULL){
        FILE *logfile = fopen(logfile_name, "w+");
        for (int i = 0; i < num_urls_visited; i++){
            if (visited_urls[i] != NULL){        // this inefficient, easy workaround is just to create a seperate container (array) w/ an end index
                fprintf(logfile, "%s\n", visited_urls[i]);  // variable, and add to it everytime we find a unique url. if this causes time influence @ the end we can do the workaround
            }
        }
        fclose(logfile);
    }

    return 0;
}

int main(int argc, char * argv[]){
    //./findpng2 –t=num –m=num –v=filename seed_url -- t, m, v are opt
    curl_global_init(CURL_GLOBAL_DEFAULT);
    int c;
    char* logfile_name = NULL;

    if(argc < 2){
        fprintf(stderr, "Incorrect arguments!\n");
        return -1;
    }
    if (hcreate(MAX_URLS) == 0){
        printf("Visited URLs table could not be made!\n");
        return errno;
    }

    visited_urls = malloc(sizeof(char*)*500);   // visited urls array
    xmlInitParser();

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
            max_png = (int)strtoul(optarg, NULL, 10);
            if(max_png <= 0){
                printf("please enter valid number of max images\n");
                return -1;
            }
            printf("arg passed for num images: %d\n", max_png);
            break;
        case 'v':
            printf("file to write log to: %s\n", optarg);
            logfile_name = optarg;
            break;
        default:
            break;
        }        
    }
    
    strcpy(seed_url, argv[argc-1]);
    printf("INITIAL URL %s\n", seed_url);

    urls_frontier = malloc(sizeof(FRONTIER));
    frontier_init(urls_frontier);

    // printf("%ld", sizeof(char*) * max_png);
    unique_pngs = malloc(sizeof(char*) * max_png);

    pthread_mutex_init(&ht_lock, NULL);
    pthread_mutex_init(&frontier_lock, NULL);
    pthread_mutex_init(&threads_lock, NULL);
    pthread_mutex_init(&add_png_lock, NULL);
    pthread_mutex_init(&add_url_lock, NULL);
    pthread_mutex_init(&frontier_empty_lock, NULL);

    pthread_cond_init(&kill_threads_cond, NULL);
    pthread_cond_init(&frontier_cond, NULL);

    // TODO: @EVELYN thread creation, joining and cleanup
    // if t == 1, just jump to visit_url [seed]
    // else, create threads, visit_url will be our thread routine
    // threads join, cleanup
     
    // DATA_BUF buf;
    // buf.max_size = 1048576; // WHAT SHOULD THE MAX SIZE BE?
    // buf.buf = malloc(1048576);
    // buf.size = 0;
    // buf.seq = -1;
    // CURL* curl_handle = easy_handle_init(&buf, seed_url);

    // if(curl_handle == NULL){
    //     printf("curl failed L\n");
    //     return -1;
    // }

    // CURLcode res = curl_easy_perform(curl_handle);    

    // if(res == CURLE_OK){
        // printf("successfully curled. starting threads...\n");
        // process_data(curl_handle, &buf);
        // curl_easy_cleanup(curl_handle);

        // visited_urls[num_urls_visited] = seed_url;
        // num_urls_visited++;

        ht_add_url(seed_url);
        frontier_push(urls_frontier, seed_url);

        // THREADS STUFF STARTS HERE
        pthread_t threads[num_threads];
        int threads_res[num_threads];
        for (int i = 0; i < num_threads; i++){
            threads_res[i] = pthread_create(threads + i, NULL, visit_url, NULL);
        }
        // C O N D I T I O N A L  F O R  K I L L I N G  T H R E A D S
        pthread_mutex_lock(&threads_lock);
        pthread_cond_wait(&kill_threads_cond, &threads_lock);
        THREADS_STOP = 1;

        pthread_cond_broadcast(&frontier_cond);
        pthread_mutex_unlock(&threads_lock);

        printf("CLEANING EVERYTHING UP\n");
        for (int i = 0; i < num_threads; i++){
            if (threads_res[i] == 0){
                pthread_cond_broadcast(&frontier_cond);
                pthread_join(threads[i], NULL);
                printf("SUCCESSFUL JOIN %d\n", i);
            }
        }
    // }
    // else{
    //     return -1;
    // }
    // write_results(logfile_name);

    xmlCleanupParser();
    free(urls_frontier->stack);
    free(urls_frontier);
    // free(buf.buf);

    free(unique_pngs);
    free(visited_urls);

    pthread_mutex_lock(&ht_lock);
    hdestroy();
    pthread_mutex_unlock(&ht_lock);

    // printf("here\n");

    pthread_mutex_destroy(&ht_lock);
    pthread_mutex_destroy(&frontier_lock);
    pthread_mutex_destroy(&threads_lock);
    pthread_mutex_destroy(&add_png_lock);
    pthread_mutex_destroy(&add_url_lock);

    pthread_cond_destroy(&frontier_cond);
    pthread_cond_destroy(&kill_threads_cond);

    printf("done\n");

    return 0;
}