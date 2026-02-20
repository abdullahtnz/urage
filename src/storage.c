#include "storage.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t size;
    uint8_t data[];
} StorageRecord;

Storage *storage_create(Pager *pager) {
    Storage *storage = malloc(sizeof(Storage));
    if (!storage) return NULL;
    
    storage->pager = pager;
    storage->next_offset = 0;
    
    return storage;
}

DB_Result storage_write(Storage *storage, const void *data, size_t size, page_num_t *page, offset_t *offset) {
    // Simple implementation - just use first page for now
    // In production, you'd manage multiple data pages
    *page = 1;  // Use page 1 for data storage
    
    void *page_data = pager_get_page(storage->pager, *page);
    if (!page_data) return DB_ERROR;
    
    // Check if we have space
    if (storage->next_offset + sizeof(uint32_t) + size > PAGE_SIZE) {
        return DB_FULL;
    }
    
    // Write size prefix then data
    memcpy((char *)page_data + storage->next_offset, &size, sizeof(uint32_t));
    memcpy((char *)page_data + storage->next_offset + sizeof(uint32_t), data, size);
    
    *offset = storage->next_offset;
    storage->next_offset += sizeof(uint32_t) + size;
    
    return DB_SUCCESS;
}

DB_Result storage_read(Storage *storage, page_num_t page, offset_t offset, void *buffer, size_t *size) {
    void *page_data = pager_get_page(storage->pager, page);
    if (!page_data) return DB_ERROR;
    
    // Read size prefix
    uint32_t record_size;
    memcpy(&record_size, (char *)page_data + offset, sizeof(uint32_t));
    
    if (*size < record_size) {
        *size = record_size;
        return DB_ERROR;  // Buffer too small
    }
    
    // Read data
    memcpy(buffer, (char *)page_data + offset + sizeof(uint32_t), record_size);
    *size = record_size;
    
    return DB_SUCCESS;
}

// Add this function to storage.c
DB_Result storage_delete(Storage *storage, page_num_t page, offset_t offset) {
    if (!storage || !storage->pager) return DB_ERROR;
    
    void *page_data = pager_get_page(storage->pager, page);
    if (!page_data) return DB_ERROR;
    
    // Read the size of the record (stored before the data)
    uint32_t size;
    memcpy(&size, (char *)page_data + offset, sizeof(uint32_t));
    
    // Add to free list (mark as available for reuse)
    // For simplicity, we'll just zero out the first few bytes to mark as deleted
    // A real implementation would maintain a proper free list
    uint32_t deleted_marker = 0xDEADBEEF;
    memcpy((char *)page_data + offset, &deleted_marker, sizeof(uint32_t));
    
    return DB_SUCCESS;
}