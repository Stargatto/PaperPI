#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

// File Struct
struct file {
        char file_name[FILENAME_MAX];
        char file_path[FILENAME_MAX];
        off_t size;
};

// Linked List
struct node {
        struct file data;
        struct node* next;
};

int is_directory(const char *path);
off_t fsize(const char *filename);
struct node* scan_directory(char *cur_dir, struct node* list);
void bubble_sort(struct node *start);