#ifndef PDF_LOADER_H
#define PDF_LOADER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mupdf/fitz.h>
#include <mupdf/fitz/image.h>
#include <mupdf/fitz/pixmap.h>
#include <mupdf/fitz/document.h>
#include <mupdf/fitz/context.h>
#include <sys/types.h>

typedef struct
{
  fz_context *ctx;
  fz_document *doc;

  ulong page_max;

  float zoom;

  float rotate;

  fz_matrix ctm;

} fz_data_t;

fz_data_t *fz_data_struct;

typedef struct
{
  fz_pixmap **fz_pixmap_collection;
  ulong page_number;
} fz_pixmap_collection_t;

fz_pixmap_collection_t *fz_pixmap_collection_struct;

fz_context *ctx;
fz_document *doc;
float zoom;
float rotate;
ulong page_max;

void ClearFzPixmapCollection();

int test_open_pdf(const char *filename);

int InitMupdf(const char *filename, const int width, const int height);
fz_pixmap* get_pdf_data_from_page(const ulong n);
ulong get_pdf_page_size();
int Scale(const int zoom_value);
int SetFzPixmapCollection(const int width, const int height);

void FzClear();

void ClearFzPixmap(fz_pixmap *pix);

#endif
