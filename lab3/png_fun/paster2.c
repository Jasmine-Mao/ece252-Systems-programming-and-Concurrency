#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "png_utils/cat_png.h"


// likely we still want a curl header and data callback function, but the data one will use the ENTIRE PNG
// curl header function
// curl data function

int consumer_protocol(){

    // this is where the consumer parses the png data (from our buffer) for the idat data, inflates and stores it

    // we can loop here

    // critical section somewhere here

    return 0;
}

int producer_protocol(){

    //this is where we wanna curl and store pngs in our buffer

    // we can loop here

    // critical section somewhere here
    
    return 0;
}

int run_proccesses(int producer_count, int consumer_count){

    for (int i = 0; i < producer_count; i++){
        // if pid == child,
        // lets break then call producer_protocol 
        // or the reverse order
        // if the process is a parent we wanna keep looping
        // int pid = fork()
        // if (pid is child){
         //   producer_protocol()
         //   break
    }

    for (int i = 0; i < consumer_count; i++){
        // if pid == child,
        // lets break then call consumer_protocol 
        // or the reverse order
        // if the process is a parent we wanna keep looping
    }

    // if we are the parent at this point, we should be ready to roll (create the png)

    return 0;
}

int main(int argc, char * argv[]){
    int c;

    // PARSE ARGS:
    // P, C passed to run_processes function
    // B, X, N are set into global variables for buffer size, sleep time, and image number.

    return 0;
}