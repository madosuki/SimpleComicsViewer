#ifndef PDF_LOADER_H
#define PDF_LOADER_H

#include <mupdf/fitz/geometry.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <mupdf/fitz.h>
#include <mupdf/fitz/image.h>
#include <mupdf/fitz/pixmap.h>
#include <mupdf/fitz/document.h>
#include <mupdf/fitz/context.h>
#include <mupdf/fitz/display-list.h>
#include <sys/types.h>

typedef struct
{
  fz_context *ctx;
  fz_document *doc;

  ulong page_max;
  ulong current_page;

  float zoom;
  float rotate;

  fz_matrix ctm;

  int window_height;
  int window_width;

} fz_data_t;

fz_data_t *fz_data_struct;

typedef struct
{
  fz_context *ctx;
  int page_number;
  fz_display_list *list;
  fz_rect bbox;
  fz_pixmap *pix;

  fz_page *page;

  fz_matrix ctm;

  int window_height;
} set_fz_pix_t;

set_fz_pix_t **result_of_thread;

typedef struct
{
  fz_pixmap **fz_pixmap_collection;
  ulong page_number;
} fz_pixmap_collection_t;

fz_pixmap_collection_t *fz_pixmap_collection_struct;

/* fz_context *ctx; */
/* fz_document *doc; */
/* float zoom; */
/* float rotate; */
/* ulong page_max; */

void ClearFzPixmapCollection();

int test_open_pdf(const char *filename);

int load_pdf(const char *filename, const int width, const int height);
fz_pixmap* get_pdf_data_from_page(const ulong n);
ulong get_pdf_page_size();
int Scale(const int zoom_value);
int set_fz_pixmap_collection(const int width, const int height);

void FzClear();

void ClearFzPixmap(fz_pixmap *pix);

void pdf_loader_lock_mutex(void *user, int lock);
void pdf_loader_unlock_mutex(void *user, int lock);


#endif
