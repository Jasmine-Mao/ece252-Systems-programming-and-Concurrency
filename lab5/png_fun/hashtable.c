
#include "hashtable.h"

int hash_entries = 0;

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
        return 0;
    } else {
        return 1;
    }
}

int ht_add_url(char * url, char ** hash_data){ 
    // adds a new url to hash table, returns 1 for success
    // and 0 if the url already exists in the table (this shouldn't occur)
    ENTRY temp_url;

    hash_data[hash_entries] = strdup(url);

    temp_url.key = hash_data[hash_entries];
    temp_url.data = hash_data[hash_entries];

    //printf("ATTEMPTING TO ADD URL: %s\n", temp_url.key);

    if (errno != ENOMEM){
        if(hsearch(temp_url, ENTER) != NULL){
            hash_entries++;
            return 1;
        }
    }
    free(hash_data[hash_entries]);
    return 0;
}

void ht_cleanup(char ** hash_data){
    for (int i = 0; i < hash_entries; i++){
        free(hash_data[i]);
    }
}