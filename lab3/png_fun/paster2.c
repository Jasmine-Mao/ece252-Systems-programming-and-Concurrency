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

#include <stdint.h>
#include <stdbool.h>
#include "png_utils/zutil.h"

#include "png_utils/cat_png.h"
#include "ring_buffer.h"
#include "paster2.h"

#define URL_1           "http://ece252-1.uwaterloo.ca:2530/image?img="
#define URL_2           "http://ece252-2.uwaterloo.ca:2530/image?img="
#define URL_3           "http://ece252-3.uwaterloo.ca:2530/image?img="
#define URL_IMAGE_SEG   "&part="

#define SEM_PROC 1
#define ECE_252_HEADER  "X-Ece252-Fragment: "
#define SEM_PROC        1

int BUFFER_SIZE;
int SLEEP_TIME;
int IMG_NUM;

atomic_bool check_img[50] = {false};
int num_fetched = 0;

sem_t * sem_items;
sem_t * sem_spaces;
sem_t * sem_lock;


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

size_t header_write_callback(char* recv, size_t size, size_t nmemb, void* userdata){
    size_t real_size = size * nmemb;
    DATA_BUF* strip_data = userdata;

    if(real_size > strlen(ECE_252_HEADER) && strncmp(recv, ECE_252_HEADER, strlen(ECE_252_HEADER)) == 0){
        strip_data->seq = atoi(recv + strlen(ECE_252_HEADER));
    }
    return real_size;
}

int consumer_protocol(void* arg, int sleeptime){ //iman
    //implement while loop somewhere, cond while num_fetched < number of segments
    sem_t* access = (sem_t*)arg;
    IMG_BUF idat_holder; //temp buffer
    IMG_BUF original_img;
    original_img.buf = malloc(10000); //max size of img
    
    while (num_fetched != 50){
    idat_holder.buf = NULL;
    idat_holder.seq = 0;
    idat_holder.size = 0;
    idat_holder.write_success = -1;

    sem_wait(access);
    /*critical section: access image, clear queue slot, exit*/
    //original_img.buf = image data //idat_holder will hold length of compressed idat
    idat_holder.seq = 0; //seq number from buffer
    sem_post(access);

    usleep(sleeptime);

    if (!check_img[idat_holder.seq]){
        /* extract idat */
        int read_index = PNG_SIG_SIZE + IHDR_CHUNK_SIZE;
        unsigned int size_buf = 0;
        memcpy(&size_buf, original_img.buf + read_index, CHUNK_LEN_SIZE);
        size_buf = ntohl(size_buf);

        idat_holder.size = (size_t)size_buf;

        read_index += CHUNK_LEN_SIZE + CHUNK_TYPE_SIZE;
        idat_holder.buf = malloc(idat_holder.size);

        memcpy(idat_holder.buf, original_img.buf + read_index, idat_holder.size);

        /* store idat */
        int store_index = idat_holder.seq * STRIP_HEIGHT * (PNG_WIDTH * 4 + 1);
        U64 inf_size;
        mem_inf(idat_data + store_index, &inf_size, idat_holder.buf, idat_holder.size);

        idat_holder.write_success = 0;
        check_img[idat_holder.seq] = true;
        free(idat_holder.buf);
        num_fetched++;
    }
    }
    free(original_img.buf);
    return 0;
}
    
    /*steps*/
    /*1. access shared memory (buffer)*/
    /*2. fetch image segments in order*/
        //set up a structure (array etc. check lab 2) to put image data fetched from buffer into -- consumers get image segment, identify image segment number, and put into respective memory slot
        //once all image segments have been found, continue onto concatenate
    /*3. inflate and concatenate all*/
    /*4. deflate once all have been concatenated*/

    /*
    take the current queued item, (head of queue)
    this item is a png binary
        find idat_length
        create temp buffer of size idat_length
        store idat data into this buffer
        inflate
        store into global shared memory, in the proper index (grabbed from header in producer)
        
    curl easy perform
    147 in paster.c
    stored index
    */

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

int producer_protocol(int process_number, int num_processes){ //does this need the buffer access semaphore passed to it
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

        curl_easy_setopt(curl_handle, CURLOPT_HEADER, header_write_callback);
        curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void*)&strip_data);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        res = curl_easy_perform(curl_handle);

        if((res == CURLE_OK)){
            printf("made it\n");
            sprintf(fname, "./output_%d.png", process_number);
            write_file(fname, strip_data.png_data, strip_data.size);
        }

        process_number += num_processes;

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
    sem_items = shmat(sem_items_shmid, NULL, 0);
    sem_spaces = shmat(sem_spaces_shmid, NULL, 0);
    sem_lock = shmat(sem_lock_shmid, NULL, 0);
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
    // ./paster2 B P C X N

    // P, C passed to run_processes function
    // B, X, N are set into global variables for buffer size, sleep time, and image number.;
    if (argc != 6){
        fprintf(stderr, "Incorrect arguments!\n");
        return -1;
    }
    BUFFER_SIZE = atoi(argv[1]);
    int p = atoi(argv[2]);
    int c = atoi(argv[3]);
    SLEEP_TIME = atoi(argv[4]);
    IMG_NUM = atoi(argv[5]);

    run_processes(p, c);

    /*int c;
    int x = 0;
    curl_global_init(CURL_GLOBAL_DEFAULT);  // init curl stuff
    
    sem_t* buffer_spaces;
    sem_t* buffer_items;

    sem_t buffer_permission;
    sem_init(&buffer_permission, 0, 1);
    producer_protocol(3, 50, 11, 1);
    consumer_protocol(&buffer_permission, x);
    sem_destroy(&buffer_permission);*/

    return 0;
}