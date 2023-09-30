#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include <unistd.h>

int is_png(const char *fpath){                                                                                  /*is_png function, authored by Evelyn; modified by Jasmine*/
    
    FILE *f = fopen(fpath, "r");                                                                                /*given a filepath, open the file in readmode*/
    if (!f) {                                                                                                   /*check to ensure the file passed is a file that can be opened. by the nature of when we pass this function,*/
        perror("fopen");                                                                                        /*  we should never run into a case where the file is not regular and openable*/
        return -1;
    }

    char *read_buffer = malloc(4);                                                                              /*input buffer for reading 4 bytes*/
    fread(read_buffer, sizeof(read_buffer), 1, f);                                                              /*reads first 4 bytes of the file f. stores read into read_buffer*/

    if ((read_buffer[1] != 0x50) ||                                                                             /*compares bytes 2, 3, and 4 to see if they match the png signature*/
        (read_buffer[2] != 0x4E) || 
        (read_buffer[3] != 0x47)) {
        return -1;                                                                                              /*if signature does not match, return -1 (error). otherwise return 0*/
    }
    return 0;
}

int function(DIR *directory, char filePath[], int numPNGs){                                                     /*function that takes a directory, filepath, and the totla number of pngs as arguments. recursively searches through files for pngs*/
    if(directory != NULL){                                                                                      /*make sure directory passed is not null*/
        struct dirent *dirent_pointer;
        dirent_pointer = readdir(directory);                                                                    /*dirent_pointer now points to files in directory*/

        while((dirent_pointer = readdir(directory)) != NULL){                                                   /*iterate through all files in the directory*/
            if((strcmp(dirent_pointer->d_name, "..") != 0)&&(strcmp(dirent_pointer->d_name, "."))){             /*ignore the .. and . directories (no useful info and could trap us in a big loop)*/
                if(dirent_pointer->d_type == 4){                                                                /*if the dirent_pointer is pointing to a directory*/
                        char newPath[strlen(filePath)];
                        strcpy(newPath, filePath);
                        strcat(newPath, dirent_pointer->d_name);
                        strcat(newPath, "/");                                                                   /*create a new file path that concatenates the old file path with the new directory*/

                        DIR *newDirectory = opendir(newPath);                                                   /*create a new DIR object for the new directory we will look at*/
                        function(newDirectory, newPath, numPNGs);                                               /*call itself to iterate through the subdirectory*/
                }
                else if(dirent_pointer->d_type == 8){                                                           /*if the dirent_pointer is pointing to a regular file*/
                    int fileLength = strlen(dirent_pointer->d_name);
                    if(fileLength > 4){                                                                         /*if the file name is longer than 4, it might be a png.*/
                        const char* lastFour = &dirent_pointer->d_name[fileLength - 4];                         /*get the last 4 characters of the file. hopefully, this will be .png or .PNG*/
                        if((strcmp(lastFour, ".png") == 0) || (strcmp(lastFour, ".PNG") == 0)){                 /*compare last 4 characters with .png and .PNG*/
                            char temp[strlen(filePath)];
                            strcpy(temp, filePath);
                            strcat(temp, dirent_pointer->d_name);                                               /*create a new path for the png file. we will need the full file path for is_png*/
                            if(is_png(temp) == 0){          
                                numPNGs++;                                                                      /*if is_png returns 0, the file matches the png signature ->png*/
                                printf("%s\n", temp);                                                           /*print full png file path*/
                            }
                        }
                    }
                }  
            }
        }
        closedir(directory);                                                                                    /*one there are no more files to be read in a directory, close the directory*/
    }
    if(numPNGs == 0){
        return -1;
    }
    return 0;
}



int main(int argc, char* argv[]){
    DIR *directory;
    struct dirent *dirent_ptr;

     if(argc != 2){                                                                                              /*case where no or multiple areguments ar passed. ERROR*/
        printf("ERROR, please enter one (1) argument.\n");
        exit(1);
    } 

    /* printf("inputs (DELETE ME AFTER, CHANGE TO argv[1]!!!): ");
    scanf("%[^\n]%*c", argv[0]); */


    if(((directory = opendir(argv[1])) == NULL) || (function(directory, strcat(argv[1], "/"), 0) == -1)){                                                                 /*arg2 is the directory we will be looking for. is we try opening a directory with that name and we get NULL, the directory doesnt exist*/
        printf("findpng: No PNG file found\n");
        exit(1);
    }                               
    


    return 0;
}