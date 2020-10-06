#include <stdint.h>
#include "doc.h"

int paperpi_init_gpio_pins();
struct node* paperpi_scan_directory(char *scan_dir);
int paperpi_show_files(struct node* file_list);
void paperpi_display_page(fbval_t *data, int width, int height);
void paperpi_print_char(uint16_t Xpos,uint16_t Ypos,uint8_t chr);
void paperpi_show_test();
int paperpi_displayfile(char *file_name, int start_page);
int paperpi_select_file(struct node* file_list, char *file_path);
int paperpi_init_IT8951();
int paperpi_cancel_IT8951();