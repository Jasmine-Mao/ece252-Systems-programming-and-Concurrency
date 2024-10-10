#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include "png_utils/png_info.h"
#include "find_png.h"

/*
memory leaks (valgrind):
==90765== LEAK SUMMARY:
==90765==    definitely lost: 8 bytes in 2 blocks
==90765==    indirectly lost: 0 bytes in 0 blocks
==90765==      possibly lost: 0 bytes in 0 blocks
==90765==    still reachable: 8,496 bytes in 18 blocks
==90765==         suppressed: 0 bytes in 0 blocks
==90765== Rerun with --leak-check=full to see details of leaked memory
==90765== 
==90765== For lists of detected and suppressed errors, rerun with: -s
==90765== ERROR SUMMARY: 18 errors from 1 contexts (suppressed: 0 from 0)
*/

int pngs = 0;

int find_png(DIR *directory, char filePath[]){                                                     /*function that takes a directory, filepath, and the total number of pngs as arguments. recursively searches through files for pngs*/
    if (directory != NULL){
        struct dirent *dirent_pointer;
        dirent_pointer = readdir(directory);

        int len = strlen(filePath);
        char lastChar[1];
        lastChar[0] = filePath[len-1];
        if(strcmp(lastChar, "/") != 0){
            strcat(filePath, "/");
        }
        
        while ((dirent_pointer = readdir(directory)) != NULL){
            if ((strcmp(dirent_pointer->d_name, "..") != 0) && (strcmp(dirent_pointer->d_name, "."))){
                if (dirent_pointer->d_type == 4){                                                                /*if the dirent_pointer is pointing to a directory*/
                    char newPath[strlen(filePath)];
                    strcpy(newPath, filePath);
                    strcat(newPath, dirent_pointer->d_name);
                    strcat(newPath, "/");                                                                       /*create a new file path that concatenates the old file path with the new directory*/                        

                    DIR *newDirectory = opendir(newPath);
                    find_png(newDirectory, newPath);
                    // free(newDirectory);
                }
                else{
                    int fileLength = strlen(dirent_pointer->d_name);
                    if (fileLength > 4){                                                                         /*if the file name is longer than 4, it might be a png.*/
                        char temp[strlen(filePath)];                                                            /*regardless of how the file ends, check to see if the first 8 bytes includes the png marker*/
                        strcpy(temp, filePath);
                        strcat(temp, dirent_pointer->d_name);                                                   /*create a new path for the png file. we will need the full file path for is_png*/
                        if (is_png(temp) == 0){          
                            pngs++;                                                                          /*if is_png returns 0, the file matches the png signature -> png*/
                            printf("%s\n", temp);                                                               /*print full png file path*/
                        }
                    }
                }  
            }
        }
        free(dirent_pointer);
    }
    
    closedir(directory); 
    if (pngs == 0){
        return -1;
    }
    return 0;
}

int main (int argc, char* argv[]){
    DIR *directory;

    if(argc != 2){                                                                                              /*case where no or multiple arguments ar passed. ERROR*/
        printf("ERROR, please enter one (1) argument.\n");
        exit(1);
    }

    if (((directory = opendir(argv[1])) == NULL) || (find_png(directory, argv[1]) == -1)){      /*arg2 is the directory we will be looking for. is we try opening a directory with that name and we get NULL, the directory doesnt exist*/
        printf("findpng: No PNG file found\n");
        return -1;
    }
    return 0;
}