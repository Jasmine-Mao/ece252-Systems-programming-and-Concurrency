#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h> 
#include <getopt.h>
#include <curl/curl.h>
#include <string.h>
#include <stdbool.h>
#include "png_utils/cat_png.h"
#include "paster.h"

#define URL_1 "http://ece252-1.uwaterloo.ca:2520/image?img="
#define URL_2 "http://ece252-2.uwaterloo.ca:2520/image?img="
#define URL_3 "http://ece252-3.uwaterloo.ca:2520/image?img="
#define ECE252_HEADER "X-Ece252-Fragment: "

/* GLOBAL VARIABLES */
atomic_bool checkImg[50] = {false};
atomic_int numFetched = 0;

/* Using the additional global variable defined in cat_png.h:
atomic_uchar idat_data[INFLATED_DATA_SIZE];
*/

/*STRUCT DECLERATIONS FOR THREADING STUFF*/
struct thread_args
{
    // variables here
    int thread_number;
    CURL *curl_handle;
    int image_number;
};

struct thread_return
{
    int thread_number;   // for debugging purposes only!!
    int return_success;  // returns a number; based on that number we know if the thread has succeeded or failed in its task
};

/*FROM STARTER; WILL BE MODIFIED LATER*/
typedef struct recv_buf2 {
    char *buf;       /* memory to hold a copy of received data */
    size_t size;     /* size of valid data in buf in bytes*/
    size_t max_size; /* max capacity of buf in bytes*/
    int seq;         /* >=0 sequence number extracted from http header */
                     /* <0 indicates an invalid seq number */
} RECV_BUF;

/*GLOBAL VARIABLES; WILL BE INCLUDED IN THE HEADER FILE LATER*/
atomic_bool check_img[50] = {false};    // false if image has not been fetched, become true if image is fetched
atomic_int num_fetched = 0;     // counter for the number of unique images we have fetched. should eventually reach 50

/*THREAD SAFE FUNCTION TO BE CALLED BY ALL THREADS*/
void *fetch_image(void *arg){
    /*INIT STUFF FOR CURL HANDLING*/
    struct thread_args *p_in = arg;
    struct thread_return *p_out = malloc(sizeof(struct thread_return));

    p_in->curl_handle = curl_easy_init();
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
    char img = p_in->image_number + '0';
    strcat(url, &img);
    printf("url: %s\n", url);

    /*INITIALIZE CURL OPTION; MUST LATER BE CONVERTED TO ACTUALLY TAKING DATA*/
    curl_easy_setopt(p_in->curl_handle, CURLOPT_URL, url);

    /*ACTUAL FETCHING STUFF*/
    while(num_fetched < 50){
        /*FIRST GET THE IMAGE*/
        // res = curl_easy_perform(p_in->curl_handle);

        /*CHECK HEADER FOR IMAGE NUMBER*/

        /*SEE IF IMAGE NUMBER HAS NOT BEEN FETCHED*/

        /*IF UNFETCHED, GET HERE*/

        /*ADD TO BUFFER*/

        /*CHECK OFF THIS IMAGE IN THE ARRAY*/

        /*INCREMENT NUM FETCHED*/

        num_fetched++;
        check_img[p_in->thread_number] = true;
        printf("%d, thread %d\n", num_fetched, p_in->thread_number);
    }
    /*CLEAN UP ENVIRONMENT AND EVERYTHING ELSE*/
    curl_easy_cleanup(p_in->curl_handle);
    p_out->thread_number = p_in->thread_number;
    return((void*)p_out);
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
    struct thread_args in_params[num_threads];

    /*SET CURL ENVIRONMENT AND INIT UNIQUE CURL FOR ALL THREADS*/
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl_handle[num_threads];

    /*CREATE THREADS; NUMBER OF THREADS CREATED SPECIFIED BY USER*/
    for(int x = 0; x < num_threads; x++){
        in_params[x].thread_number = x;
        in_params[x].curl_handle = curl_handle[x];
        in_params[x].image_number = img_number;
        pthread_create(&threads[x], NULL, fetch_image, &in_params[x]);
    }

    /*JOIN ALL THREADS BACK TO MAIN*/
    for(int i = 0; i < num_threads; i++){
        pthread_join(threads[i], NULL);
        printf("thread %d joined\n", i);
    }
    printf("all threads joined\n");

    curl_global_cleanup();  // clean up curl environment before return
    return 0;
}