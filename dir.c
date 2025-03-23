#include <stdlib.h>
#include <string.h>
#include "dir.h"


// Initialize a new directory listing
Directory* dir_create(int initialCapacity) {
    Directory* dir = (Directory*)malloc(sizeof(Directory));
    if (!dir) return NULL;
    
    dir->capacity = initialCapacity > 0 ? initialCapacity : 10; // Default initial capacity
    dir->count = 0;
    dir->entries = (File*)malloc(sizeof(File) * dir->capacity);
    
    if (!dir->entries) {
        free(dir);
        return NULL;
    }
    
    return dir;
}

// Resize the entries array when needed
int dir_resize(Directory* dir, int newCapacity) {
    if (!dir || newCapacity < dir->count) return 1;
    
    File* newEntries = (File*)realloc(dir->entries, sizeof(File) * newCapacity);
    if (!newEntries) return 1;
    
    dir->entries = newEntries;
    dir->capacity = newCapacity;
    return 0;
}

// Add a file entry to the directory
int dir_add_entry(Directory* dir, const File* fileInfo) {
    if (!dir || !fileInfo) return 1;
    
    // Check if we need to resize
    if (dir->count >= dir->capacity) {
        // Double the capacity
        int newCapacity = dir->capacity * 2;
        if (!dir_resize(dir, newCapacity)) {
            return 1;
        }
    }
    
    // Add the new entry
    memcpy(&dir->entries[dir->count], fileInfo, sizeof(File));
    dir->count++;
    return 0;
}

// Remove an entry by index
int dir_remove_entry(Directory* dir, int index) {
    if (!dir || index < 0 || index >= dir->count) return 1;
    
    // Move all entries after the removed one
    if (index < dir->count - 1) {
        memmove(&dir->entries[index], &dir->entries[index + 1], 
                sizeof(File) * (dir->count - index - 1));
    }
    
    dir->count--;
    return 0;
}

// Find an entry by name (assuming File has a name field)
int dir_find_entry_by_name(Directory* dir, const char* name) {
    if (!dir || !name) return -1;
    
    for (int i = 0; i < dir->count; i++) {
        if (strcmp(dir->entries[i].name, name) == 0) {
            return i;
        }
    }
    
    return -1; // Not found
}

// Clear all entries but keep the allocated memory
void dir_clear(Directory* dir) {
    if (!dir) return;
    dir->count = 0;
}

// Free all memory associated with the directory
void dir_free(Directory* dir) {
    if (!dir) return;
    
    if (dir->entries) {
        free(dir->entries);
    }
    
    free(dir);
}
