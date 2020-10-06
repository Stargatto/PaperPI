#include "file_man.h"

// function to swap data of two nodes a and b
void swap(struct node *a, struct node *b)
{
    struct file tmp = a->data;
    a->data = b->data;
    b->data = tmp;
}

// Bubble sort the given linked list
void bubble_sort(struct node *start)
{
    int swapped;
    struct node *ptr1;
    struct node *lptr = NULL;

    /* Checking for empty list */
    if (start == NULL)
        return;

    do {
        swapped = 0;
        ptr1 = start;

        while (ptr1->next != lptr) {
            if (strcasecmp(ptr1->data.file_name, ptr1->next->data.file_name) > 0) {
                swap(ptr1, ptr1->next);
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (swapped);
}

int is_directory(const char *path)
{
        struct stat statbuf;
        if(stat(path, &statbuf) != 0) {
                fprintf(stderr, "Error while scanning %s\n", path);
                return(-1);
        }
        return S_ISDIR(statbuf.st_mode);
}

off_t fsize(const char *filename)
{
        struct stat st;

        if (stat(filename, &st) == 0)
                return st.st_size;

        fprintf(stderr, "Cannot determine size of %s: %s\n",
                        filename, strerror(errno));

        return -1;
}

struct node* scan_directory(char *cur_dir, struct node* list)
{
        DIR *dir;
        struct dirent *ent;

        if((dir = opendir(cur_dir)) != NULL) {
                while ((ent = readdir(dir)) != NULL) {

                        if(strncmp("..", ent->d_name, 2)==0 || strncmp(".", ent->d_name, 1) == 0) {
                                continue;
                        }

                        char new_file[FILENAME_MAX] = "";
                        char new_filepath[FILENAME_MAX] = "";
                        strcat(new_file, ent->d_name);
                        strcpy(new_filepath, cur_dir);
                        strcat(new_filepath, "/");
                        strcat(new_filepath, ent->d_name);

                        if(is_directory(new_filepath)==1) {
                                struct node* tmp = NULL;
                                tmp = scan_directory(new_filepath, list);
                                if(tmp != NULL) {
                                        list = tmp;
                                }
                        } else if(strcasestr(new_filepath, ".pdf") != 0){
                                struct node* tmp = NULL;
                                tmp = (struct node*)malloc(sizeof(struct node));
                                strcpy(tmp->data.file_name, new_file);
                                strcpy(tmp->data.file_path, new_filepath);
                                tmp->data.size = fsize(tmp->data.file_path);
                                tmp->next = list;
                                list = tmp;
                        }

                }
                closedir(dir);

                bubble_sort(list);
                return list;
        } else {
                perror("Could not open directory.");
                return NULL;
        }
}
