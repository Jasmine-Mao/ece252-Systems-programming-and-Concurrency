#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h> 
#include <getopt.h>
#include <curl/curl.h>
#include "paster.h"


struct thread_args
{
    // variables here
    int threadNumber;   // for debugging purposes only!!
    CURL *curl_handle;
};

struct thread_return
{
    int threadNumber;   // for debugging purposes only!!
    int returnSuccess;  // returns a number; based on that number we know if the thread has succeeded or failed in its task
};

void *fetchImage(void *arg){    // currently just spits out what number the thread is; currently for debugging purposes
    struct thread_args *p_in = arg;
    struct thread_return *p_out = malloc(sizeof(struct thread_return));
    printf("i'm thread number %d\n", p_in->threadNumber);

    /*one we get here, we actually do stuff, everything above is just to make sure we are in the right thread*/
    p_in->curl_handle = curl_easy_init();

    if(p_in->curl_handle){
        printf("curl handle for %d initialized\n", p_in->threadNumber);
    }

    curl_easy_cleanup(p_in->curl_handle);
    p_out->threadNumber = p_in->threadNumber;
    return((void*)p_out);
}

int main(int argc, char* argv[]){
    int c;
    int numThreads = 1; // default num of threads and images
    int imgNumber = 1;

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