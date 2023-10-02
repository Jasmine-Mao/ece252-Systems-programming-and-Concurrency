#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "png_utils/png_info.h"

#include <arpa/inet.h>


/*
goal: given the paths to several png files, return a png file titled all.png that is a vertical concatenation of the given pngs
procedure:
1, locating the given pngs
2. verifying that they are pngs (is_png)
3. open pngs (fopen)


3. reading the pngs and their data
a. inflate w zlib
b. identify ihdr, idat, iend
4. creating a new png file; writing to the png file (appending the new pngs one by one to the file)
5. compress and create one idat
*/




int main(int argc, char* argv[]){
    
    printf("Have %d arguments:\n", argc);
    printf("test\n");
    for (int i = 1; i < argc; ++i) {
        
        printf("%s\n", argv[i]);
        if(is_png(argv[i]) == 0){
            printf("is a png\n");
        } else {
            printf("idk\n");
        }
    }
    return 0;
}