#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#define SERVER_1 "http://ece252-1.uwaterloo.ca:2520/image?img="
#define SERVER_2 "http://ece252-2.uwaterloo.ca:2520/image?img="
#define SERVER_3 "http://ece252-3.uwaterloo.ca:2520/image?img="
// want to somehow split the work of requesting images among all the servers (assuming server can only service one request at a time)

void paster(int numThreads, int picture){
    // function that takes the number of threads and the picture to look for
}

int main(int argv, char* argc[]){
    int c;
    int numThreads = 1; // default num of threads and images
    int imgNumber = 1;

    return 0;
}