#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/types.h>
#include<ctype.h> 
#include<getopt.h>
#include<curl/curl.h>

struct thread_args
{
    // variables here
    int threadNumber;
};

struct thread_return
{
    int threadNumber;   // for debugging purposes only!!
};

void *paster(void *arg){
    // function that takes the number of threads and the picture to look for
    struct thread_args *p_in = arg;
    struct thread_return *p_out = malloc(sizeof(struct thread_return));
    printf("i'm thread number %d\n", p_in->threadNumber);
    p_out->threadNumber = p_in->threadNumber;
    return((void*)p_out);
}

int main(int argc, char* argv[]){
    int c;
    int numThreads = 1; // default num of threads and images
    int imgNumber = 1;

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

    printf("number of threads: %d\n", numThreads);
    printf("image number: %d\n", imgNumber);

    pthread_t threads[numThreads];

    struct thread_ret *results[numThreads];
    struct thread_args in_params[numThreads];

    curl_global_init(CURL_GLOBAL_DEFAULT);  // init curl environment; must be called before any curl operations can work

    for(int x = 0; x < numThreads; x++){
        in_params[x].threadNumber = x + 1;
        pthread_create(&threads[x], NULL, paster, &in_params[x]);
        pthread_join(threads[x], NULL);
    }

    /* for(int x = 0; x < numThreads; x++){
        pthread_join(threads[x], NULL);
    } */
    curl_global_cleanup();  // clean up curl environment before return
    return 0;
}