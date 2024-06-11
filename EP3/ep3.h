
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


void load_directory(FILE *fs_file,Directory *dir);
void load_filesystem(FileSystem *fs, FILE *fs_file);
void save_directory(FILE *fs_file, Directory *dir);
void save_filesystem(FileSystem *fs, FILE *fs_file);
void init_fs(FileSystem *fs);
void print_directory_tree(Directory *dir, int level);
FILE * mount_fs(FileSystem *fs, const char *fs_path);
void write_block(FILE *fs_file, int block_index, const char *data);
void read_block(FILE *fs_file, int block_index, char *data);
int find_free_block(FileSystem *fs);
void copy_file(FileSystem *fs, const char *src_path, const char *dest_path, FILE *fs_file);
Directory* navigate_to_directory(Directory *root, const char *path);
void create_dir(FileSystem *fs, const char *path);
void unmount_fs(FileSystem *fs, FILE *fs_file) ;
void list_directory(const char *path, FileSystem *fs);
void delete_file(FileSystem *fs, File *file);
void delete_directory(FileSystem *fs, Directory *dir);
void delete_dir(FileSystem *fs, const char *path);
void delete_file_by_name(FileSystem *fs, const char *path);
void toca_arquivo(FileSystem *fs, const char *path);
File *find_file(Directory *dir, const char *file_name);
void mostra_arquivo(FileSystem *fs,  const char *file_path, FILE *fs_file);
void init_db(Database *db);
void fill_database(Database *db, Directory *dir, char *path);
void atualizadb(FileSystem *fs, Database *db);
void print_database(Database *db);
void busca(Database *db, const char *search_string);
void status(FileSystem *fs);
void prompt();
