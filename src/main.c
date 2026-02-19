#include "database.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void print_help() {
    printf("\nSimple Database Commands:\n");
    printf("  insert <key> <value>   - Insert key-value pair\n");
    printf("  find <key>             - Find value by key\n");
    printf("  delete <key>           - Delete key\n");
    printf("  print                  - Print B-tree structure\n");
    printf("  help                   - Show this help\n");
    printf("  exit                   - Exit program\n");
}

int main(int argc, char *argv[]) {
    const char *db_name = argc > 1 ? argv[1] : "mydb";
    
    printf("Opening database: %s\n", db_name);
    Database *db = db_open(db_name);
    if (!db) {
        printf("Failed to open database\n");
        return 1;
    }
    
    printf("Database ready. Type 'help' for commands.\n");
    
    char command[256];
    char value[256];
    uint32_t key;
    
    while (1) {
        printf("\ndb> ");
        fflush(stdout);
        
        if (!fgets(command, sizeof(command), stdin)) {
            break;
        }
        
        // Remove newline
        command[strcspn(command, "\n")] = 0;
        
        // Parse command
        if (strncmp(command, "insert", 6) == 0) {
            if (sscanf(command + 6, "%u %255s", &key, value) == 2) {
                DB_Result result = db_insert(db, key, value, strlen(value) + 1);
                if (result == DB_SUCCESS) {
                    printf("Inserted: %u -> %s\n", key, value);
                } else {
                    printf("Insert failed: %d\n", result);
                }
            } else {
                printf("Usage: insert <key> <value>\n");
            }
        }
        else if (strncmp(command, "find", 4) == 0) {
            if (sscanf(command + 4, "%u", &key) == 1) {
                char buffer[256];
                size_t size = sizeof(buffer);
                DB_Result result = db_find(db, key, buffer, &size);
                if (result == DB_SUCCESS) {
                    printf("Found: %u -> %s\n", key, buffer);
                } else {
                    printf("Key %u not found\n", key);
                }
            } else {
                printf("Usage: find <key>\n");
            }
        }
        else if (strncmp(command, "delete", 6) == 0) {
            if (sscanf(command + 6, "%u", &key) == 1) {
                DB_Result result = db_delete(db, key);
                if (result == DB_SUCCESS) {
                    printf("Deleted key %u\n", key);
                } else {
                    printf("Delete failed\n");
                }
            } else {
                printf("Usage: delete <key>\n");
            }
        }
        else if (strcmp(command, "print") == 0) {
            btree_print(db->index);
        }
        else if (strcmp(command, "help") == 0) {
            print_help();
        }
        else if (strcmp(command, "exit") == 0) {
            break;
        }
        else if (strlen(command) > 0) {
            printf("Unknown command. Type 'help' for commands.\n");
        }
    }
    
    printf("\nClosing database...\n");
    db_close(db);
    
    return 0;
}