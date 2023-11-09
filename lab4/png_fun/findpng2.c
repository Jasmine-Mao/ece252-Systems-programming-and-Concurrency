// from starter code
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <curl/curl.h>
/*#include <libxml/HTMLparser.h> //these aren't working for some reason
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/uri.h>*/

#define SEED_URL "http://ece252-1.uwaterloo.ca/lab4/"
#define ECE252_HEADER "X-Ece252-Fragment: "
#define BUF_SIZE 1048576  /* 1024*1024 = 1M */
#define BUF_INC  524288   /* 1024*512  = 0.5M */

#define CT_PNG  "image/png"
#define CT_HTML "text/html"
#define CT_PNG_LEN  9
#define CT_HTML_LEN 9

int main(int argc, char * argv[]){
    //./findpng2 –t=num –m=num –v=filename seed_url -- t, m, v are opt
    return 0;
}