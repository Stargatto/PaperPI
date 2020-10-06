#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "file_man.h"
#include "paperpi.h"
#include "miniGUI.h"
#include "IT8951.h"


int main(int argc, char *argv[])
{
        char scan_dir[FILENAME_MAX];
        char pdf_file[FILENAME_MAX];
        struct node* file_list = NULL;
        int page = 1, selected_file;

        if(argc < 2) {
                printf("Usage: ./paperpi DIRECTORY\n");
                return -1;
        } else {
                strcpy(scan_dir, argv[1]);
        }

        paperpi_init_IT8951();
	paperpi_init_gpio_pins();
        
        while(1) { //MAIN LOOP
                file_list = paperpi_scan_directory((char *)&scan_dir);
                paperpi_show_files(file_list);
                selected_file = paperpi_select_file(file_list, (char *)&pdf_file);
                if (selected_file == -1)
                        break;
                paperpi_displayfile((char *)&pdf_file, page);
        }

        paperpi_cancel_IT8951();

        return 0;
}
