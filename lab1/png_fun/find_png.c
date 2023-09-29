// find_png code
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include <unistd.h>

void function(struct dirent *dirent_ptr){
    // function that takes a directory as an argument
    // goes through all files of the directory. 
    //      if it finds a file, check if it's a png. if it is, print the file path
    //                                               if not, move on to the next file

    // if it comes across a directory, call the function again
    // function returns when all files in a given directory have been checked

    while(dirent_ptr != NULL){
        if(dirent_ptr->d_type == 4){    // DIRECTORY
            function(dirent_ptr);
        }
        else if(dirent_ptr->d_type == 8){   // REGULAR FILE
            printf("regular file: %s\n", dirent_ptr->d_name);
        }
    }

    // gets here once all files have been read through (dirent ptr is pointing to NULL)

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

    function(dirent_ptr = readdir(directory));

    closedir(directory);


    return 0;
}