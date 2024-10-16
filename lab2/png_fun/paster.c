#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h> 
#include <getopt.h>
#include <curl/curl.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include "png_utils/zutil.h"
#include "png_utils/cat_png.h"
#include "paster.h"

#define URL_1 "http://ece252-1.uwaterloo.ca:2520/image?img="
#define URL_2 "http://ece252-2.uwaterloo.ca:2520/image?img="
#define URL_3 "http://ece252-3.uwaterloo.ca:2520/image?img="
#define ECE252_HEADER "X-Ece252-Fragment: "

/* GLOBAL VARIABLES */
atomic_bool check_img[50] = {false};    /*false if image has not been fetched, true if image is already stored*/
atomic_int num_fetched = 0;             /*counter for the number of unique images we have fetched, limit is 50*/

/*STRUCT DECLARATIONS FOR THREADING STUFF*/
struct thread_args
{
    // variables here
    int thread_number;
    CURL *curl_handle;
    int image_number; /* N in [1,3]*/
};

/*FROM STARTER*/
typedef struct data_buf {
    U8 *buf;            /* memory to hold a copy of received IDAT data */
    size_t size;        /* size of valid data in buf in bytes*/
    int write_success;  /* 0 if success, -1 if failure */
    int seq;            /* >=0 sequence number extracted from http header */
                        /* <0 indicates an invalid seq number */
} DATA_BUF;


size_t idat_write_callback(char * recv, size_t size, size_t nmemb, void *userdata){
    /*total size of received png data*/
    size_t real_size = size * nmemb;

    /*type cast to recv_buf struct*/
    DATA_BUF * strip_data = (DATA_BUF *) userdata;

    /*early return if fragment is non-unique*/
    if (check_img[strip_data->seq]){
        return real_size;
    }

    /*copy length*/
    int read_index = PNG_SIG_SIZE + IHDR_CHUNK_SIZE;
    unsigned int size_buf = 0;
    memcpy(&size_buf, recv + read_index, CHUNK_LEN_SIZE);
    size_buf = ntohl(size_buf);

    if (size_buf > real_size){
        return CURLE_WRITE_ERROR;
    }

    strip_data->size = (size_t)size_buf;

    /* copy IDAT data*/
    read_index += CHUNK_LEN_SIZE + CHUNK_TYPE_SIZE;
    strip_data->buf = malloc(strip_data->size);

    memcpy(strip_data->buf, recv + read_index, strip_data->size);

    strip_data->write_success = 0;

    return real_size;
}

size_t header_callback(char *p_recv, size_t size, size_t nmemb, void *userdata)
{
    int realsize = size * nmemb;
    DATA_BUF *strip_data = userdata; 
    
    if (realsize > strlen(ECE252_HEADER) &&
	 strncmp(p_recv, ECE252_HEADER, strlen(ECE252_HEADER)) == 0) {
         /* extract img sequence number */
        strip_data->seq = atoi(p_recv + strlen(ECE252_HEADER)); /*uses ECE252_HEADER as an offset to extract img sequence number*/
    }
    return realsize;
}

/*THREAD SAFE FUNCTION TO BE CALLED BY ALL THREADS*/
void *fetch_image(void *arg){
    /*INIT STUFF FOR CURL HANDLING*/
    struct thread_args *p_in = arg;

    CURL *curl_handle = curl_easy_init();

    if (curl_handle == NULL) {
        fprintf(stderr, "curl_easy_init: returned NULL\n");
        return NULL;
    }

    CURLcode res;
    char url[256];

    /*SEND THREAD TO APPROPRIATE SERVER*/
    int server = p_in->thread_number % 3;
    if(server == 1)
        strcpy(url, URL_1);
    else if(server == 2)
        strcpy(url, URL_2);
    else if(server == 0)
        strcpy(url, URL_3);

    /*FULLY CONSTRUCT URL WITH THE IMAGE NUMBER*/
    char * img_num = malloc(sizeof(p_in->image_number));
    sprintf(img_num, "%d", p_in->image_number);
    strcat(url, img_num);

    /*INITIALIZE CURL OPTION; MUST LATER BE CONVERTED TO ACTUALLY TAKING DATA*/
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);

    /*ACTUAL FETCHING STUFF*/
    while(num_fetched < 50){
        DATA_BUF strip_data;

        strip_data.size = 0;
        strip_data.seq = -1;
        strip_data.write_success = -1;

        /* Define write callback*/
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, idat_write_callback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&strip_data);

        /* Define header callback*/
 
        curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, header_callback);  
        curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void *)&strip_data);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        

        /*Fetch data from server*/
        res = curl_easy_perform(curl_handle);

        if ((res == CURLE_OK) && (!check_img[strip_data.seq]) && (strip_data.write_success == 0)){ /*checks if image segment was fetched already*/
            /* Inflate and store idat data*/
            int store_index = strip_data.seq * STRIP_HEIGHT * (PNG_WIDTH * 4 + 1);
            U64 inf_size;
            mem_inf(idat_data + store_index, &inf_size, strip_data.buf, strip_data.size);

            //printf("Found unique segment: %d for thread number %d\n", strip_data.seq, p_in->thread_number);

            // STORE DATA HERE
            check_img[strip_data.seq] = true;
            num_fetched++;
        }
        if (strip_data.write_success == 0){
            free(strip_data.buf);
        }
        strip_data.size = 0;
    }

    /*CLEAN UP ENVIRONMENT AND EVERYTHING ELSE*/
    curl_easy_cleanup(curl_handle);
    free(img_num);

    return NULL;
}

int main(int argc, char* argv[]){
    int c;
    int num_threads = 1; /* default num of threads and images */
    int img_number = 1;

    /*OPTION STUFF*/
    while((c = getopt(argc, argv, "t:n:")) != -1){
          switch(c){
            case't':
            num_threads = strtoul(optarg, NULL, 10);
            if(num_threads <= 0){
                printf("please enter valid thread number\n");
                return -1;
            }
            printf("arg passed for threads: %d\n", num_threads);
            break;
            case 'n':
            img_number = strtoul(optarg, NULL, 10);
            if(img_number <= 0 || img_number > 3){
                printf("please enter valid image number\n");
                return -1;
            }
            printf("arg passed for image: %d\n", img_number);
            break;
            default:
            /* if nothing was passed, we just use the default value*/
            break;
        }
    }

    /*INIT THREAD STUFF*/
    pthread_t threads[num_threads];
    struct thread_args in_params[num_threads];

    /*SET CURL ENVIRONMENT AND INIT UNIQUE CURL FOR ALL THREADS*/
    curl_global_init(CURL_GLOBAL_DEFAULT);

    /*CREATE THREADS; NUMBER OF THREADS CREATED SPECIFIED BY USER*/
    int return_success[num_threads];
    memset(return_success, -1, sizeof(int) * num_threads);
    for(int x = 0; x < num_threads; x++){
        in_params[x].thread_number = x;
        in_params[x].image_number = img_number;
        return_success[x] = pthread_create(threads + x, NULL, fetch_image, in_params + x);
        //printf("trying thread %d\n", x);
        // if (return_success[x] == 0){
        //     printf("thread %d succeeded, returned %d\n", x, return_success[x]);
        // }
        // else{
        //     printf("THREAD %d FAILED\n", x);
        // }
    }

    /*JOIN ALL THREADS BACK TO MAIN*/
    for(int i = 0; i < num_threads; i++){
        /*CHECK IF THREAD CREATED WAS SUCCESSFUL. IF NOT, DO NOT ATTEMPT TO JOIN THE THREAD -- SEG FAULT*/
        if(return_success[i] == 0){
            pthread_join(threads[i], NULL);
        }
    }

    /*printf("all threads joined\n");*/
    int result = concatenate_png();
    if (result != 0){
        printf("catpng failure \n");
    }

    curl_global_cleanup();  /*clean up curl environment before return*/
    return 0;
}