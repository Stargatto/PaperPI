#include "file_man.h"
#include "miniGUI.h"
#include "IT8951.h"
#include "doc.h"
#include "font-14x30.h"

#define VERSION                 "v0.7"

// Input on RPi GPIO
#define PIN0                    RPI_GPIO_P1_08
#define PIN1                    RPI_GPIO_P1_10
#define PIN2                    RPI_GPIO_P1_12

#define INPUT_NONE		0
#define INPUT_FORWARD           1
#define INPUT_SELECT    	2
#define INPUT_FORWARD_10	3
#define INPUT_BACKWARD          4
#define INPUT_ROTATE            5
#define INPUT_BACKWARD_10	6
#define INPUT_EXIT		7


// From IT8951.c
extern IT8951DevInfo gstI80DevInfo;
extern uint8_t* gpFrameBuf; //Host Source Frame buffer
extern uint32_t gulImgBufAddr; //IT8951 Image buffer address

struct page_buffer {
        fbval_t *buffer;
        int height;
        int width;
};

int paperpi_init_gpio_pins() 
{
        //NOTE: bcm2835_init must already be called!!!

        // Set RPI pins to be an input
        bcm2835_gpio_fsel(PIN0, BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_fsel(PIN1, BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_fsel(PIN2, BCM2835_GPIO_FSEL_INPT);

        //  with a pullup
        bcm2835_gpio_set_pud(PIN0, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_set_pud(PIN1, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_set_pud(PIN2, BCM2835_GPIO_PUD_UP);

        return 1;
}

int paperpi_init_IT8951()
{
	if(IT8951_Init())
	{
		printf("IT8951_Init error \n");
		return 1;
	}

        return 0;
}

/*uint8_t *paperpi_convert_to_4bpp(uint8_t *gpFrameBuf)
{
        uint8_t *buf = malloc(WIN_WIDTH * WIN_HEIGHT / 2);
        for(int i = 0; i < WIN_WIDTH * WIN_HEIGHT / 2; i++) {
                uint8_t t1 = *(gpFrameBuf + i * 2);
                uint8_t t2 = *(gpFrameBuf + (i * 2) + 1);
                uint8_t res = (t1 & 0xF0) | (t2 >> 4); // Big Endian
                memcpy(buf + i, &res, 1);
        }

        return buf;
}*/

unsigned char paperpi_gpio_read(unsigned char *pin_)
{
        // pin_ identifies the previous state of GPIO pins and must be set to 0 at the begin.
        // Being in pull-up configuration, when the button is pressed
        // the signal goes from HIGH to LOW.
        // The output is set to 1 when a transition 0->1 occours:
        //
        // signal: 11111111111000011111111111111111
        // output: 00000000000000010000000000000000
        //                        ^
        // Commands:
        // pin2    pin1    pin0
        // <       0       >
        // 0       1       2
        // 
        // 0       0       0       = None                  0
        // 0       0       1       = Forward               1
        // 0       1       0       = Select/Exit from page 2
        // 0       1       1       = Forward + 10 pages    3
        // 1       0       0       = Backward              4
        // 1       0       1       = Rotate                5
        // 1       1       0       = Backward + 10         6
        // 1       1       1       = Exit from PaperPI     7

        // Read some data
        unsigned char pin = (!bcm2835_gpio_lev(PIN0)) | (!bcm2835_gpio_lev(PIN1) << 1) | (!bcm2835_gpio_lev(PIN2) << 2);

        unsigned char res = ~pin & *pin_;
        *pin_ = pin;

        //fprintf(stderr, "DEBUG: read from gpio: %d\n", res);
        
	return res;
}

int paperpi_cancel_IT8951()
{
        IT8951DisplayArea(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 0); // Reset to white
        IT8951_Cancel();
        return 1;
}

struct node* paperpi_scan_directory(char *scan_dir)
{
        struct node* file_list = NULL;
        printf("Scanning: %s\n", scan_dir);

        file_list = scan_directory(scan_dir, file_list);

        if(file_list == NULL) {
                printf("No file to scan.\n");
        }

        return file_list;
}

int paperpi_select_file(struct node* file_list, char *file_path)
{
        unsigned char gpio_input;
        unsigned char pin_ = 0;
        struct node* iter = file_list;
        int s = 0, j = 0;

        do {
                gpio_input = paperpi_gpio_read(&pin_);

                switch(gpio_input) {
                        case INPUT_NONE:
                                break;
                        case INPUT_FORWARD:
                                s++;
                                break;
                        case INPUT_BACKWARD:
                                s--;
                        case INPUT_FORWARD_10:
                                s+=10;
                                break;
                        case INPUT_BACKWARD_10:
                                s-=10;
                                break;
                        case INPUT_EXIT:
                                return -1;
                                break;
                }

                //-------------------------------------------------------------
                // DELAY
                //-------------------------------------------------------------
                delay(200);

        } while (gpio_input != INPUT_SELECT);

        fprintf(stderr, "DEBUG: file selected %d\n", s);

        while (iter != NULL && j < s-1) {
                j++;
                iter = iter->next;
        }

        strcpy(file_path, iter->data.file_path);

        return s;
}

void paperpi_print_char(uint16_t Xpos, uint16_t Ypos, uint8_t chr)
{
        uint16_t x, y;
        uint32_t buf;
        unsigned char *start_pos = console_font_14x30 + (chr * 228);

        for(int i = 0; i < 30; i++) {
                x = Xpos;
                y = Ypos + i;
                start_pos = console_font_14x30 + (chr * 60) + (i * 2);
                buf = (*(start_pos) << 8) + *(start_pos + 1);
                for(int j = 15; j >= 0; j--)
                        if(buf & (1<<j)) EPD_DrawPixel90(x + 15 - j, y, 0x00);
        }
}

void paperpi_print_text(uint16_t Xpos, uint16_t Ypos, uint8_t *text)
{
        uint16_t x = Xpos;
        uint16_t y = Ypos;
        uint8_t buf;

	do {
		buf = *text++;
                paperpi_print_char(x, y, buf);
                x+=14;
        } while (*text != 0);
}


int paperpi_show_files(struct node *file_list)
{
	IT8951LdImgInfo stLdImgInfo;
	IT8951AreaImgInfo stAreaImgInfo;
        
        struct node* iter = file_list;
        int c = 0;

	EPD_Clear(0xff);

        char title[FILENAME_MAX];
        sprintf((char *)&title, "%c PAPERPI %s %c", 6, VERSION, 6);

	paperpi_print_text(32, 32, (uint8_t*)&title);

        while (iter != NULL) {
                c++;
                char item[FILENAME_MAX];
                sprintf((char *)&item, "%d. %s", c, (char *)iter->data.file_name);
                paperpi_print_text(32, 32 * (c + 2), (uint8_t*)&item);
                iter = iter->next;
        }

	IT8951WaitForDisplayReady();

	//Setting Load image information
	stLdImgInfo.ulStartFBAddr    = (uint32_t)gpFrameBuf;
	stLdImgInfo.usEndianType     = IT8951_LDIMG_B_ENDIAN;
	stLdImgInfo.usPixelFormat    = IT8951_4BPP; 
	stLdImgInfo.usRotate         = IT8951_ROTATE_0;
	stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;
	//Set Load Area
	stAreaImgInfo.usX      = 0;
	stAreaImgInfo.usY      = 0;
	stAreaImgInfo.usWidth  = gstI80DevInfo.usPanelW;
	stAreaImgInfo.usHeight = gstI80DevInfo.usPanelH;

	//Load Image from Host to IT8951 Image Buffer
	IT8951HostAreaPackedPixelWrite(&stLdImgInfo, &stAreaImgInfo);//Display function 2
	IT8951DisplayArea(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 2);

        //free(buf);
        return 0;
}

void paperpi_display_page(fbval_t *data, int width, int height)
{
        fbval_t *pointer;
        uint8_t pixel;

	IT8951LdImgInfo stLdImgInfo;
	IT8951AreaImgInfo stAreaImgInfo;
	
        fprintf(stderr, "DEBUG: display page\n");

	EPD_Clear(0xff);
	
        int start_x = (WIN_WIDTH - width) / 2;
        int start_y = (WIN_HEIGHT - height) / 2;

        //fprintf(stderr, "DEBUG: drawing pixels w %d h %d start_x %d start_y %d\n", width, height, start_x, start_y);
        for(int x = 0; x < width; x++)
                for(int y = 0; y < height; y++) {
                        pointer = data + x + y * width;
                        pixel = *(pointer) & 0xF0;
                        EPD_DrawPixel(start_x + x, start_y + y, pixel);
                }

        //fprintf(stderr, "DEBUG: IT8951WaitForDisplayReady\n");
	IT8951WaitForDisplayReady();

	//Setting Load image information
	stLdImgInfo.ulStartFBAddr    = (uint32_t)gpFrameBuf;
	stLdImgInfo.usEndianType     = IT8951_LDIMG_B_ENDIAN;
	stLdImgInfo.usPixelFormat    = IT8951_4BPP; 
	stLdImgInfo.usRotate         = IT8951_ROTATE_0;
	stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;
	//Set Load Area
	stAreaImgInfo.usX      = 0;
	stAreaImgInfo.usY      = 0;
	stAreaImgInfo.usWidth  = gstI80DevInfo.usPanelW;
	stAreaImgInfo.usHeight = gstI80DevInfo.usPanelH;

	//Load Image from Host to IT8951 Image Buffer
        //fprintf(stderr, "DEBUG: IT8951HostAreaPackedPixelWrite\n");
	IT8951HostAreaPackedPixelWrite(&stLdImgInfo, &stAreaImgInfo);//Display function 2
	//Display Area ?V (x,y,w,h) with mode 2 for fast gray clear mode - depends on current waveform
        //fprintf(stderr, "DEBUG: IT8951DisplayArea\n");
	IT8951DisplayArea(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 2);

}

void paperpi_move_page(struct page_buffer *dst, struct page_buffer *src)
{
        dst->buffer = src->buffer;
        dst->width = src->width;
        dst->height= src->height;
}

struct page_buffer *paperpi_create_page()
{
        struct page_buffer *tmp = malloc(sizeof(struct page_buffer));
        tmp->buffer = malloc(sizeof(fbval_t));
        
        return tmp;
}

void paperpi_free_page(struct page_buffer *dst)
{
        fprintf(stderr, "DEBUG: free memory [%d]\n", (int)dst->buffer);
        free(dst->buffer);
}


int paperpi_displayfile(char *file_name, int start_page)
{
        static struct doc *doc;
        int page = start_page;
        unsigned char gpio_input;
        unsigned char last_input;
        unsigned char pin_ = 0;
        struct page_buffer *current_page = paperpi_create_page();
        struct page_buffer *next_page = paperpi_create_page();
        struct page_buffer *previous_page = paperpi_create_page();

        doc = doc_open(file_name);

	if (!doc || !doc_pages(doc)) {
		fprintf(stderr, "Cannot open <%s>\n", file_name);
		return 1;
	}

	if (start_page < 1 || start_page > doc_pages(doc)) {
		fprintf(stderr, "Cannot open page %d\n", start_page);
		return 1;
	}

        printf("\nOpening %s (%d pages)\n", file_name, doc_pages(doc));

        current_page->buffer = doc_draw(doc, page, &current_page->width, &current_page->height);
        //fprintf(stderr, "DEBUG: w %d h %d (size: %d)\n", width, height, sizeof(*buffer));

        paperpi_display_page(current_page->buffer, current_page->width, current_page->height);
        if(start_page > 1) {
                fprintf(stderr, "DEBUG: buffer preloading (page %d)\n", page - 1);
                previous_page->buffer = doc_draw(doc, page - 1, &previous_page->width, &previous_page->height);
        }
        
        last_input = INPUT_FORWARD;

        do {
                //-------------------------------------------------------------
                // PREBUFFERING
                //-------------------------------------------------------------
                if(last_input == INPUT_FORWARD && page + 1 <= doc_pages(doc)) {
                        fprintf(stderr, "DEBUG: buffer preloading (page %d)\n", page + 1);
                        next_page->buffer = doc_draw(doc, page + 1, &next_page->width, &next_page->height);
                } else if (last_input == INPUT_BACKWARD && page > 1) {
                        previous_page->buffer = doc_draw(doc, page - 1, &previous_page->width, &previous_page->height);
                        fprintf(stderr, "DEBUG: buffer preloading (page %d)\n", page - 1);
                }
                if ((last_input == INPUT_FORWARD_10 || last_input == INPUT_FORWARD_10) && page + 1 <= doc_pages(doc)) {
                        next_page->buffer = doc_draw(doc, page + 1, &next_page->width, &next_page->height);
                        fprintf(stderr, "DEBUG: buffer preloading (page %d)\n", page + 1);
                }
                if ((last_input == INPUT_FORWARD_10 || last_input == INPUT_FORWARD_10) && page > 1) {
                        previous_page->buffer = doc_draw(doc, page - 1, &previous_page->width, &previous_page->height);
                        fprintf(stderr, "DEBUG: buffer preloading (page %d)\n", page - 1);
                }               

                //-------------------------------------------------------------
                // INPUT READING
                //-------------------------------------------------------------
                gpio_input = paperpi_gpio_read(&pin_);
		
		if(gpio_input != INPUT_NONE) {
			fprintf(stderr, "DEBUG: gpio %d (current page %d)\n", gpio_input, page);
                }	

                switch(gpio_input) {
                        case INPUT_NONE:
                                break;
                        case INPUT_FORWARD:
                                if(page + 1 <= doc_pages(doc)) {
                                        page++;
                                        paperpi_free_page(previous_page);
                                        paperpi_move_page(previous_page, current_page);
                                        paperpi_move_page(current_page, next_page);
                                        paperpi_display_page(current_page->buffer, current_page->width, current_page->height);
                                }
                                break;
                        case INPUT_BACKWARD:
                                if(page > 1) {
                                        page--;
                                        paperpi_free_page(next_page);
                                        paperpi_move_page(next_page, current_page);
                                        paperpi_move_page(current_page, previous_page);
                                        paperpi_display_page(current_page->buffer, current_page->width, current_page->height);
                                }
				break;
                        case INPUT_FORWARD_10:
                                if(page + 10 <= doc_pages(doc)) {
                                        page+=10;
                                        paperpi_free_page(previous_page);
                                        paperpi_free_page(current_page);
                                        paperpi_free_page(next_page);
                                        current_page->buffer = doc_draw(doc, page, &current_page->width, &current_page->height);
                                        paperpi_display_page(current_page->buffer, current_page->width, current_page->height);
                                }
                                break;
                        case INPUT_BACKWARD_10:
                                if(page > 10) {
                                        page-=10;
                                        paperpi_free_page(previous_page);
                                        paperpi_free_page(current_page);
                                        paperpi_free_page(next_page);
                                        current_page->buffer = doc_draw(doc, page, &current_page->width, &current_page->height);
                                        paperpi_display_page(current_page->buffer, current_page->width, current_page->height);
                                }
                                break;
                }

                last_input = gpio_input;

                //-------------------------------------------------------------
                // DELAY
                //-------------------------------------------------------------
                delay(400);

        } while (gpio_input != INPUT_EXIT);

        doc_close(doc);

        return 0;
}
