int paperpi_writeppm(fbval_t *data, int width, int height, char *file_name)
{
        FILE *fptr;
        int bpp = 1;

        fptr = fopen(file_name,"w");

        if(fptr == NULL) {
                printf("Error!");   
                return -1;            
        }

        fprintf(fptr,"P5\n%d %d\n255\n", width, height); //P5 Gray8, P6 if RGB24 is used
        fwrite(data, width * height * bpp, 1, fptr);

        fclose(fptr);
        return 0;
}

void paperpi_display_page4(fbval_t *data, int width, int height)
{
	IT8951LdImgInfo stLdImgInfo;
	IT8951AreaImgInfo stAreaImgInfo;
	
        printf("DEBUG: display page\n");

	EPD_Clear(0xff);
	
        //int start_x = (WIN_WIDTH - width) / 2;
        //int start_y = (WIN_HEIGHT - height) / 2;

        //printf("DEBUG: drawing pixels w %d h %d start_x %d start_y %d\n", width, height, start_x, start_y);

        int pbuf_size = ((width * height) + 1) / 2; // size for the 4bit image;

        memcpy(gpFrameBuf, data, pbuf_size);

        //gpFrameBuf[y0*gstI80DevInfo.usPanelW + x0] = color

        /*for(int x = 0; x < width; x++)
                for(int y = 0; y < height; y++) {
                        pointer = data + x + y * width;
                        pixel = *(pointer) & 0xF0;
                        EPD_DrawPixel(start_x + x, start_y + y, pixel);
                }*/

        //printf("DEBUG: IT8951WaitForDisplayReady\n");
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
        //printf("DEBUG: IT8951HostAreaPackedPixelWrite\n");
	IT8951HostAreaPackedPixelWrite(&stLdImgInfo, &stAreaImgInfo);//Display function 2
	//Display Area ?V (x,y,w,h) with mode 2 for fast gray clear mode - depends on current waveform
        //printf("DEBUG: IT8951DisplayArea\n");
	IT8951DisplayArea(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 2);
}

void paperpi_welcome()
{
        printf("***********************************\n");
        printf("PaperPi Homemade E-Book Reader %s\n", VERSION);
        printf("(C) 2020 by C@rotaSoftware Corp.\n");
        printf("***********************************\n\n");
}

void *doc_draw4(struct doc *doc, int p, int *width, int *height)
{
	poppler::page *page = doc->doc->create_page(p - 1);
	poppler::rectf r = page->page_rect(poppler::page_box_enum::crop_box);

	double zoom_w, zoom_h, zoom;
	poppler::rotation_enum rotate;

	if(r.width() > r.height()) {
		rotate = poppler::rotate_180;
		zoom_w = WIN_WIDTH * 72 / r.width();
		zoom_h = WIN_HEIGHT * 72 / r.height();
	} else {
		rotate = poppler::rotate_270;
		zoom_w = WIN_WIDTH * 72 / r.height();
		zoom_h = WIN_HEIGHT * 72 / r.width();		
	}

	zoom = MIN(zoom_w, zoom_h);

	printf("DEBUG: w = %f, h = %f, rotation = %d, zoom_w = %f, zoom_h = %f, zoom = %f\n", r.width(), r.height(), page->orientation(), zoom_w, zoom_h, zoom);

	poppler::page_renderer pr;
	int h, w, br;
	fbval_t *pbuf;
	unsigned char *dat;
	pr.set_render_hint(poppler::page_renderer::antialiasing, true);
	pr.set_render_hint(poppler::page_renderer::text_antialiasing, true);
	//pr.set_image_format(poppler::image::format_rgb24); // Force to RGB 24 bit
	pr.set_image_format(poppler::image::format_gray8);
	poppler::image img = pr.render_page(page, zoom, zoom,
				-1, -1, -1, -1, rotate);
	//img.save("cippo","png", -1);	
	w = img.width();
	h = img.height();
	br = img.bytes_per_row(); // NOTE: bytes_per_row may be different from w !!!

	int pbuf_size = ((w * h) + 1) / 2; // size for the 4bit image;

	printf("DEBUG: image rendered w = %d, h = %d, bytes_row = %d, format %d\n", w, h, br, img.format());
	dat = (unsigned char *) img.data();
	if (!(pbuf = (fbval_t *) malloc(pbuf_size))) {
		delete page;
		return NULL;
	}

	unsigned char t1, t2, res;
	int c = 0;
        for(int i = 0; i < h; i++)
		for(int j = 0; j < w; j+=2) {
			t1 = *(dat + (i * br) + j);
			t2 = *(dat + (i * br) + j + 1);
			res =  (t1 & 0xF0) | (t2 >> 4); // Big Endian
			memcpy(pbuf + c, &res, 1);
		}

	printf("DEBUG: copied %d bytes\n", c);

	*width = w;
	*height = h;

	delete page;
	return pbuf;
}