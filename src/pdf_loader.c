#include "./pdf_loader.h"
#include <mupdf/fitz/context.h>
#include <mupdf/fitz/document.h>
#include <stdio.h>

void ClearFzPixmapCollection()
{
  if(fz_pixmap_collection_struct->page_number > 0 && fz_data_struct->ctx != NULL) {
    for(ulong i = 0; i < fz_pixmap_collection_struct->page_number; i++) {
      if(fz_pixmap_collection_struct->fz_pixmap_collection[i] != NULL) {
        fz_drop_pixmap(fz_data_struct->ctx, fz_pixmap_collection_struct->fz_pixmap_collection[i]);
      }
    }

    free(fz_pixmap_collection_struct->fz_pixmap_collection);
    fz_pixmap_collection_struct->fz_pixmap_collection = NULL;

    fz_pixmap_collection_struct->page_number = 0;
  }
}

void FzClear()
{
  if(fz_data_struct != NULL) {
    ClearFzPixmapCollection();
    free(fz_pixmap_collection_struct);
    fz_pixmap_collection_struct = NULL;

    if(doc != NULL && ctx != NULL) {
      fz_drop_document(ctx, doc);
    }

    if(ctx != NULL) {
      fz_drop_context(ctx);
    }


    free(fz_data_struct);

  }
}


int InitMupdf(const char *filename, const int width, const int height)
{

  rotate = 0.0;
  zoom = 1.0;

  ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);

  if(!ctx)
  {
    printf("cannot create mupdf context\n");
    return 0;
  }

  fz_try(ctx)
    fz_register_document_handlers(ctx);
  fz_catch(ctx)
  {
    printf("cannot register document handlers: %s\n", fz_caught_message(ctx));
    fz_drop_context(ctx);
    return 0;
  }

  fz_try(ctx)
    doc = fz_open_document(ctx, filename);
  fz_catch(ctx)
  {
    printf("cannot open document: %s\n", fz_caught_message(ctx));
    fz_drop_context(ctx);
    return 0;
  }

  // int page_max = 0;
  fz_try(ctx)
    page_max = fz_count_pages(ctx, doc);
  fz_catch(ctx)
  {
    printf("cannot count number of pages: %s\n", fz_caught_message(ctx));
    fz_drop_document(ctx, doc);
    fz_drop_context(ctx);

    return 0;
  }

  /*
  fz_data_struct = (fz_data_t*)malloc(sizeof(fz_data_t));

  fz_data_struct->doc = doc;
  fz_data_struct->ctx = ctx;
  fz_data_struct->page_max = page_max;
  fz_data_struct->rotate = rotate;
  fz_data_struct->zoom = zoom;
  */

  fz_pixmap_collection_struct = (fz_pixmap_collection_t*)malloc(sizeof(fz_pixmap_collection_t));
  fz_pixmap_collection_struct->fz_pixmap_collection = NULL;
  fz_pixmap_collection_struct->page_number = page_max;

  int check = SetFzPixmapCollection(width, height);

  printf("pages count of that document: %ld\n", page_max);

  return 1;
}

ulong GetPageSize()
{
  if(fz_pixmap_collection_struct->page_number > 0)
    return fz_pixmap_collection_struct->page_number;

  return 0;
}

fz_pixmap* GetPageData(const ulong n)
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
  size_t fz_collection_size = sizeof(fz_pixmap*) * page_max;
  fz_pixmap_collection_struct->fz_pixmap_collection = (fz_pixmap**)malloc(fz_collection_size);

  fz_matrix ctm = fz_scale(zoom, zoom);
  ctm = fz_pre_rotate(ctm, rotate);
  int page = 0;
  int isDetectMinimum = 0;
  while((ulong)page < page_max)
  {
    fz_pixmap *pix;
    fz_try(ctx)
      pix = fz_new_pixmap_from_page_number(ctx, doc, page, ctm, fz_device_rgb(ctx), 0);
    fz_catch(ctx)
    {
      printf("cannot render page: %s\n", fz_caught_message(ctx));
      fz_drop_document(ctx, doc);
      fz_drop_context(ctx);

      return 0;
    }

    printf("current page: %d, width: %d, height: %d\n", page, pix->w, pix->h);

    if(pix->h < height && !isDetectMinimum) {
      zoom = (float)height / (float)pix->h;

      printf("zoom: %f, isDetectMinimum: %d\n", zoom, isDetectMinimum);
      
      ctm = fz_scale(zoom, zoom);
      fz_drop_pixmap(ctx, pix);
      pix = fz_new_pixmap_from_page_number(ctx, doc, page, ctm, fz_device_rgb(ctx), 0);
      isDetectMinimum = 1;
    }


    
    printf("current page: %d, width: %d, height: %d\n", page, pix->w, pix->h);

    /* fz_bitmap *bitmap = fz_new_bitmap_from_pixmap(ctx, pix, NULL); */
    /* fz_drop_bitmap(ctx, bitmap); */

    // fz_drop_pixmap(ctx, pix);

    fz_pixmap_collection_struct->fz_pixmap_collection[page] = pix;

    page++;
  }

  return 1;
}
