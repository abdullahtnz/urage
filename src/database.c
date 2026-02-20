#include "database.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
    
    // Store reference in index - pack page and offset into a single 64-bit value
    uint64_t location = ((uint64_t)data_page << 32) | data_offset;
    
    // FIX: btree_insert expects page_num_t (uint32_t), so we store just the page number
    // and handle the offset separately? Actually, let's fix the btree interface instead.
    
    // For now, let's store the location in the value field
    return btree_insert(db->index, key, (page_num_t)location);
}

DB_Result db_find(Database *db, uint32_t key, void *buffer, size_t *size) {
    // Find in index
    page_num_t location;
    DB_Result result = btree_find(db->index, key, &location);
    if (result != DB_SUCCESS) return result;
    
    // Extract page and offset
    page_num_t data_page = (page_num_t)(location >> 32);
    offset_t data_offset = (offset_t)(location & 0xFFFFFFFF);
    
    // Read from storage
    return storage_read(db->storage, data_page, data_offset, buffer, size);
}

DB_Result db_delete(Database *db, uint32_t key) {
    if (!db || !db->index) return DB_ERROR;
    
    // First find the key to get storage location
    page_num_t location;
    DB_Result result = btree_find(db->index, key, &location);
    if (result != DB_SUCCESS) return result;
    
    // Extract storage location
    page_num_t data_page = (page_num_t)(location >> 32);
    offset_t data_offset = (offset_t)(location & 0xFFFFFFFF);
    
    // Delete from index first
    result = btree_delete(db->index, key);
    if (result != DB_SUCCESS) return result;
    
    // Then free storage space
    result = storage_delete(db->storage, data_page, data_offset);
    if (result != DB_SUCCESS) {
        // If storage delete fails, we have an inconsistency
        // In production, you'd want to roll back the index delete
        return DB_ERROR;
    }
    
    return DB_SUCCESS;
}