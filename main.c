#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#define PATH_MAX    256

void ncurses_init(void)
{
    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    return;
}

void display_footer(WINDOW* win, int y, int x)
{   
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

    // Second pass to display with selection
    for (int i = 0; i < *file_count; i++) {
        if (i == *sel) {
            wattron(win, A_REVERSE); // Highlight selected item
        }
        mvwprintw(win, y, x, "%s", filenames[i]);
        if (i == *sel) {
            wattroff(win, A_REVERSE);
        }
        y++;
    }
    
    closedir(dir);
}

void change_directory(WINDOW* win, char* path, int* sel)
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
            snprintf(full_path, PATH_MAX, "%s/%s", path, entry->d_name);
            
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
    const char* home = getenv("HOME");
    char current_path[PATH_MAX];
    strncpy(current_path, home, PATH_MAX - 1);
    current_path[PATH_MAX - 1] = '\0';

    WINDOW* win;
    ncurses_init();
    getmaxyx(stdscr, max_y, max_x);

    int win_height = max_y - 5;
    int win_width = max_x - 5;
    int win_start_y = (max_y - win_height) / 2;  // Center vertically
    int win_start_x = (max_x - win_width) / 2;   // Center horizontally

    win = newwin(win_height, win_width, win_start_y, win_start_x);
    keypad(win, TRUE); // Enable keypad for the window
    box(win, 0, 0);


    int running = 1;
    
    int sel = 0;
    int fc = 0;
    unsigned int ch;

    list_directory(win, current_path, &sel, &fc);
    display_footer(stdscr, max_y - 1, 2);
    wrefresh(win);

    while(running) {
        ch = wgetch(win);

        switch (ch) {
            case KEY_UP:
                if (sel > 0)
                    sel--;
                else
                    sel = fc - 1;
                break;
            case KEY_DOWN:
                if (sel < fc - 1)
                    sel++;
                else
                    sel = 0;
                break;
            case 'q':
                running = 0;
                break;
            
            case 10:
                change_directory(win, current_path, &sel);
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
