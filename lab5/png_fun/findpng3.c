#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <curl/curl.h>
#include <curl/multi.h>

#include <getopt.h>
#include <search.h>
#include <errno.h>
#include <stdatomic.h>

#include <libxml/HTMLparser.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/uri.h>

#include <time.h>
#include <sys/time.h>

#include "findpng3.h"
#include "frontier_stack.h"
#include "hashtable.h"

#define MAX_URLS 500
#define ECE_252_HEADER "X-Ece252-Fragment: "
#define MAX_WAIT_MS 30*1000
// from starter

#define CT_PNG  "image/png"
#define CT_HTML "text/html"
#define CT_PNG_LEN  9
#define CT_HTML_LEN 9

#define MAX_WAIT_MSECS 30*1000 // 30 seconds for now, this should be long enough for any activity

// GLOBAL VARIABLES
FRONTIER * urls_frontier;
char ** unique_pngs;    // this is a pointer to an array of pointers (where each item is a pointer to a char array (string)
char ** visited_urls; //for writing purposes
char ** hash_data;

char seed_url[256];
int max_png = 50;

int num_png_obtained = 0;
int num_urls_visited = 0;

size_t data_write_callback(char* recv, size_t size, size_t nmemb, void* user_data){
    size_t real_size = size * nmemb;

    DATA_BUF* data_fetched = user_data;

    memcpy(data_fetched->buf + data_fetched->size, recv, real_size);
    data_fetched->size += real_size;
    // PRINTING HERE WOULD JUST SPIT OUT EVERYTHING IN THE HTML DOC
    return real_size;

    // taken directly from paster2
}

CURL *easy_handle_init(CURLM *cm, DATA_BUF *ptr, const char *url)
{
    CURL *easy_handle = curl_easy_init(); //leaks
    if (easy_handle == NULL) {
        fprintf(stderr, "curl_easy_init: returned NULL\n");
        free(ptr->buf);
        free(ptr);
        return NULL;
    }

    EH_INFO* eh_info = malloc(sizeof(EH_INFO));
    eh_info->data_buf = ptr;
    eh_info->url = url;
    // printf("MAX SIZE GRABBED FROM THINGY %ld\n", eh_info->data_buf->max_size);


    curl_easy_setopt(easy_handle, CURLOPT_URL, url);

    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, data_write_callback); 
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, (void *)ptr);

    curl_easy_setopt(easy_handle, CURLOPT_PRIVATE, eh_info);
    curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 0L);

    curl_easy_setopt(easy_handle, CURLOPT_USERAGENT, "ece252 lab5 crawler");
    curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(easy_handle, CURLOPT_UNRESTRICTED_AUTH, 1L);
    curl_easy_setopt(easy_handle, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(easy_handle, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(easy_handle, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(easy_handle, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
    curl_easy_setopt(easy_handle, CURLOPT_HTTPAUTH, CURLAUTH_ANY);

    curl_multi_add_handle(cm, easy_handle);
    return easy_handle;
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
        return NULL;
    }
    return result;
}

int process_png(CURL *curl_handle, DATA_BUF *recv_buf) {
    char *eurl = NULL;
    curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &eurl);
    
    if ((eurl != NULL) && is_png(recv_buf->buf) == 0) {
        ht_add_url(eurl, hash_data);

        unique_pngs[num_png_obtained] = strdup(eurl);
        printf("PNG: %s\n", eurl);
        
        num_png_obtained++;
    }
    return 0;
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
            if ( href != NULL && !strncmp((const char *)href, "http", 4) && ht_search_url((char*)href) == 0) {
                printf("this is a unique url: %s\n", (char*)href);
                
                ht_add_url((char*)href, hash_data);

                frontier_push(urls_frontier, (char*)href);
            }
            xmlFree(href);
        }
        xmlXPathFreeObject (result);
    }
    xmlFreeDoc(doc);
    return 0;
}

int process_html(CURL *curl_handle, DATA_BUF *recv_buf) {
    int follow_relative_link = 1;
    char *url = NULL; 

    curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &url);
    // printf("%s\n", url);
    return find_http(recv_buf->buf, recv_buf->size, follow_relative_link, url); 
}

int process_data(CURL *curl_handle, DATA_BUF *recv_buf) {
    CURLcode res;
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

void webcrawler(int max_connections){
    // printf("HERE IN WEB CRAWLER! MAX CONNECTIONS: %d\n", max_connections);
    int urls_available;
    int connections_to_add;
    int connections_free = max_connections;
    int current_connections = 0;
    int pending = 1;

    CURLM *cm = curl_multi_init();

    CURLMsg *msg = NULL;
    CURL *eh=NULL;
    CURLcode return_code=0;
    int msgs_in_queue = 0;

    while ((!frontier_is_empty(urls_frontier) && (num_png_obtained < max_png)) || (pending > 0)){
        // printf("Pending val %d\n", pending);
        urls_available = frontier_get_count(urls_frontier);
        // printf("Num urls available in the frontier: %d\n", urls_available);

        connections_free = max_connections - current_connections;
        // printf("Number of free connections in the mutli: %d\n", connections_free);
        if (connections_free > urls_available){
            connections_to_add = urls_available;
        }
        else {
            connections_to_add = connections_free;
        }
        // printf("Connections to add: %d\n", connections_to_add);
        for (int i = 0; i < connections_to_add; i++){
            // printf("adding a new url...\n");
            char url[256];
            DATA_BUF* data_buf = malloc(sizeof(DATA_BUF));

            data_buf->max_size = 1048576; // WHAT SHOULD THE MAX SIZE BE?
            data_buf->buf = malloc(data_buf->max_size); //leaks
            data_buf->size = 0;
            frontier_pop(urls_frontier, url);
            CURL* curl_handle = easy_handle_init(cm, data_buf, url);
            if (curl_handle == NULL){
                frontier_push(urls_frontier, url);
                free(data_buf->buf);
                free(data_buf);
            }
        }

        CURLMcode perform_result = curl_multi_perform(cm, &current_connections); //leaks
        // check if perform fails
        if (perform_result != CURLM_OK) {
            fprintf(stderr, "curl_multi_perform failed: %s\n", curl_multi_strerror(perform_result));
        }

        pending = current_connections;

        long timeout = 0;
        curl_multi_timeout(cm, &timeout);
        // printf("Curled successfully! Number of current connections: %d\n", current_connections);
        // printf("Our current timeout is this: %ld\n", timeout);

        int numfds = 0;
        curl_multi_wait(cm, NULL, 0, MAX_WAIT_MSECS, &numfds);
        
        while ((msg = curl_multi_info_read(cm, &msgs_in_queue))){
            if (msg->msg == CURLMSG_DONE){
                eh = msg->easy_handle;
                return_code = msg->data.result;
                if (return_code != CURLE_OK){
                    printf("O-0\n");
                    //curl_multi_perform(cm, &current_connections);
                    continue;
                }

                EH_INFO* temp;

                curl_easy_getinfo(eh, CURLINFO_PRIVATE, &temp);

                printf("HERE**************************************************************************************\n");

                process_data(msg->easy_handle, temp->data_buf);
                
                curl_multi_remove_handle(cm, eh);
                if (temp != NULL) {
                    if (temp->data_buf != NULL) {
                        free(temp->data_buf->buf);
                        free(temp->data_buf);
                    }
                    free(temp);
                }

                // Comment out the following line temporarily to see if the leak persists
                curl_easy_cleanup(eh);
            }
            else{
                // terribly wrong (maybe, theres not documentation on what it means if we get here)
                fprintf(stderr, "O_O\n");
            }
        }

    }
    curl_multi_cleanup(cm);
    return;
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
    double times[2];
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) {
        perror("gettimeofday");
        abort();
    }
    times[0] = (tv.tv_sec) + tv.tv_usec/1000000.;

    int c;
    int connections = 1;
    char* logfile_name = NULL;

    if(argc < 2){
        fprintf(stderr, "Incorrect arguments!\n");
        return -1;
    }
    if (hcreate(MAX_URLS) == 0){
        printf("Visited URLs table could not be made!\n");
        return errno;
    }

    while((c = getopt(argc, argv, "t:m:v:")) != -1){
        switch (c)
        {
        case 't':
            connections = strtoul(optarg, NULL, 10);
            if(connections <= 0){
                printf("please enter valid thread number\n");
                return -1;
            }
            break;
        
        case 'm':
            max_png = (int)strtoul(optarg, NULL, 10);
            if(max_png <= 0){
                printf("please enter valid number of max images\n");
                return -1;
            }
            break;
        case 'v':
            logfile_name = optarg;
            break;
        default:
            break;
        }        
    }


    strcpy(seed_url, argv[argc-1]);

    visited_urls = malloc(sizeof(char*)*500);
    hash_data = malloc(sizeof(char*)*500);
    unique_pngs = malloc(sizeof(char*) * max_png);
    
    xmlInitParser();

    urls_frontier = malloc(sizeof(FRONTIER));
    frontier_init(urls_frontier);

    ht_add_url(seed_url, hash_data);
    frontier_push(urls_frontier, seed_url);
    printf("Did we successfully add a url? %d\n", !frontier_is_empty(urls_frontier));

    curl_global_init(CURL_GLOBAL_ALL);
    
    // Call to multicurl async i/o crawler

    webcrawler(connections);
    write_results(logfile_name);
    
    curl_global_cleanup();
    xmlCleanupParser();

    frontier_cleanup(urls_frontier);
    free(urls_frontier->stack);
    free(urls_frontier);

    ht_cleanup(hash_data);
    free(unique_pngs);
    free(visited_urls);
    free(hash_data);

    hdestroy();

    if (gettimeofday(&tv, NULL) != 0) {
        perror("gettimeofday");
        abort();
    }
    times[1] = (tv.tv_sec) + tv.tv_usec/1000000.;
    printf("findpng2 execution time: %.6lf seconds\n",  times[1] - times[0]);    

    return 0;
}