#include "database.h"
#include <stdlib.h>
#include <string.h>

Database *db_open(const char *db_name) {
    Database *db = malloc(sizeof(Database));
    if (!db) return NULL;
    
    db->name = strdup(db_name);
    
    // Construct filenames
    char index_filename[256];
    char data_filename[256];
    snprintf(index_filename, sizeof(index_filename), "%s.idx", db_name);
    snprintf(data_filename, sizeof(data_filename), "%s.dat", db_name);
    
    // Open index file
    Pager *index_pager = pager_open(index_filename);
    if (!index_pager) {
        free(db->name);
        free(db);
        return NULL;
    }
    
    // Open data file
    Pager *data_pager = pager_open(data_filename);
    if (!data_pager) {
        pager_close(index_pager);
        free(db->name);
        free(db);
        return NULL;
    }
    
    // Create index
    db->index = btree_create(index_pager);
    if (!db->index) {
        pager_close(index_pager);
        pager_close(data_pager);
        free(db->name);
        free(db);
        return NULL;
    }
    
    // Create storage
    db->storage = storage_create(data_pager);
    if (!db->storage) {
        // Cleanup
        free(db);
        return NULL;
    }
    
    return db;
}

DB_Result db_close(Database *db) {
    if (!db) return DB_ERROR;
    
    // Flush all changes
    pager_flush_all(db->index->pager);
    pager_flush_all(db->storage->pager);
    
    // Close pagers
    pager_close(db->index->pager);
    pager_close(db->storage->pager);
    
    free(db->name);
    free(db);
    
    return DB_SUCCESS;
}

DB_Result db_insert(Database *db, uint32_t key, const void *data, size_t size) {
    // Write data to storage
    page_num_t data_page;
    offset_t data_offset;
    DB_Result result = storage_write(db->storage, data, size, &data_page, &data_offset);
    if (result != DB_SUCCESS) return result;
    
    // Store reference in index
    // We pack page and offset into a single 64-bit value for simplicity
    uint64_t location = ((uint64_t)data_page << 32) | data_offset;
    return btree_insert(db->index, key, location);
}

DB_Result db_find(Database *db, uint32_t key, void *buffer, size_t *size) {
    // Find in index
    uint64_t location;
    DB_Result result = btree_find(db->index, key, &location);
    if (result != DB_SUCCESS) return result;
    
    // Extract page and offset
    page_num_t page = location >> 32;
    offset_t offset = location & 0xFFFFFFFF;
    
    // Read from storage
    return storage_read(db->storage, page, offset, buffer, size);
}

DB_Result db_delete(Database *db, uint32_t key) {
    // For simplicity, we just delete from index
    // In production, you'd also handle storage cleanup
    return btree_delete(db->index, key);
}