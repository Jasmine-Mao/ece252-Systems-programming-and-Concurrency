// find_png code
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include <unistd.h>

void function(DIR *directory){
    // function that takes a directory as an argument
    // goes through all files of the directory. 
    //      if it finds a file, check if it's a png. if it is, print the file path
    //                                               if not, move on to the next file

    // if it comes across a directory, call the function again
    // function returns when all files in a given directory have been checked
    struct dirent *dirent_pointer;
    dirent_pointer = readdir(directory);

    while((dirent_pointer = readdir(directory)) != NULL){
        
        if(strcmp(dirent_pointer->d_name, "..") != 0){
            // SKIPS OVER THE FIRST '..' DIRECTORY IN EACH SUBDIRECTORY --> NO GOOD INFO IN HERE
            if(dirent_pointer->d_type == 4){    
                // FOUND A NEW DIRECTORY! CREATE A NEW DIR OBJECT 
                DIR *newDirectory = opendir(dirent_pointer->d_name);
                // printf("%s\n", dirent_pointer->d_name);
                function(newDirectory);
            }
            // IF IT GETS HERE, THE FILE IS (PROBABLY) A NORMAL FILE. NOW WE NEED TO CHECK IF IT'S A PNG
            printf("%s\n", dirent_pointer->d_name);
        }
    }
    closedir(directory);
        /* if(dirent_pointer->d_type == 4){    // DIRECTORY
            DIR *newDirectory = opendir(dirent_pointer->d_name);
            function(newDirectory);
        }
        else if(dirent_pointer->d_type == 8){   // REGULAR FILE
            printf("regular file: %s\n", dirent_pointer->d_name);
        } */
}





int main(int argc, char* argv[]){
    /*findpng takes a directory name as an argument. only takes ONE argument*/

    /*VARIABLES*/
    DIR *directory;
    struct dirent *dirent_ptr;

    /* if(argc != 2){
        // CASE WHERE NO OR MULTIPLE DIRECTOY PATHS ARE PASSED
        printf("ERROR, please enter one (1) argument.\n");
        exit(1);
    }  */
    printf("inputs (DELETE ME AFTER, CHANGE TO argv[1]!!!): ");
    scanf("%[^\n]%*c", argv[0]);

    if((directory = opendir(argv[0])) == NULL){
        // IF THE DIRECTORY PASSED IS NOT A DIRECTORY, EXIT WITH ERROR STATUS
        printf("Arg passed is not a directory. THIS NEEDS TO BE CHANGED TO 'No PNG file found'\n");
        exit(1);
    }

    /* while((dirent_ptr = readdir(directory)) != NULL){
        // PRINTS ALL FILES AND SUBDIRECTORIES GIVEN THE DIRECTORY PATH
        printf("%s\n", dirent_ptr->d_name);
    } */
    // dirent_ptr = readdir(directory);
    function(directory);

    closedir(directory);


    return 0;
}