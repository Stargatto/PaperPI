#define WIN_WIDTH       1872
#define WIN_HEIGHT      1404

/* framebuffer depth */
typedef unsigned char fbval_t;

struct doc *doc_open(char *path);
int doc_pages(struct doc *doc);
void *doc_draw(struct doc *doc, int page, int *rows, int *cols);
void doc_close(struct doc *doc);
