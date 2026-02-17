#include "db.h"
#include <stdlib.h>
#include <string.h>

int db_open(const char *filename, DatabaseHeader *header){
    //opening file in read + write mode
    FILE *file = fopen(filename, "r+b");
    int is_new = 0;

    //creating file (w+b) if not exist
    if (!file){
        file = fopen(filename, "w+b");
        is_new = 1;
    }

    //writes inside created file
    if (is_new){
        //setting header
        header->magic = MAGIC_NUMBER;
        header->version = 1;
        header->page_count = 1;
        header->free_list_head = 0;

        //adding header
        fwrite(header, sizeof(DatabaseHeader), 1 , file);
        fflush(file);

    } else{
        fread(header, sizeof(DatabaseHeader), 1 , file);

        //checks if it is a valid db file
        if (header->magic != MAGIC_NUMBER){
            fclose(file);
            return -2;
        }
    }
    
    return 0;
}
