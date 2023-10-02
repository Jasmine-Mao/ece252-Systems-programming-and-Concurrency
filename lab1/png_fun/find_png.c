#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include <unistd.h>

int is_png(const char *fpath){                                                                                  /*is_png function, authored by Evelyn; modified by Jasmine*/
    
    FILE *f = fopen(fpath, "r");
    if (!f) {
        perror("fopen");
        return -1;
    }

    char *read_buffer = malloc(4);
    fread(read_buffer, sizeof(read_buffer), 1, f);

    if ((read_buffer[1] != 0x50) ||
        (read_buffer[2] != 0x4E) || 
        (read_buffer[3] != 0x47)) {
        return -1;                                                                                              /*if signature does not match, return -1 (error). otherwise return 0*/
    }
    return 0;
}

int function(DIR *directory, char filePath[], int numPNGs){                                                     /*function that takes a directory, filepath, and the total number of pngs as arguments. recursively searches through files for pngs*/
    if(directory != NULL){
        struct dirent *dirent_pointer;
        dirent_pointer = readdir(directory);
        
        while((dirent_pointer = readdir(directory)) != NULL){
            if((strcmp(dirent_pointer->d_name, "..") != 0)&&(strcmp(dirent_pointer->d_name, "."))){
                if(dirent_pointer->d_type == 4){                                                                /*if the dirent_pointer is pointing to a directory*/
                    int len = strlen(filePath);
                    char lastChar[1];
                    lastChar[0] = filePath[len - 1];
                    char newPath[strlen(filePath)];
                    if(strcmp(lastChar, "/") != 0){                                                             /*above lines just for checking if the passed directory has a / or not to prevent double printing*/
                        strcat(newPath, "/");
                    }
                    strcpy(newPath, filePath);
                    strcat(newPath, dirent_pointer->d_name);
                    strcat(newPath, "/");                                                                       /*create a new file path that concatenates the old file path with the new directory*/                        

                    DIR *newDirectory = opendir(newPath);
                    function(newDirectory, newPath, numPNGs);
                }
                else{
                    int fileLength = strlen(dirent_pointer->d_name);
                    if(fileLength > 4){                                                                         /*if the file name is longer than 4, it might be a png.*/
                        char temp[strlen(filePath)];                                                            /*regardless of how the file ends, check to see if the first 8 bytes includes the png marker*/
                        strcpy(temp, filePath);
                        strcat(temp, dirent_pointer->d_name);                                                   /*create a new path for the png file. we will need the full file path for is_png*/
                        if(is_png(temp) == 0){          
                            numPNGs++;                                                                          /*if is_png returns 0, the file matches the png signature -> png*/
                            printf("%s\n", temp);                                                               /*print full png file path*/
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

    /* if(argc != 2){
        printf("findpng: No PNG file found.\n");
        exit(1);
    }   */

    /* printf("inputs (DELETE ME AFTER, CHANGE TO argv[1]!!!): ");
    scanf("%[^\n]%*c", argv[0]); */


    if(((directory = opendir(argv[1])) == NULL) || (function(directory, argv[1], 0) == -1) || (argc != 2)){       /*arg2 is the directory we will be looking for. is we try opening a directory with that name and we get NULL, the directory doesnt exist*/
        printf("findpng: No PNG file found\n");
        exit(1);
    }                               
    


    return 0;
}