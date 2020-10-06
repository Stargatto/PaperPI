#include <stdlib.h>
#include <string.h>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-image.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>

#define MIN(a, b)	((a) < (b) ? (a) : (b))

extern "C" {
#include "doc.h"
}

struct doc {
	poppler::document *doc;
};

void *doc_draw(struct doc *doc, int p, int *width, int *height)
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

	fprintf(stderr, "DEBUG: w = %f, h = %f, rotation = %d, zoom_w = %f, zoom_h = %f, zoom = %f\n", r.width(), r.height(), page->orientation(), zoom_w, zoom_h, zoom);

	poppler::page_renderer pr;
	int h, w, br, bpp;
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
	bpp = 1; //bytes per pixel
	br = img.bytes_per_row(); // NOTE: bytes_per_row may be different from w * bpp !!!

	fprintf(stderr, "DEBUG: image rendered w = %d, h = %d, bytes_row = %d, format %d\n", w, h, br, img.format());
	dat = (unsigned char *) img.data();
	if (!(pbuf = (fbval_t *) malloc(w * h * bpp))) {
		delete page;
		return NULL;
	}

	for(int i = 0; i < h; i++)
		memcpy(pbuf + i * (w * bpp), dat + (i * br), w * bpp);

	fprintf(stderr, "DEBUG: copied %d bytes [each %d]\n", w * h * bpp, sizeof(pbuf[0]));

	*width = w;
	*height = h;

	delete page;
	return pbuf;
}

int doc_pages(struct doc *doc)
{
	return doc->doc->pages();
}

struct doc *doc_open(char *path)
{
	struct doc *doc = (struct doc *) malloc(sizeof(*doc));
	doc->doc = poppler::document::load_from_file(path);
	if (!doc->doc) {
		doc_close(doc);
		return NULL;
	}
	return doc;
}

void doc_close(struct doc *doc)
{
	delete doc->doc;
	free(doc);
}
