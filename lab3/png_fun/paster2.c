#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <curl/curl.h>
#include <semaphore.h>
#include <unistd.h>

#include "png_utils/cat_png.h"
#include "ring_buffer.h"
#include "paster2.h"

#define URL_1           "http://ece252-1.uwaterloo.ca:2520/image?img="
#define URL_2           "http://ece252-2.uwaterloo.ca:2520/image?img="
#define URL_3           "http://ece252-3.uwaterloo.ca:2520/image?img="
#define URL_IMAGE_SEG   "&part="
#define SEM_PROC 1

int BUFFER_SIZE;
int SLEEP_TIME;
int IMG_NUM;

/*ASK ABOUT WRITE CALL BACK*/
size_t data_write_callback(char* recv, size_t size, size_t nmemb, void *userdata){
    size_t real_size = size * nmemb;

    DATA_BUF* strip_data = (DATA_BUF*)userdata;
    if(strip_data->size + real_size +1 > strip_data->max_size){
        









        return CURLE_WRITE_ERROR;
    }

    memcpy(strip_data->png_data + strip_data->size, recv, real_size);
    strip_data->size += real_size;
    strip_data->png_data[strip_data->size] = 0;

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

int producer_protocol(int process_number, int num_processes){
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
    sprintf(image_num, "%d", IMG_NUM);
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

        curl_easy_setopt(curl_handle, CURLOPT_URL, temp);

        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, data_write_callback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&strip_data);

        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        res = curl_easy_perform(curl_handle);
        char fname[256];

        if((res == CURLE_OK)){
            printf("made it");
            sprintf(fname, "./output_%d.png", process_number);
            write_file(fname, strip_data.png_data, strip_data.size);



        }

        process_number = process_number + num_processes;

        free(seg);

        usleep(SLEEP_TIME * 100);
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

int run_processes(int producer_count, int consumer_count){
    int ring_buf_size = sizeof(RING_BUFFER) + (sizeof(DATA_BUF) * BUFFER_SIZE);
    int ring_buf_shmid = shmget(IPC_PRIVATE, ring_buf_size, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    int idat_buf_shmid = shmget(IPC_PRIVATE, INFLATED_DATA_SIZE, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (ring_buf_shmid == -1 || idat_buf_shmid == -1){
        perror("shmget");
        abort();
    }

    RING_BUFFER * ring_buf = shmat(ring_buf_shmid, NULL, 0);
    u_int8_t * idat_buf  = shmat(idat_buf_shmid, NULL, 0);
    if ( ring_buf == (void *) -1 || idat_buf == (void *) -1) {
        perror("shmat");
        abort();
    }
    ring_buffer_init(ring_buf, BUFFER_SIZE);

    int sem_items_shmid = shmget(IPC_PRIVATE, sizeof(sem_t), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    int sem_spaces_shmid = shmget(IPC_PRIVATE, sizeof(sem_t), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    int sem_lock_shmid = shmget(IPC_PRIVATE, sizeof(sem_t), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    sem_t * sem_items = shmat(sem_items_shmid, NULL, 0);
    sem_t * sem_spaces = shmat(sem_spaces_shmid, NULL, 0);
    sem_t * sem_lock = shmat(sem_lock_shmid, NULL, 0);
    sem_init(sem_items, SEM_PROC, 0);
    sem_init(sem_spaces, SEM_PROC, BUFFER_SIZE);
    sem_init(sem_lock, SEM_PROC, 1);

    int child_count = producer_count + consumer_count;
    pid_t pid = 0;
    pid_t children[child_count];

    for (int i = 0; i < producer_count; i++){
        pid = fork();
        if (pid > 0){
            children[i] = pid;
        }
        else if (pid == 0){
            producer_protocol(i, producer_count);
            break;
        }
        else{
            perror("fork");
            abort();
        }
    }

    for (int i = 0; i < consumer_count; i++){
        pid = fork();
        if (pid > 0){
            children[producer_count + i] = pid;
        }
        else if (pid == 0){
            //consumer_protocol();
            break;
        }
        else{
            perror("fork");
            abort();
        }
    }

    int status;

    if (pid > 0){
        for (int i = 0; i < child_count; i++){
            waitpid(children[i], &status, 0);
        }
        shmctl(ring_buf_shmid, IPC_RMID, NULL);
        shmctl(sem_items_shmid, IPC_RMID, NULL);
        shmctl(sem_spaces_shmid, IPC_RMID, NULL);
        shmctl(sem_lock_shmid, IPC_RMID, NULL);

        // Write all.png
        // Timing operations

        shmctl(idat_buf_shmid, IPC_RMID, NULL);

    }

    return 0;
}


int main(int argc, char * argv[]){
    // PARSE ARGS:
    // P, C passed to run_processes function
    // B, X, N are stored in global variables BUFFER_SIZE, SLEEP_TIME, IMG_NUM
    if (argc != 6){
        fprintf(stderr, "Incorrect arguments!\n");
        return -1;
    }
    BUFFER_SIZE = atoi(argv[1]);
    SLEEP_TIME = atoi(argv[4]);
    IMG_NUM = atoi(argv[5]);

    int p = atoi(argv[2]);
    int c = atoi(argv[3]);

    run_processes(p, c);

    // curl_global_init(CURL_GLOBAL_DEFAULT);  // init curl stuff
    
    // sem_t* buffer_spaces;
    // sem_t* buffer_items;

    // sem_t* buffer_permission;
    // producer_protocol(3, 50, 11, 1);

    return 0;
}