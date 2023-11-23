
#include "hashtable.h"

// TODO: @Iman Hash table operations (ht_search_url and ht_add_url, maybe also add a ht_init?)
    // parses url string into a numerical representation (sum of ascii values) to be used as the key
    /*
    for length of url
        get url character at index
        convert to ascii
        add to sum
    divide by number of characters
    mod 10 and store
    add one and mod 10 and store
        size_t len = strlen(url);
    size_t i;
    int asciival = 0;
    char* key = url;
    for (i = 0; i < len; i ++){
        asciival = url[i];
        printf("%d", asciival);
        key[i] = (char) (asciival);
    }
    return key;
    */
//hcreate and hdestroy in main

char* url_to_key(char * url){
    char* key = url;
    return key;
}

int ht_search_url(char * url){
    // gets key from url, and invokes hsearch() with said key
    // return 1 if the url exists in the hash table, 0 otherwise
    ENTRY temp_url;
    char key_temp[256];
    char data_temp[256];

    temp_url.key = key_temp;
    temp_url.data = data_temp;
    
    strcpy(temp_url.key, url);
    strcpy(temp_url.data, url);
    
    //printf("key: %s\n", temp_url.key);
    if (hsearch(temp_url, FIND) == NULL){
        //not found
        //output errno
        return 0;
    } else {
        return 1;
    }
    return -1; //should not get here
}

int ht_add_url(char * url){ //add logfile name, if logfile not null then call write logfile func (new func), create file in main and append to file in func
    // add a url to our hash table: get its key from its string url then set corresponding value (youre gonna have to check the hsearch(3) man page)
    // for the hash table stuff)

    ENTRY temp_url;
    temp_url.key = strdup(url);
    temp_url.data = strdup(url);

    if (errno != ENOMEM){
        if(hsearch(temp_url, ENTER) != NULL){
            return 1;
        }
    }
    return 0;
}
    /*
    while ((hsearch(temp_url, ENTER) == NULL) && errno != ENOMEM){
        temp_url->key = intkey_to_charkey((url_to_key(url)+i)%500);
        i++;
    }
    free(temp_url);
    return 0;*/