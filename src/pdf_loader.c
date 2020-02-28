#include "./pdf_loader.h"
#include <mupdf/fitz/context.h>
#include <mupdf/fitz/document.h>
#include <mupdf/fitz/pixmap.h>
#include <stdio.h>

void ClearFzPixmapCollection()
{
  if(fz_pixmap_collection_struct->page_number > 0 && fz_data_struct->ctx != NULL) {

    for(ulong i = 0; i < fz_pixmap_collection_struct->page_number; i++) {

      if(fz_pixmap_collection_struct->fz_pixmap_collection[i] != NULL) {
        ClearFzPixmap(fz_pixmap_collection_struct->fz_pixmap_collection[i]);
      }

    }

    free(fz_pixmap_collection_struct->fz_pixmap_collection);
    fz_pixmap_collection_struct->fz_pixmap_collection = NULL;

    fz_pixmap_collection_struct->page_number = 0;
  }

}

void FzClear()
{

  ClearFzPixmapCollection();
  free(fz_pixmap_collection_struct);
  fz_pixmap_collection_struct = NULL;

  if(fz_data_struct->doc != NULL && fz_data_struct->ctx != NULL) {
    fz_drop_document(fz_data_struct->ctx, fz_data_struct->doc);
  }

  if(fz_data_struct->ctx != NULL) {
    fz_drop_context(fz_data_struct->ctx);
  }

  free(fz_data_struct);

}


int InitMupdf(const char *filename, const int width, const int height)
{
  fz_data_struct = (fz_data_t*)malloc(sizeof(fz_data_t));

  fz_data_struct->rotate = 0.0;
  fz_data_struct->zoom = 1.0;

  fz_data_struct->ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);

  if(!fz_data_struct->ctx)
  {
    printf("cannot create mupdf context\n");
    return 0;
  }

  fz_try(fz_data_struct->ctx)
    fz_register_document_handlers(fz_data_struct->ctx);
  fz_catch(fz_data_struct->ctx)
  {
    printf("cannot register fz_data_struct->document handlers: %s\n", fz_caught_message(fz_data_struct->ctx));
    fz_drop_context(fz_data_struct->ctx);
    return 0;
  }

  fz_try(fz_data_struct->ctx)
    fz_data_struct->doc = fz_open_document(fz_data_struct->ctx, filename);
  fz_catch(fz_data_struct->ctx)
  {
    printf("cannot open fz_data_struct->document: %s\n", fz_caught_message(fz_data_struct->ctx));
    fz_drop_context(fz_data_struct->ctx);
    return 0;
  }

  // int page_max = 0;
  fz_try(fz_data_struct->ctx)
    fz_data_struct->page_max = fz_count_pages(fz_data_struct->ctx, fz_data_struct->doc);
  fz_catch(fz_data_struct->ctx)
  {
    printf("cannot count number of pages: %s\n", fz_caught_message(fz_data_struct->ctx));
    fz_drop_document(fz_data_struct->ctx, fz_data_struct->doc);
    fz_drop_context(fz_data_struct->ctx);

    return 0;
  }

  /*
     fz_data_struct = (fz_data_t*)malloc(sizeof(fz_data_t));

     fz_data_struct->fz_data_struct->doc = fz_data_struct->doc;
     fz_data_struct->fz_data_struct->ctx = fz_data_struct->ctx;
     fz_data_struct->page_max = page_max;
     fz_data_struct->rotate = rotate;
     fz_data_struct->zoom = zoom;
     */

  fz_pixmap_collection_struct = (fz_pixmap_collection_t*)malloc(sizeof(fz_pixmap_collection_t));
  fz_pixmap_collection_struct->fz_pixmap_collection = NULL;
  fz_pixmap_collection_struct->page_number = fz_data_struct->page_max;

  int check = SetFzPixmapCollection(width, height);

  printf("pages count of that fz_data_struct->document: %ld\n", fz_data_struct->page_max);

  return 1;
}

ulong get_pdf_page_size()
{
  return fz_data_struct->page_max;
}

fz_pixmap* get_pdf_data_from_page(const ulong n)
{
  if(n >= fz_pixmap_collection_struct->page_number)
    return NULL;

  if(fz_pixmap_collection_struct->fz_pixmap_collection[n] == NULL)
    return NULL;


  return fz_pixmap_collection_struct->fz_pixmap_collection[n];
}

int SetFzPixmapCollection(const int width, const int height)
{
  if(fz_pixmap_collection_struct->fz_pixmap_collection != NULL)
    ClearFzPixmapCollection();

  // int page_max = fz_data_struct->page_max;
  size_t fz_collection_size = sizeof(fz_pixmap*) * fz_data_struct->page_max;
  fz_pixmap_collection_struct->fz_pixmap_collection = (fz_pixmap**)malloc(fz_collection_size);

  fz_data_struct->ctm = fz_scale(fz_data_struct->zoom, fz_data_struct->zoom);
  fz_data_struct->ctm = fz_pre_rotate(fz_data_struct->ctm, fz_data_struct->rotate);

  int page = 0;
  int isDetectMinimum = 0;
  while((ulong)page < fz_data_struct->page_max) {
    fz_pixmap *pix;
    fz_try(fz_data_struct->ctx)
      pix = fz_new_pixmap_from_page_number(fz_data_struct->ctx, fz_data_struct->doc, page, fz_data_struct->ctm, fz_device_rgb(fz_data_struct->ctx), 0);
    fz_catch(fz_data_struct->ctx) {
      printf("cannot render page: %s\n", fz_caught_message(fz_data_struct->ctx));
      fz_drop_document(fz_data_struct->ctx, fz_data_struct->doc);
      fz_drop_context(fz_data_struct->ctx);

      return 0;
    }

    if(pix->h < height && !isDetectMinimum) {
      fz_data_struct->zoom = (float)height / (float)pix->h;

      printf("zoom: %f, isDetectMinimum: %d\n", fz_data_struct->zoom, isDetectMinimum);

      fz_data_struct->ctm = fz_scale(fz_data_struct->zoom, fz_data_struct->zoom);
      fz_drop_pixmap(fz_data_struct->ctx, pix);
      pix = fz_new_pixmap_from_page_number(fz_data_struct->ctx, fz_data_struct->doc, page, fz_data_struct->ctm, fz_device_rgb(fz_data_struct->ctx), 0);
      isDetectMinimum = 1;
    }

    printf("current page: %d, width: %d, height: %d\n", page, pix->w, pix->h);

    /* fz_bitmap *bitmap = fz_new_bitmap_from_pixmap(fz_data_struct->ctx, pix, NULL); */
    /* fz_drop_bitmap(fz_data_struct->ctx, bitmap); */

    // fz_drop_pixmap(fz_data_struct->ctx, pix);

    fz_pixmap_collection_struct->fz_pixmap_collection[page] = pix;

    page++;
  }

  return 1;
}

void ClearFzPixmap(fz_pixmap *pix)
{
  if(pix != NULL)
    fz_drop_pixmap(fz_data_struct->ctx, pix);
}
