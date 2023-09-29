// find_png code
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>

int main(int argc, char* argv[]){
    /*findpng takes a directory name as an argument. only takes ONE argument*/

    /*VARIABLES*/
    char* ptr;
    struct stat buf;
    DIR* directory;
    struct dirent *dirent_ptr;


     if(argc != 2){
        /*CASE WHERE NO OR MULTIPLE DIRECTOY PATHS ARE PASSED*/
        printf("ERROR, please enter one (1) argument.\n");
        exit(1);
    }

    /*IDENTIFY IF THIS IS A DIRECTORY OR NOT*/
    if(directory = opendir(argv[1]) == NULL){
        // IF THE DIRECTORY PASSED IS NOT A DIRECTORY, EXIT WITH ERROR STATUS
        printf("Arg passed is not a directory. THIS NEEDS TO BE CHANGED TO 'No PNG found'\n");
        exit(1);
    }

    /*REACH HERE IF THE DIRECTORY PASSED IS A REAL DIRECTORY
      
      FUNCTION TO FIND ALL PNGS WILL ONLY TAKE INTO ACCOUNT WHETHER THE FIRST 8 BYTES 
      IDENTIFY THE FILE AS A PNG. NO CORRUPTION DETECTION WILL BE MADE
      
      BASE CASE: NO SUBDIRECTORIES, GET ALL FILES WITH THE .PNG EXTENSION AND CHECK IF IT'S A PNG
      
      OTHER CASES: SEARCH OTHER SUBDIRECTORIES. THEN, ONCE RETURNED, LOOK THROUGH THE PNG'S TO SEE IF ACTUALLY PNG*/

    while(dirent_ptr = readdir(directory) != NULL){
        printf("%s\n", dirent_ptr->d_name);
    }
    closedir(directory);
    
    
    
    
    /* lstat(argv[1], &buf);
    if(S_ISDIR(buf.st_mode)){
        // if passed is a directory
        printf("DIRECTORY FOUND!\n");
        if((directory = opendir(argv[1]))){

        }
    }
    else{
        // if path passed is not a directory
        printf("No PNG file found\n");
        exit(2);
    } */




    /**/

    exit(0);
}