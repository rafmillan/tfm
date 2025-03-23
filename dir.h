/**
 * directory_listing.h
 * Dynamic directory entry management implementation
 */

#ifndef DIRECTORY_H
#define DIRECTORY_H

#define NAME_W      32
#define SIZE_W      8
#define MODIFIED_W  24
#define KIND_W      16

/**
 * @brief File structure
 *
 */
typedef struct {
    char name[NAME_W];
    char size[SIZE_W];        // Formatted size (e.g., "1.2 MB")
    char modified[MODIFIED_W];    // Last modified time (e.g., "Jan 10, 2023 12:30 PM")
    char kind[KIND_W];        // File type (e.g., "Directory", "Text file", etc.)
} File;
    

/**
 * @brief Directory listing structure
 * 
 * Dynamic array implementation for directory entries
 */
typedef struct { 
    File *entries;     /* Array of file entries */
    int count;             /* Current number of entries */
    int capacity;          /* Total capacity of the array */
} Directory;

/**
 * @brief Create a new directory listing
 * 
 * @param initialCapacity Initial capacity for entries array (use 0 for default)
 * @return Directory* Pointer to newly created listing or NULL on failure
 */
Directory* dir_create(int initialCapacity);

/**
 * @brief Resize the entries array
 * 
 * @param dir Directory listing to resize
 * @param newCapacity New capacity for the entries array
 * @return int True if successful, false otherwise
 */
int dir_resize(Directory* dir, int newCapacity);

/**
 * @brief Add a file entry to the directory
 * 
 * @param dir Directory listing
 * @param fileInfo File information to add
 * @return int True if successful, false otherwise
 */
int dir_add_entry(Directory* dir, const File* fileInfo);

/**
 * @brief Remove an entry by index
 * 
 * @param dir Directory listing
 * @param index Index of entry to remove
 * @return int True if successful, false otherwise
 */
int dir_remove_entry(Directory* dir, int index);

/**
 * @brief Find an entry by name
 * 
 * @param dir Directory listing
 * @param name Name to search for
 * @return int Index of found entry or -1 if not found
 */
int dir_find_entry_by_name(Directory* dir, const char* name);

/**
 * @brief Clear all entries but keep allocated memory
 * 
 * @param dir Directory listing to clear
 */
void dir_clear(Directory* dir);

/**
 * @brief Free all memory associated with the directory
 * 
 * @param dir Directory listing to free
 */
void dir_free(Directory* dir);

#endif /* DIRECTORY_H */
