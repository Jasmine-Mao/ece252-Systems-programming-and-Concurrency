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
// #include "png_utils/zutil.h"
#include "paster.h"

#define URL_1 "http://ece252-1.uwaterloo.ca:2520/image?img="
#define URL_2 "http://ece252-2.uwaterloo.ca:2520/image?img="
#define URL_3 "http://ece252-3.uwaterloo.ca:2520/image?img="
#define ECE252_HEADER "X-Ece252-Fragment: "

/* GLOBAL VARIABLES */
atomic_bool check_img[50] = {false};    // false if image has not been fetched, true if image is already stored
atomic_int num_fetched = 0;             // counter for the number of unique images we have fetched, limit is 50
//#define BUF_SIZE 1048576  /* 1024*1024 = 1M */

/* Using the additional global variable defined in cat_png.h:
atomic_uchar idat_data[INFLATED_DATA_SIZE];
*/

/*STRUCT DECLARATIONS FOR THREADING STUFF*/
struct thread_args
{
    // variables here
    int thread_number;
    CURL *curl_handle;
    int image_number; /* N in [1,3]*/
};

struct thread_return
{
    int thread_number;   // for debugging purposes only!!
    int return_success;  // returns a number; based on that number we know if the thread has succeeded or failed in its task
};

/*FROM STARTER; WILL BE MODIFIED LATER*/
typedef struct data_buf {
    U8 *buf;            /* memory to hold a copy of received IDAT data */
    size_t size;        /* size of valid data in buf in bytes*/
    int write_success;  /* 0 if success, -1 if failure */
    int seq;            /* >=0 sequence number extracted from http header */
                        /* <0 indicates an invalid seq number */
} DATA_BUF;


size_t idat_write_callback(char * recv, size_t size, size_t nmemb, void *userdata){
    // total size of received png data
    size_t real_size = size * nmemb;

    // type cast to recv_buf struct
    DATA_BUF * strip_data = (DATA_BUF *) userdata;

    // early return if fragment is non-unqie
    if (check_img[strip_data->seq]){
        return real_size;
    }

    // copy length
    int read_index = PNG_SIG_SIZE + IHDR_CHUNK_SIZE;
    memcpy(&(strip_data->size), recv + read_index, CHUNK_LEN_SIZE);
    strip_data->size = ntohl(strip_data->size);

    // copy IDAT data
    read_index += CHUNK_LEN_SIZE + CHUNK_TYPE_SIZE;
    strip_data->buf = malloc(strip_data->size);
    memcpy(strip_data->buf, recv + read_index, strip_data->size);

    strip_data->write_success = 0;

    return real_size;
}

size_t header_callback(char *p_recv, size_t size, size_t nmemb, void *userdata)
{
    //printf(p_recv);
    int realsize = size * nmemb;
    DATA_BUF *strip_data = userdata; 
    
    if (realsize > strlen(ECE252_HEADER) &&
	 strncmp(p_recv, ECE252_HEADER, strlen(ECE252_HEADER)) == 0) {
         /* extract img sequence number */
        strip_data->seq = atoi(p_recv + strlen(ECE252_HEADER));
    }
    return realsize;
}

/*THREAD SAFE FUNCTION TO BE CALLED BY ALL THREADS*/
void *fetch_image(void *arg){
    /*INIT STUFF FOR CURL HANDLING*/
    struct thread_args *p_in = arg;
    struct thread_return *p_out = malloc(sizeof(struct thread_return));

    CURL *curl_handle = curl_easy_init();
    CURLcode res;
    char url[256];
    //RECV_BUF recv_buf;

    //recv_buf_init(&recv_buf, BUF_SIZE);

    /*SEND THREAD TO APPROPRIATE SERVER*/
    int server = p_in->thread_number % 3;
    if(server == 1)
        strcpy(url, URL_1);
    else if(server == 2)
        strcpy(url, URL_2);
    else if(server == 0)
        strcpy(url, URL_3);

    /*FULLY CONSTRUCT URL WITH THE IMAGE NUMBER*/
    char img = p_in->image_number + '0'; 
    strcat(url, &img);
    // printf("url: %s\n", url);

    /*INITIALIZE CURL OPTION; MUST LATER BE CONVERTED TO ACTUALLY TAKING DATA*/
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);

    /*ACTUAL FETCHING STUFF*/
    while(num_fetched < 50){
        DATA_BUF strip_data;

        // data_buf_init(&strip_data, 10000);
        /*FIX LATER*/
        strip_data.size = 0;
        strip_data.seq = -1;
        strip_data.write_success = -1;

        // Define write callback
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, idat_write_callback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&strip_data);

        // Define header callback
 
        curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, header_callback);  
        curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void *)&strip_data);
        

        // Fetch data from server
        res = curl_easy_perform(curl_handle);

        if ((res == CURLE_OK) && (!check_img[strip_data.seq]) && (strip_data.write_success == 0)){ /*checks if image segment was fetched already*/
            // Inflate and store idat data
            int store_index = strip_data.seq * STRIP_HEIGHT * (PNG_WIDTH * 4 + 1);
            U64 inf_size;
            mem_inf(idat_data + store_index, &inf_size, strip_data.buf, strip_data.size);

            //printf("Found unique segment: %d for thread number %d\n", strip_data.seq, p_in->thread_number);

            // STORE DATA HERE
            check_img[strip_data.seq] = true;
            num_fetched++;
        }
        else if(res != CURLE_OK){
            //printf("Curl failed for thread number %d.\n", p_in->thread_number);
        }
        else {
            //printf("Found repeated segment: %d for thread number %d\n", strip_data.seq, p_in->thread_number);
        }

        if (strip_data.write_success == 0){
            free(strip_data.buf);
        }
    }

    /*CLEAN UP ENVIRONMENT AND EVERYTHING ELSE*/
    curl_easy_cleanup(curl_handle);
    void* ret_val = (void*)p_out->thread_number;
    free(p_out);
    /* p_out->thread_number = p_in->thread_number;
    free(p_out); */
    return ret_val;
}

int main(int argc, char* argv[]){
    int c;
    int num_threads = 1; // default num of threads and images
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
            // if nothing was passed, we just use the defauly value
            break;
        }
    }

    /*INIT THREAD STUFF*/
    pthread_t threads[num_threads];

    struct thread_return *results[num_threads];
    //struct thread_args in_params[num_threads];

    /*SET CURL ENVIRONMENT AND INIT UNIQUE CURL FOR ALL THREADS*/
    curl_global_init(CURL_GLOBAL_DEFAULT);

    /*CREATE THREADS; NUMBER OF THREADS CREATED SPECIFIED BY USER*/
    int return_success[num_threads];
    for(int x = 0; x < num_threads; x++){
        struct thread_args in_params = { 0, NULL, 0 };
        in_params.thread_number = x;
        in_params.image_number = img_number;
        do{
            return_success[x] = pthread_create(&threads[x], NULL, fetch_image, &in_params);
            printf("trying thread %d\n", x);
        }
        while(return_success[x] != 0);
        printf("thread %d succeeded\n", x);
    }

    /*JOIN ALL THREADS BACK TO MAIN*/
    for(int i = 0; i < num_threads; i++){
        /*CHECK IF THREAD CREATED WAS SUCCESSFUL. IF NOT, DO NOT ATTEMPT TO JOIN THE THREAD -- SEG FAULT*/
        if(return_success[i] == 0){
            printf("thread %d created successfully\n", i);
            pthread_join(threads[i], (void **)&(results[i]));
            printf("thread %d joined\n", i);
        }
    }
    printf("all threads joined\n");
    int result = concatenate_png();
    if (result != 0){
        printf("catpng failure \n");
    }

    curl_global_cleanup();  // clean up curl environment before return
    return 0;
}