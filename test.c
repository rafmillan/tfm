#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

void list_directory(const char *path) {
    DIR *dir;
    struct dirent *entry;
    
    if ((dir = opendir(path)) == NULL) {
        perror("opendir");
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
                if (entry->d_name[0] != '.') {
            printf("%s\n", entry->d_name);
        }
    }
    
    closedir(dir);
}

int main() {
    // Get home directory path
    char *home = getenv("HOME");
    list_directory(home);
    
    return 0;
}
