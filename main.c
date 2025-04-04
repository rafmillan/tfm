#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "dir.h"
#include "util.h"

#define PATH_MAX    256
#define PRIMARY_W   NAME_W + SIZE_W + MODIFIED_W + KIND_W

void ncurses_init(void)
{
    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    return;
}

int scroll_offset = 0;

int get_file_info(File* info, const char* path) {
    struct stat file_stat;
    
    // Get file stats
    if (stat(path, &file_stat) != 0) {
        // Error getting file info
        strncpy(info->size, "Error", sizeof(info->size) - 1);
        strncpy(info->modified, "Error", sizeof(info->modified) - 1);
        strncpy(info->kind, "Unknown", sizeof(info->kind) - 1);
        return 0;
    }
    
    // Get file size
    if (S_ISDIR(file_stat.st_mode)) {
        strncpy(info->size, "--", sizeof(info->size) - 1);
    } else {
        // Format size
        if (file_stat.st_size < 1024) {
            snprintf(info->size, sizeof(info->size), "%lld B", file_stat.st_size);
        } else if (file_stat.st_size < 1024 * 1024) {
            snprintf(info->size, sizeof(info->size), "%.1f KB", file_stat.st_size / 1024.0);
        } else if (file_stat.st_size < 1024 * 1024 * 1024) {
            snprintf(info->size, sizeof(info->size), "%.1f MB", file_stat.st_size / (1024.0 * 1024.0));
        } else {
            snprintf(info->size, sizeof(info->size), "%.1f GB", file_stat.st_size / (1024.0 * 1024.0 * 1024.0));
        }
    }
    
    // Get last modified time
    struct tm *timeinfo = localtime(&file_stat.st_mtime);
    strftime(info->modified, sizeof(info->modified), "%b %d, %Y %I:%M %p", timeinfo);
    
    // Get file type
    if (S_ISDIR(file_stat.st_mode)) {
        strncpy(info->kind, "Directory", sizeof(info->kind) - 1);
    } else if (S_ISREG(file_stat.st_mode)) {
        // Try to determine file type by extension
        const char* ext = strrchr(path, '.');
        if (ext) {
            ext++; // Skip the dot
            if (strcasecmp(ext, "txt") == 0 || strcasecmp(ext, "md") == 0) {
                strncpy(info->kind, "Text file", sizeof(info->kind) - 1);
            } else if (strcasecmp(ext, "c") == 0 || strcasecmp(ext, "h") == 0) {
                strncpy(info->kind, "C source", sizeof(info->kind) - 1);
            } else if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "png") == 0 || 
                      strcasecmp(ext, "gif") == 0 || strcasecmp(ext, "bmp") == 0) {
                strncpy(info->kind, "Image", sizeof(info->kind) - 1);
            } else if (strcasecmp(ext, "mp3") == 0 || strcasecmp(ext, "wav") == 0 || 
                      strcasecmp(ext, "ogg") == 0 || strcasecmp(ext, "flac") == 0) {
                strncpy(info->kind, "Audio", sizeof(info->kind) - 1);
            } else {
                strncpy(info->kind, ext, sizeof(info->kind) - 1);
            }
        } else {
            strncpy(info->kind, "File", sizeof(info->kind) - 1);
        }
    } else if (S_ISLNK(file_stat.st_mode)) {
        strncpy(info->kind, "Symlink", sizeof(info->kind) - 1);
    } else {
        strncpy(info->kind, "Special", sizeof(info->kind) - 1);
    }
    
    return 0;
}

void display_footer(int y, int x)
{
    wattron(stdscr, A_REVERSE);
    mvwprintw(stdscr, y, x, "%s", " ENTER ");
    wattroff(stdscr, A_REVERSE);
    x += 8;
    mvwprintw(stdscr, y, x, "%s", "Select");

    x += 12;

    wattron(stdscr, A_REVERSE);
    mvwprintw(stdscr, y, x, "%s", " Q ");
    wattroff(stdscr, A_REVERSE);
    x += 4;
    mvwprintw(stdscr, y, x, "%s", "Quit");

    wrefresh(stdscr);
}

void list_directory(WINDOW* win, const char* path, int* sel, int* file_count)
{
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(path)) == NULL) {
        perror("opendir");
        return;
    }

    *file_count = 0;
    char filenames[1000][PATH_MAX] = {0}; // Simple approach for now
    strncpy(filenames[*file_count], "..", strlen(".."));
    (*file_count)++;
    
    // First pass to collect filenames
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }
        int len = strlen(entry->d_name);
        strncpy(filenames[*file_count], entry->d_name, len);
        filenames[*file_count][len] = '\0'; // Ensure null termination
        (*file_count)++;
    }

    if (*sel >= *file_count) {
        *sel = *file_count - 1;
    }
    if (*sel < 0) {
        *sel = 0;
    }

    int x = 2;
    int y = 0;
    mvwprintw(win, y, x, "%s", path);
    y++;

    // Column widths
    int name_width = NAME_W;
    int size_width = SIZE_W;
    int kind_width = KIND_W;
    int modified_width = MODIFIED_W;

    int win_y, win_x;
    getmaxyx(win, win_y, win_x);

    // Display column headers
    wattron(win, A_BOLD);
    mvwprintw(win, y, x, "%-*s %-*s %-*s %-*s", 
              name_width, "Name",
              size_width, "Size", 
              kind_width, "Kind",
              modified_width, "Modified");
    wattroff(win, A_BOLD);
    y++;

    // Calculate available rows for files (subtract header rows, border, etc.)
    int available_rows = win_y - y - 1; // Ensure space for bottom border

    // Check if we need indicators
    int needs_top_indicator = 0;
    int needs_bottom_indicator = 0;
    
    // Adjust scroll offset if selected item is out of view
    if (*sel < scroll_offset) {
        scroll_offset = *sel;
    } else if (*sel >= scroll_offset + available_rows - 1) {
        scroll_offset = *sel - available_rows + 1;
    }

    // Ensure scroll offset is within valid range
    if (scroll_offset < 0) {
        scroll_offset = 0;
    } else if (scroll_offset > *file_count - available_rows) {
        if (*file_count > available_rows) {
            scroll_offset = *file_count - available_rows;
        } else {
            scroll_offset = 0;
        }
    }

    // Determine if we need indicators
    needs_top_indicator = (scroll_offset > 0);
    needs_bottom_indicator = (*file_count > scroll_offset + available_rows);

    // If both indicators are needed, reduce available rows by 2
    if (needs_top_indicator && needs_bottom_indicator) {
        available_rows -= 2;
    }
    // If only one indicator is needed, reduce available rows by 1
    else if (needs_top_indicator || needs_bottom_indicator) {
        available_rows -= 1;
    }

    // Display top scroll indicator if needed (before any file listing)
    if (needs_top_indicator) {
        wattron(win, A_BOLD);
        mvwprintw(win, y, x, "%-*s", PRIMARY_W, "More...");
        wattroff(win, A_BOLD);
        y++;
    }

    // Second pass to display files with scrolling
    int displayed = 0;

    for (int i = scroll_offset; i < *file_count && displayed < available_rows; i++) {
        char full_path[PATH_MAX];
        File fi = {"", "", "", ""};

        if (i == 0) {
            // Parent directory
            if (strcmp(path, "/") == 0) {
                // Root directory, parent is still root
                strcpy(full_path, "/");
            } else {
                // Create parent path
                strcpy(full_path, path);
                char* last_slash = strrchr(full_path, '/');
                if (last_slash != full_path) {
                    *last_slash = '\0';
                } else {
                    // We're in a first-level directory
                    full_path[1] = '\0';
                }
            }
        } else {
            // Regular file or directory
            if (path[strlen(path) - 1] == '/') {
                snprintf(full_path, PATH_MAX, "%s%s", path, filenames[i]);
            } else {
                snprintf(full_path, PATH_MAX, "%s/%s", path, filenames[i]);
            }
        }
        
        get_file_info(&fi, full_path);
        strncpy(fi.name, filenames[i], min((int)NAME_W - 1, (int)strlen(filenames[i])));
        fi.name[min(NAME_W - 1, strlen(filenames[i]))] = '\0'; // Ensure null termination

        if (i == *sel) {
            wattron(win, A_REVERSE); // Highlight selected item
        }
        
        // Display file name and information
        mvwprintw(win, y, x, "%-*s %-*s %-*s %-*s", 
                  name_width, fi.name,
                  size_width, fi.size, 
                  kind_width, fi.kind,
                  modified_width, fi.modified);
        
        if (i == *sel) {
            wattroff(win, A_REVERSE);
        }
        y++;
        displayed++;
    }

    // Display bottom scroll indicator if needed (after file listing)
    if (needs_bottom_indicator) {
        wattron(win, A_BOLD);
        mvwprintw(win, y, x, "%-*s", PRIMARY_W, "More...");
        wattroff(win, A_BOLD);
    }
    
    closedir(dir);
}

void change_directory(char* path, int* sel)
{
    // Special case for parent directory (..)
    if (*sel == 0) {
        // Find the last slash in the current path
        char* last_slash = strrchr(path, '/');
        
        // If we found a slash and it's not at the beginning
        if (last_slash && last_slash != path) {
            // Terminate the string at the last slash to go up one directory
            *last_slash = '\0';
        } else if (last_slash == path) {
            // We're at root (/), keep the slash but don't go higher
            path[1] = '\0';
        }
        
        *sel = 0;  // Reset selection
        scroll_offset = 0;
        return;
    }
    
    DIR *dir;
    struct dirent *entry;
    int count = 0;
    
    if ((dir = opendir(path)) == NULL) {
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }
        
        if (count == ((*sel) - 1)) {
            // Check if it's a directory
            char full_path[PATH_MAX];

            // Check if the path already ends with a slash before adding another
            if (path[strlen(path) - 1] == '/') {
                snprintf(full_path, PATH_MAX, "%s%s", path, entry->d_name);
            } else {
                snprintf(full_path, PATH_MAX, "%s/%s", path, entry->d_name);
            }
            DIR *test_dir = opendir(full_path);
            if (test_dir != NULL) {
                closedir(test_dir);
                strncpy(path, full_path, strlen(full_path));
                path[strlen(full_path)] = '\0';
                *sel = 0;
            }
            break;
        }
        count++;
    }
    closedir(dir);
}

int main() {
    int max_y, max_x;
    int win_height, win_width;
    int win_start_y, win_start_x;
    int running;

    char current_path[PATH_MAX];
    const char* home;

    WINDOW* win;
    
    scroll_offset = 0;
    home = getenv("HOME");
    strncpy(current_path, home, PATH_MAX - 1);
    current_path[PATH_MAX - 1] = '\0';

    ncurses_init();
    getmaxyx(stdscr, max_y, max_x);

    win_height = max_y - 2;
    win_width = max_x - 2;
    win_start_y = (max_y - win_height) / 2;  // Center vertically
    win_start_x = (max_x - win_width) / 2;   // Center horizontally
    win = newwin(win_height, win_width, win_start_y, win_start_x);
    keypad(win, TRUE); // Enable keypad for the window
    box(win, 0, 0);


    running = 1;
    int sel = 0;
    int fc = 0;
    unsigned int ch;
    int available_rows;

    list_directory(win, current_path, &sel, &fc);
    display_footer(max_y - 1, 2);
    wrefresh(win);
    
    while(running) {
        ch = wgetch(win);
        switch (ch) {
            case KEY_UP:
                if (sel > 0)
                    sel--;
                else
                    sel = fc - 1;
                
                if (sel < scroll_offset)
                    scroll_offset = sel;
                break;
            case KEY_DOWN:
                if (sel < fc - 1)
                    sel++;
                else
                    sel = 0;

                available_rows = win_height - 5; 
                if (sel >= scroll_offset + available_rows)
                    scroll_offset = sel - available_rows + 1;
                break;
            case 'q':
                running = 0;
                break;
            
            case 10:
                change_directory(current_path, &sel);
                break;
        }

        wclear(win);
        box(win, 0, 0);
        list_directory(win, current_path, &sel, &fc);
        wrefresh(win);
    }
    
    delwin(win);
    endwin();

    return 0;
}
