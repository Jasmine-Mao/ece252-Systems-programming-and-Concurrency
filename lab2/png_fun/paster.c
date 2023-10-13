#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h> 
#include <getopt.h>
#include <curl/curl.h>
#include <string.h>
#include <stdatomic.h>
#include <stdbool.h>
#include "paster.h"

#define URL_1 "http://ece252-1.uwaterloo.ca:2520/image?img="
#define URL_2 "http://ece252-2.uwaterloo.ca:2520/image?img="
#define URL_3 "http://ece252-3.uwaterloo.ca:2520/image?img="

struct thread_args
{
    // variables here
    int threadNumber;   // for debugging purposes only!!
    CURL *curl_handle;
    int imageNumber;
    _Atomic bool *boolPtr;  // pointer to 
    _Atomic int *intPtr;
};

struct thread_return
{
    int threadNumber;   // for debugging purposes only!!
    int returnSuccess;  // returns a number; based on that number we know if the thread has succeeded or failed in its task
};

void *fetchImage(void *arg){    // currently just spits out what number the thread is; currently for debugging purposes
    /*INIT STUFF FOR CURL HANDLING*/
    struct thread_args *p_in = arg;
    struct thread_return *p_out = malloc(sizeof(struct thread_return));

    p_in->curl_handle = curl_easy_init();
    CURLcode res;
    char url[256];

    /*SEND THREAD TO APPROPRIATE SERVER*/
    int server = p_in->threadNumber % 3;
    if(server == 1)
        strcpy(url, URL_1);
    else if(server == 2)
        strcpy(url, URL_2);
    else if(server == 0)
        strcpy(url, URL_3);

    /*FULLY CONSTRUCT URL WITH THE IMAGE NUMBER*/
    char img = p_in->imageNumber + '0';
    strcat(url, &img);  // append image number to end of url to get the full url
    printf("url: %s\n", url);


    /*ACTUAL FETCHING STUFF*/
    while(*p_in->intPtr < 50){
        (*p_in->intPtr)++;
        printf("%d, thread %d\n", *p_in->intPtr, p_in->threadNumber);
    }

    if(p_in->curl_handle){
        curl_easy_setopt(p_in->curl_handle, CURLOPT_URL, url);
        printf("curl handle for thread %d initialized\n", p_in->threadNumber);
        res = curl_easy_perform(p_in->curl_handle);

    }

    curl_easy_cleanup(p_in->curl_handle);
    p_out->threadNumber = p_in->threadNumber;
    return((void*)p_out);
}

int main(int argc, char* argv[]){
    int c;
    int numThreads = 1; // default num of threads and images
    int imgNumber = 1;

    atomic_bool checkImg[50] = {false};
    atomic_int numFetched = 0;

    // option stuff

    // default to 1 thread, image 1
    while((c = getopt(argc, argv, "t:n:")) != -1){
          switch(c){
            case't':
            numThreads = strtoul(optarg, NULL, 10);
            if(numThreads <= 0){
                printf("please enter valid thread number\n");
                return -1;
            }
            printf("arg passed for threads: %d\n", numThreads);
            break;
            case 'n':
            imgNumber = strtoul(optarg, NULL, 10);
            if(imgNumber <= 0 || imgNumber > 3){
                printf("please enter valid image number\n");
                return -1;
            }
            printf("arg passed for image: %d\n", imgNumber);
            break;
            default:
                // if nothing was passed, we just use the defauly value
            break;
        }
    }

    // for debugging purposes; will be removed later
    printf("number of threads: %d\n", numThreads);
    printf("image number: %d\n", imgNumber);

    // init the threads
    pthread_t threads[numThreads];

    CURL *curl_handle[numThreads];

    // init return value and input arguments
    struct thread_return *results[numThreads];
    struct thread_args in_params[numThreads];

    curl_global_init(CURL_GLOBAL_DEFAULT);  // init curl environment; must be called before any curl operations can work

    for(int x = 0; x < numThreads; x++){
        in_params[x].threadNumber = x;
        in_params[x].curl_handle = curl_handle[x];
        in_params[x].imageNumber = imgNumber;
        in_params[x].boolPtr = checkImg;
        in_params[x].intPtr = &numFetched;
        pthread_create(&threads[x], NULL, fetchImage, &in_params[x]);
        // create [numThreads] threads; arg passed is the number assigned to each thread
    }

    for(int i = 0; i < numThreads; i++){
        pthread_join(threads[i], NULL);
        printf("thread %d joined\n", i);
        // join all threads
    }
    printf("all threads joined\n");

    curl_global_cleanup();  // clean up curl environment before return
    return 0;
}