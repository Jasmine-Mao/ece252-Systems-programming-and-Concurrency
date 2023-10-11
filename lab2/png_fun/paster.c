#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/types.h>
#include<ctype.h> 
#include<getopt.h>

void *paster(void* threadNumber){
    // function that takes the number of threads and the picture to look for
    int *number = (int*)threadNumber;
    printf("i'm thread number %d\n", *number);
    pthread_exit(NULL);
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

    for(int x = 0; x < numThreads; x++){
        int* arg;
        *arg = x;
        pthread_create(&threads[x], NULL, paster, arg);
        pthread_join(threads[x], NULL);
    }

    /* for(int x = 0; x < numThreads; x++){
        pthread_join(threads[x], NULL);
    } */

    return 0;
}