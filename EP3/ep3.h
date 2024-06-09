
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BLOCK_SIZE 4096
#define MAX_FILENAME_LEN 255
#define MAX_PATH_LEN 4096
#define MAX_FILE_SIZE 104857600
#define TOTAL_BLOCKS 25600
#define MAX_FILES 1024

// Tamanhos em blocos
#define FAT_BLOCKS ((TOTAL_BLOCKS * sizeof(int) + BLOCK_SIZE - 1) / BLOCK_SIZE)
#define BITMAP_BLOCKS ((TOTAL_BLOCKS * sizeof(int) + BLOCK_SIZE - 1) / BLOCK_SIZE) 
#define ROOT_DIR_BLOCKS ((MAX_FILES * sizeof(File) + BLOCK_SIZE - 1) / BLOCK_SIZE)

#define FILESYSTEM_METADATA_BLOCKS (FAT_BLOCKS + BITMAP_BLOCKS + ROOT_DIR_BLOCKS)

typedef struct {
    char name[MAX_FILENAME_LEN];
    int size;
    time_t creation_time;
    time_t modification_time;
    time_t access_time;
    int start_block;
    int is_directory;
    struct Directory *directory;
} File;

typedef struct Directory {
    char name[MAX_FILENAME_LEN];
    File files[MAX_FILES];
    int file_count;
} Directory;

typedef struct {
    int fat[MAX_FILE_SIZE / BLOCK_SIZE];
    int bitmap[MAX_FILE_SIZE / BLOCK_SIZE];
    Directory root;
} FileSystem;

typedef struct{
    char paths [MAX_FILES][MAX_PATH_LEN];
    int count;
} Database;

void create_dir(FileSystem *fs, const char *path);

Directory* navigate_to_directory(Directory *root, const char *path);

void copy_file(FileSystem *fs, const char *src_path, const char *dest_path);

void create_file(FileSystem *fs, const char *file_path, int size) ;


