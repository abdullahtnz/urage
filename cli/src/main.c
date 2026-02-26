#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "urage.h"

void print_help() {
    printf("\n╔══════════════════════════════════════╗");
    printf("\n║        URAGE Database CLI            ║");
    printf("\n╚══════════════════════════════════════╝");
    
    printf("\n\n┌─ 📦 Type System Commands ─────────────────┐");
    printf("\n│  define <name> { <fields> }  - Define a new struct type");
    printf("\n│  undefine <name>             - Delete a type definition");
    printf("\n│  structs                     - List all defined types");
    printf("\n│  desc <name>                  - Describe a type's fields");
    printf("\n└────────────────────────────────────────────┘");
    
    printf("\n\n┌─ 📝 Data Commands (with types) ───────────┐");
    printf("\n│  add <type> <key> <field=value...>  - Add typed data");
    printf("\n│  get <type> <key>                   - Get typed data");
    printf("\n└────────────────────────────────────────────┘");
    
    printf("\n\n┌─ 🔢 Numeric Key Commands ─────────────────┐");
    printf("\n│  addn <key> <value>           - Insert with numeric key");
    printf("\n│  getn <key>                    - Retrieve with numeric key");
    printf("\n│  deln <key>                    - Delete with numeric key");
    printf("\n│  existsn <key>                  - Check numeric key");
    printf("\n└────────────────────────────────────────────┘");

    printf("\n\n┌─ 🔤 String Key Commands ──────────────────┐");
    printf("\n│  adds <key> <value>        - Insert with string key");
    printf("\n│  gets <key>                 - Retrieve with string key");
    printf("\n│  dels <key>                 - Delete with string key");
    printf("\n│  exists_str <key>            - Check string key");
    printf("\n└────────────────────────────────────────────┘");
    
    printf("\n\n┌─ 📊 General Commands ──────────────────────┐");
    printf("\n│  stats                       - Show database stats");
    printf("\n│  sync                        - Flush to disk");
    printf("\n│  help                        - Show this help");
    printf("\n│  exit                        - Exit program");
    printf("\n└────────────────────────────────────────────┘\n\n");
}

int main(int argc, char* argv[]) {
    const char* db_path = argc > 1 ? argv[1] : "mydb";
    
    printf("Opening database: %s\n", db_path);
    
    
    urage_db_t* db = urage_open(db_path, 0);
    if (!db) {
        printf("Failed to open database!\n");
        return 1;
    }
    
    printf("URAGE Database ready. Type 'help' for commands.\n");
    printf("\n╔══════════════════════════════════════╗");
    printf("\n║        URAGE Database CLI            ║");
    printf("\n╚══════════════════════════════════════╝");
    printf("\n")

    
    char line[512];
    char cmd[32];
    char type_name[64];
    char field_name[64];
    char field_value[256];
    uint32_t key;
    char value[256];
    
    while (1) {
        printf("\nurage> ");
        fflush(stdout);
        
        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\n")] = 0;
        
        if (strlen(line) == 0) continue;
        
        // Parse command
        if (sscanf(line, "%s", cmd) != 1) continue;
        
        // ===== TYPE SYSTEM COMMANDS =====
        // ===== TYPE SYSTEM COMMANDS =====
if (strcmp(cmd, "define") == 0) {
    // Parse: define name field1 type1 field2 type2 ...
    char name[64];
    char fields_str[256] = "";
    
    // Try to parse with braces first
    int braces_matched = sscanf(line, "define %63s { %[^}] }", name, fields_str);
    
    // If that fails, try without braces
    if (braces_matched < 2) {
        // Extract everything after "define " as fields
        char* fields_start = strchr(line, ' ');
        if (fields_start) {
            fields_start++; // Skip space
            char* name_end = strchr(fields_start, ' ');
            if (name_end) {
                // Copy name
                int name_len = name_end - fields_start;
                strncpy(name, fields_start, name_len);
                name[name_len] = '\0';
                
                // Copy fields
                strcpy(fields_str, name_end + 1);
                braces_matched = 2;
            }
        }
    }
    
    if (braces_matched >= 2) {
        printf("Defining type '%s' with fields: %s\n", name, fields_str);
        
        // Parse field definitions
        FieldDef* fields = NULL;
        uint32_t field_count = 0;
        
        if (type_parse_fields(fields_str, &fields, &field_count) == URAGE_OK) {
            // Create the type
            urage_result_t r = urage_define_type(db, name, fields, field_count);
            
            if (r == URAGE_OK) {
                printf("✅ Type '%s' defined successfully (%u fields, %u bytes total)\n", 
                       name, field_count, 
                       field_count > 0 ? fields[field_count-1].offset + fields[field_count-1].size : 0);
            } else if (r == URAGE_ERROR) {
                printf("❌ Type '%s' already exists\n", name);
            } else {
                printf("❌ Failed to define type '%s' (error %d)\n", name, r);
            }
            
            free(fields);
        } else {
            printf("❌ Invalid field definitions. Use: field type field type ...\n");
            printf("   Example: id int score int winner string(64)\n");
        }
    } else {
        printf("❌ Usage: define <name> { field1 type field2 type ... }\n");
        printf("   Example: define game { id int score int winner string(64) }\n");
        printf("   Or: define game id int score int winner string(64)\n");
    }
}
        else if (strcmp(cmd, "undefine") == 0) {
            if (sscanf(line, "undefine %63s", type_name) == 1) {
                printf("Deleting type '%s'\n", type_name);
                // TODO: Call urage_undefine_type
            } else {
                printf("Usage: undefine <name>\n");
            }
        }
        else if (strcmp(cmd, "desc") == 0) {
    if (sscanf(line, "desc %63s", type_name) == 1) {
        // Try to get the type
        TypeDef* type = urage_get_type(db, type_name);
        
        if (type) {
            printf("📌 Type: %s\n", type->name);
            printf("   ID: %u\n", type->id);
            printf("   Total size: %u bytes\n", type->size);
            printf("   Fields (%u):\n", type->field_count);
            
            // Print each field with details
            for (uint32_t i = 0; i < type->field_count; i++) {
                FieldDef* f = &type->fields[i];
                printf("     %d. %s ", i+1, f->name);
                
                if (f->type == TYPE_INT) {
                    printf("(int, offset: %u, size: %u)\n", f->offset, f->size);
                } else if (f->type == TYPE_STRING) {
                    printf("(string[%u], offset: %u, size: %u)\n", f->size, f->offset, f->size);
                } else {
                    printf("(unknown type, offset: %u, size: %u)\n", f->offset, f->size);
                }
            }
            
            free(type);
        } else {
            printf("❌ Type '%s' does not exist\n", type_name);
        }
    } else {
        printf("Usage: desc <typename>\n");
    }
}
        
        // ===== TYPED DATA COMMANDS =====
        else if (strcmp(cmd, "add") == 0) {
            // Parse: add type key field=value field=value ...
            char type[64];
            unsigned int key_val;
            char field_data[256];
            
            // This is simplified - you'd need better parsing
            if (sscanf(line, "add %63s %u %[^\n]", type, &key_val, field_data) >= 2) {
                urage_result_t r = urage_add_typed(db, type, key_val, field_data);
                if (r == URAGE_OK)
                    printf("OK: %s:%u stored\n", type, key_val);
                else if (r == URAGE_NOT_FOUND)
                    printf("Type '%s' not defined\n", type);
                else
                    printf("Error: %d\n", r);
            } else {
                printf("Usage: add <type> <key> field=value ...\n");
            }
        }
        else if (strcmp(cmd, "get") == 0) {
            // Parse: get type key
            if (sscanf(line, "get %63s %u", type_name, &key) == 2) {
                char output[512];
                urage_result_t r = urage_get_typed(db, type_name, key, output, sizeof(output));
                if (r == URAGE_OK) {
                    printf("%s:%u -> %s\n", type_name, key, output);
                } else if (r == URAGE_NOT_FOUND) {
                    printf("%s:%u not found\n", type_name, key);
                } else {
                    printf("Error: %d\n", r);
                }
            } else {
                printf("Usage: get <type> <key>\n");
            }
        }
        
        // ===== NUMERIC KEY COMMANDS =====
        else if (strcmp(cmd, "addn") == 0) {
            if (sscanf(line, "%*s %u %255s", &key, value) == 2) {
                urage_result_t r = urage_add(db, key, value, strlen(value) + 1);
                if (r == URAGE_OK)
                    printf("OK: %u -> %s\n", key, value);
                else
                    printf("Error: %d\n", r);
            } else {
                printf("Usage: addn <key> <value>\n");
            }
        }
        else if (strcmp(cmd, "getn") == 0) {
            if (sscanf(line, "%*s %u", &key) == 1) {
                char buffer[256];
                size_t size = sizeof(buffer);
                urage_result_t r = urage_get(db, key, buffer, &size);
                if (r == URAGE_OK) {
                    buffer[size] = '\0';
                    printf("%u -> %s\n", key, buffer);
                } else if (r == URAGE_NOT_FOUND) {
                    printf("Key %u not found\n", key);
                } else {
                    printf("Error: %d\n", r);
                }
            } else {
                printf("Usage: getn <key>\n");
            }
        }
        else if (strcmp(cmd, "deln") == 0) {
            if (sscanf(line, "%*s %u", &key) == 1) {
                urage_result_t r = urage_delete(db, key);
                if (r == URAGE_OK)
                    printf("Key %u deleted\n", key);
                else if (r == URAGE_NOT_FOUND)
                    printf("Key %u not found\n", key);
                else
                    printf("Error: %d\n", r);
            } else {
                printf("Usage: deln <key>\n");
            }
        }
        else if (strcmp(cmd, "existsn") == 0) {
            if (sscanf(line, "%*s %u", &key) == 1) {
                int exists = urage_exists(db, key);
                printf("Key %u %s\n", key, exists ? "exists" : "does not exist");
            } else {
                printf("Usage: existsn <key>\n");
            }
        }

        // ===== STRING KEY COMMANDS =====
else if (strcmp(cmd, "adds") == 0) {
    char str_key[256];
    if (sscanf(line, "%*s %255s %255s", str_key, value) == 2) {
        urage_result_t r = urage_put_str(db, str_key, value, strlen(value) + 1);
        if (r == URAGE_OK)
            printf("OK: '%s' -> %s\n", str_key, value);
        else
            printf("Error: %d\n", r);
    } else {
        printf("Usage: adds <str_key> <value>\n");
    }
}
else if (strcmp(cmd, "gets") == 0) {
    char str_key[256];
    if (sscanf(line, "%*s %255s", str_key) == 1) {
        char buffer[256];
        size_t size = sizeof(buffer);
        urage_result_t r = urage_get_str(db, str_key, buffer, &size);
        if (r == URAGE_OK) {
            buffer[size] = '\0';
            printf("'%s' -> %s\n", str_key, buffer);
        } else if (r == URAGE_NOT_FOUND) {
            printf("Key '%s' not found\n", str_key);
        } else {
            printf("Error: %d\n", r);
        }
    } else {
        printf("Usage: gets <str_key>\n");
    }
}
else if (strcmp(cmd, "dels") == 0) {
    char str_key[256];
    if (sscanf(line, "%*s %255s", str_key) == 1) {
        urage_result_t r = urage_del_str(db, str_key);
        if (r == URAGE_OK)
            printf("Key '%s' deleted\n", str_key);
        else if (r == URAGE_NOT_FOUND)
            printf("Key '%s' not found\n", str_key);
        else
            printf("Error: %d\n", r);
    } else {
        printf("Usage: dels <str_key>\n");
    }
}
else if (strcmp(cmd, "exists_str") == 0) {
    char str_key[256];
    if (sscanf(line, "%*s %255s", str_key) == 1) {
        int exists = urage_exists_str(db, str_key);
        printf("Key '%s' %s\n", str_key, exists ? "exists" : "does not exist");
    } else {
        printf("Usage: exists_str <str_key>\n");
    }
}
        
        // ===== GENERAL COMMANDS =====
        else if (strcmp(cmd, "stats") == 0) {
            urage_stats_t stats;
            if (urage_stats(db, &stats) == URAGE_OK) {
                printf("Database Statistics:\n");
                printf("  Keys: %llu\n", (unsigned long long)stats.keys_count);
                printf("  B-tree height: %u\n", stats.btree_height);
                printf("  Data size: %llu bytes\n", (unsigned long long)stats.data_size);
                printf("  Pages: %u\n", stats.page_count);
            } else {
                printf("Failed to get stats\n");
            }
        }
        else if (strcmp(cmd, "sync") == 0) {
            if (urage_sync(db) == URAGE_OK)
                printf("Synced to disk\n");
            else
                printf("Sync failed\n");
        }
        else if (strcmp(cmd, "help") == 0) {
            print_help();
        }
        else if (strcmp(cmd, "exit") == 0) {
            break;
        }
        else {
            printf("Unknown command. Type 'help'\n");
        }
    }
    
    urage_close(db);
    printf("Database closed.\n");
    return 0;
}