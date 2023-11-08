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

#include "png_utils/zutil.h"
#include "png_utils/cat_png.h"
#include "ring_buffer.h"
#include "paster2.h"

#define URL_1 "http://ece252-1.uwaterloo.ca:2530/image?img="
#define URL_2 "http://ece252-2.uwaterloo.ca:2530/image?img="
#define URL_3 "http://ece252-3.uwaterloo.ca:2530/image?img="

#define URL_IMAGE_SEG "&part="
#define ECE_252_HEADER "X-Ece252-Fragment: "

#define SEM_PROC 1

int BUFFER_SIZE;
int SLEEP_TIME;
int IMG_NUM;

//atomic_bool check_img[50];
int * num_downloaded;
int * num_processed;

sem_t * sem_items;
sem_t * sem_spaces;
sem_t * sem_lock;

RING_BUFFER * ring_buf;
u_int8_t * idat_data;

size_t data_write_callback(char* recv, size_t size, size_t nmemb, void *userdata){
    size_t real_size = size * nmemb;

    if(real_size > 10000){
        printf("Received item exceeds 10000 bytes!");
        return CURLE_WRITE_ERROR;
    }

    char* header_bytes = malloc(5);
    memcpy(header_bytes, recv, 4);
    header_bytes[4] = '\0';
    //printf("(Producer) I curled a %s\n", header_bytes);

    if ((header_bytes[1] != 0x50) ||
        (header_bytes[2] != 0x4E) ||
        (header_bytes[3] != 0x47)) {
        printf("CURLED GARBAGE!!\n");
        return CURLE_WRITE_ERROR;
    }
    free(header_bytes);

    DATA_BUF* strip_data = (DATA_BUF*)userdata;
    memcpy(strip_data->png_data, recv, real_size);
    strip_data->size = real_size;

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

int consumer_protocol(RING_BUFFER *ring_buf){
    DATA_BUF* idat_holder = malloc(sizeof(DATA_BUF)); 

    while (*num_processed < 50){

        /*critical section: access image, clear queue slot, exit*/ 
        printf("(Consumer) I'm waiting :3\n");
        sem_wait(sem_items);
        sem_wait(sem_lock);
        printf("(Consumer) critical section begin:\n");
        //printf("in crit sect\n");
        ring_buffer_pop(ring_buf, idat_holder);

        sem_post(sem_lock);
        sem_post(sem_spaces);
        printf("(Consumer) out of crit sect!\n");
       // printf("out crit sect\n");

        usleep(SLEEP_TIME);
        //printf("done sleep\n");
        //printf("img seq: %d\n", idat_holder->seq)
            /* extract idat */
        int read_index = PNG_SIG_SIZE + IHDR_CHUNK_SIZE;
        unsigned int compressed_size = 0;
        memcpy(&compressed_size, idat_holder->png_data + read_index, CHUNK_LEN_SIZE);
        compressed_size = ntohl(compressed_size);

        printf("compressed size: %d\n", compressed_size);

        read_index += CHUNK_LEN_SIZE + CHUNK_TYPE_SIZE;

        u_int8_t* inflate_buffer = malloc(compressed_size);

        memcpy(inflate_buffer, idat_holder->png_data + read_index, compressed_size);

        printf("extracted idat\n");
            /* store idat */
        int store_index = idat_holder->seq * STRIP_HEIGHT * (PNG_WIDTH * 4 + 1);
        U64 inf_size;
        mem_inf(idat_data + store_index, &inf_size, idat_holder->png_data, compressed_size);

        printf("stored idat\n");
        //check_img[idat_holder->seq] = 1;
        free(inflate_buffer);
        (*num_processed)++;
    }
    
    free(idat_holder);
    return 0;
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

        curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, header_write_callback);
        curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void*)&strip_data);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        res = curl_easy_perform(curl_handle);

        if((res == CURLE_OK)){
            printf("(Producer) I'm waiting :3\n");
            sem_wait(sem_spaces);
            sem_wait(sem_lock);
            printf("(Producer) critical section begin:\n");

            ring_buffer_insert(ring_buf, &strip_data);
            
            sem_post(sem_lock);
            sem_post(sem_items);

            printf("(Producer) out of crit sect!\n");
            process_number += num_processes;
            (*num_downloaded)++;
        }
        free(seg);
    }
    curl_easy_cleanup(curl_handle);
    return 0;
}

int run_processes(int producer_count, int consumer_count){
    int ring_buf_size = sizeof(RING_BUFFER) + (sizeof(DATA_BUF) * BUFFER_SIZE);
    int ring_buf_shmid = shmget(IPC_PRIVATE, ring_buf_size, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    int idat_buf_shmid = shmget(IPC_PRIVATE, INFLATED_DATA_SIZE, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    int num_processed_shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    int num_downloaded_shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (ring_buf_shmid == -1 || idat_buf_shmid == -1 || num_processed_shmid == -1 || num_downloaded_shmid == -1){
        perror("shmget");
        abort();
    }

    ring_buf = shmat(ring_buf_shmid, NULL, 0);
    idat_data = shmat(idat_buf_shmid, NULL, 0);
    num_processed = shmat(num_processed_shmid, NULL, 0);
    num_downloaded = shmat(num_downloaded_shmid, NULL, 0);
    if ( ring_buf == (void *) -1 || idat_data == (void *) -1) {
        perror("shmat");
        abort();
    }
    ring_buffer_init(ring_buf, BUFFER_SIZE);
    *num_processed = 0;
    *num_downloaded = 0;

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
            printf("Spawned producer child %d\n", i);
            producer_protocol(i, producer_count);
            exit(EXIT_SUCCESS);
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
            printf("Spawned consumer child %d\n", producer_count + i);
            consumer_protocol(ring_buf);
            exit(EXIT_SUCCESS);
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
        int result = concatenate_png(idat_data);
        if (result != 0){
            printf("catpng failure \n");
        }
    
        shmctl(idat_buf_shmid, IPC_RMID, NULL);
        curl_global_cleanup();
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
    SLEEP_TIME = atoi(argv[4]);
    IMG_NUM = atoi(argv[5]);

    int p = atoi(argv[2]);
    int c = atoi(argv[3]);

    curl_global_init(CURL_GLOBAL_DEFAULT);

    run_processes(p, c);

    return 0;
}