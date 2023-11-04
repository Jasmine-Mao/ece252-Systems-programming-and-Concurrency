#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <curl/curl.h>
#include <semaphore.h>
#include <unistd.h>

#include "png_utils/cat_png.h"
#include "paster2.h"

#define URL_1           "http://ece252-1.uwaterloo.ca:2520/image?img="
#define URL_2           "http://ece252-2.uwaterloo.ca:2520/image?img="
#define URL_3           "http://ece252-3.uwaterloo.ca:2520/image?img="
#define URL_IMAGE_SEG   "&part="

#define BUFFER_SIZE     1
/*^TEMPORARY!!! JUST SO THINGS DONT THROW ERR0RS WHILE COMPILING*/

typedef struct data_buf{
    U8* buf;
    size_t size;
    size_t max_size;
    int seq;
} DATA_BUF;

/*ASK ABOUT WRITE CALL BACK*/
size_t data_write_callback(char* recv, size_t size, size_t nmemb, void *userdata){
    size_t real_size = size * nmemb;

    DATA_BUF* strip_data = (DATA_BUF*)userdata;
    if(strip_data->size + real_size +1 > strip_data->max_size){
        









        return CURLE_WRITE_ERROR;
    }

    memcpy(strip_data->buf + strip_data->size, recv, real_size);
    strip_data->size += real_size;
    strip_data->buf[strip_data->size] = 0;

    return real_size;
}

// likely we still want a curl header and data callback function, but the data one will use the ENTIRE PNG
// curl header function
// curl data function
// queue implementation 
    // struct typedef blah blah blad
    // ask chat gpt or steal from stack overflow

int consumer_protocol(){ //iman
    /*steps*/
    /*1. access shared memory (buffer)*/
    /*2. fetch image segments in order*/
    /*3. inflate and concatenate all*/
    /*4. deflate once all have been concatenated*/

    //have to manage multiple consumers (child processes)
    // this is where the consumer parses the png data (from our buffer) for the idat data, inflates and stores it

    // we can loop here

    // critical section somewhere here

    return 0;
}

int write_file(const char *path, const void *in, size_t len)
{
    FILE *fp = NULL;

    if (path == NULL) {
        fprintf(stderr, "write_file: file name is null!\n");
        return -1;
    }

    if (in == NULL) {
        fprintf(stderr, "write_file: input data is null!\n");
        return -1;
    }

    fp = fopen(path, "wb");
    if (fp == NULL) {
        perror("fopen");
        return -2;
    }

    if (fwrite(in, 1, len, fp) != len) {
        fprintf(stderr, "write_file: incomplete write!\n");
        return -3; 
    }
    return fclose(fp);
}

int producer_protocol(int process_number, int sleep_time, int num_processes, int image_number){
    /*----CURL SETUP----*/
    CURL* curl_handle = curl_easy_init();

    if(curl_handle == NULL){
        fprintf(stderr, "curl_easy_init: returned NULL\n");
        exit(-1);
    }
    CURLcode res;
    /*----END----*/


    /*----URL SETUP----*/
    char url[256];

    int server_number = process_number % 3;
    if(server_number == 1){
        strcpy(url, URL_1);
    }
    else if(server_number == 2){
        strcpy(url, URL_2);
    }
    else if(server_number == 0){
        strcpy(url, URL_3);
    }
    char* image_num = malloc(sizeof(int));
    sprintf(image_num, "%d", image_number);
    strcat(url, image_num);
    free(image_num);
    strcat(url, URL_IMAGE_SEG);
    /*----END----*/
    
    while(process_number < 50){
        char temp[256];
        strcpy(temp, url);

        char* seg = malloc(sizeof(int));
        sprintf(seg, "%d", process_number);
        strcat(temp, seg);

        DATA_BUF strip_data;
        strip_data.size = 0;
        strip_data.seq = -1;
        strip_data.max_size = 10000;
        strip_data.buf = malloc(strip_data.max_size);

        curl_easy_setopt(curl_handle, CURLOPT_URL, temp);

        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, data_write_callback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&strip_data);

        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        res = curl_easy_perform(curl_handle);
        char fname[256];

        if((res == CURLE_OK)){
            printf("made it");
            sprintf(fname, "./output_%d.png", process_number);
            write_file(fname, strip_data.buf, strip_data.size);



        }

        process_number = process_number + num_processes;

        free(seg);
        free(strip_data.buf);

        usleep(sleep_time * 100);
        break;
    }


    
    // this is where we wanna curl and store pngs in our buffer
    // if there is no space in the buffer, do nothing

    // smeaphore to access the buffer as a producer bc only one producer allowed in the buffer at a time
    // we can loop here

    // critical section somewhere here
    curl_easy_cleanup(curl_handle);
    return 0;
}

int run_proccesses(int producer_count, int consumer_count){
    pid_t pid = 0;
    pid_t children[producer_count + consumer_count];

    for (int i = 0; i < producer_count; i++){
        // if pid == child,
        // lets break then call producer_protocol 
        // or the reverse order
        // if the process is a parent we wanna keep looping
        // int pid = fork()
        // if (pid is child){
         //   producer_protocol()
         //   break
    }

    for (int i = 0; i < consumer_count; i++){
        // if pid == child,
        // lets break then call consumer_protocol 
        // or the reverse order
        // if the process is a parent we wanna keep looping
    }

    // if we are the parent at this point, we should be ready to roll (create the png)

    return 0;
}


int main(int argc, char * argv[]){
    int c;
    curl_global_init(CURL_GLOBAL_DEFAULT);  // init curl stuff
    
    sem_t* buffer_spaces;
    sem_t* buffer_items;

    sem_t* buffer_permission;
    producer_protocol(3, 50, 11, 1);
    // PARSE ARGS:
    // P, C passed to run_processes function
    // B, X, N are set into global variables for buffer size, sleep time, and image number.

    return 0;
}