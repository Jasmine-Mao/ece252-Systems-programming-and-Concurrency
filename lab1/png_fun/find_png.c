// find_png code
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include <unistd.h>

void function(DIR *directory, char filePath[]){
    // function that takes a directory as an argument
    // goes through all files of the directory. 
    //      if it finds a file, check if it's a png. if it is, print the file path
    //                                               if not, move on to the next file

    // if it comes across a directory, call the function again
    // function returns when all files in a given directory have been checked
    printf("%s\n", filePath);
    if(directory != NULL){
        struct dirent *dirent_pointer;
        dirent_pointer = readdir(directory);

        while((dirent_pointer = readdir(directory)) != NULL){
            if(strcmp(dirent_pointer->d_name, "..") != 0){
                // SKIPS OVER THE FIRST '..' DIRECTORY IN EACH SUBDIRECTORY --> NO GOOD INFO IN HERE
                if(dirent_pointer->d_type == 4){    
                    // FOUND A NEW DIRECTORY! CREATE A NEW DIR OBJECT AND CALL THE FUNCTION AGAIN
                        char newPath[strlen(filePath)];
                        strcpy(newPath, filePath);
                        strcat(newPath, dirent_pointer->d_name);
                        strcat(newPath, "/");

                    DIR *newDirectory = opendir(newPath);
                    if(newDirectory != NULL){
                        
                        function(newDirectory, newPath);
                    }
                }
                else if(dirent_pointer->d_type == 8){
                    // IF THE FILE IS A REGULAR FILE. NOW WE NEED TO CHECK IF IT'S A PNG

                    // BEGIN BY CHECKING THE NAME OF THE FILE(ENDING IN .png)
                    // find the last '.' in the file name, then see if the last 3 characters after the dot are 'png'
                    
                    int fileLength = strlen(dirent_pointer->d_name);
                    if(fileLength > 4){
                        // if file length at least contains enough stuff for '.png'
                        const char* lastFour = &dirent_pointer->d_name[fileLength - 4];
                        // get a pointer to the last 
                        if((strcmp(lastFour, ".png") == 0) || (strcmp(lastFour, ".PNG") == 0)){
                            // if the last 4 characters end with '.png' or '.PNG', as per the lab manual
                            char temp[strlen(filePath)];
                            strcpy(temp, filePath);
                            strcat(temp, dirent_pointer->d_name);
                            printf("%s\n", temp);
                        }
                    }
                }
                
            }
        }
        closedir(directory);
    }
}

int is_png(const char *fpath){
    // ensure file is valid type (regular) 
    // NOTE @jasmine: if you would rather redo this part in find_png
    // feel free since we rlly just need to check header here,
    // i thought i might as well include it in this function
    
    struct stat type_buffer;
    
    FILE *f = fopen(fpath, "r");
    if (!f) {
        perror("fopen");
        return -1;
    }

    char * read_buffer = malloc(4);     // input buffer for reading 4 bytes

    fread(read_buffer, sizeof(read_buffer), 1, f);

    if (read_buffer[0] != 0x89 || 
        read_buffer[1] != 0x50 ||
        read_buffer[2] != 0x4E || 
        read_buffer[3] != 0x47) {
        return -1;
    }

    return 0;
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
    function(directory, strcat(argv[0], "/"));
    
    return 0;
}