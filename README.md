# Terminal-Based File Manager (TFM) Specification Sheet

## 1. Project Overview

A terminal-based file manager written in C that provides efficient file navigation, management, and operations through a text-based interface. This tool will focus on productivity, speed, and customizability.

## 2. Core Features

### Navigation & Display

Two-panel interface for source/destination operations
Directory tree navigation with shortcuts
File/directory details (permissions, size, timestamps)
Sorting options (by name, date, size, type)
File previews for text files

### File Operations

Basic operations: copy, move, delete, rename
Batch operations on multiple files
Archive creation/extraction (.zip, .tar.gz)
File search with regex support
File content search

### User Experience

Keyboard shortcut system
Customizable keybindings
Command history
Bookmarks for frequent locations
Custom color schemes

## 3. Technical Components

### Core Architecture

Main loop for input processing
Terminal handling (ncurses or similar library)
File system interaction layer
Configuration management

### Data Structures

Directory cache for faster navigation
File metadata storage
Command history queue
Bookmark storage

### Key Libraries to Consider

ncurses (UI rendering)
libarchive (archive handling)
regex.h (pattern matching)
getopt (command-line argument parsing)

## 4. Development Phases

### Phase 1: Core Navigation

Basic UI with single panel
Directory listing and navigation
File information display
Simple keyboard navigation

### Phase 2: File Operations

Implement copy, move, delete, rename
Add second panel
Implement file selection

### Phase 3: Advanced Features

Search functionality
Archive operations
Bookmarks and history
Configuration system

### 5. Technical Challenges

Handling terminal resize events
Efficient file operations on large directories
Cross-platform compatibility considerations
Handling large files for preview
Unicode/internationalization support

### 6. Testing Approach

Unit tests for file operations
Integration tests for UI functionality
Edge case testing (permissions, special files)
