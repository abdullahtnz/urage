// db.h
#ifndef DB_H
#define DB_H

#include <stdio.h>
#include <stdint.h>

#define PAGE_SIZE 4096
#define MAGIC_NUMBER 0xDB1234

typedef struct {
    uint32_t magic;          // File format identifier
    uint32_t version;         // Version number
    uint64_t page_count;      // Total pages in file
    uint64_t free_list_head;  // First free page
} DatabaseHeader;

typedef struct {
    uint32_t key;
    uint32_t value_size;
    uint64_t value_offset;    // Position in file
} Record;

// Basic operations
int db_open(const char *filename, DatabaseHeader *header);
int db_close(FILE *db_file);
int db_insert(FILE *db_file, uint32_t key, const void *value, uint32_t value_size);
int db_find(FILE *db_file, uint32_t key, void *value, uint32_t *value_size);

#endif