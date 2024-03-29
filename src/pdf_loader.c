#include "./pdf_loader.h"

fz_data_t *fz_data_struct = NULL;
set_fz_pix_t **result_of_thread = NULL;
fz_pixmap_collection_t *fz_pixmap_collection_struct = NULL;

void* set_fz_pixmap_with_thread(void *data)
{
  fz_context *ctx = ((set_fz_pix_t*) data)->ctx;
  fz_display_list *list = ((set_fz_pix_t*) data)->list;
  fz_rect bbox = ((set_fz_pix_t*) data)->bbox;
  fz_pixmap *pix = ((set_fz_pix_t*) data)->pix;
  fz_device *dev;
  fz_matrix ctm = ((set_fz_pix_t*) data)->ctm;

  ctx = fz_clone_context(ctx);

  dev = fz_new_draw_device(ctx, ctm, pix);

  // fz_identity
  fz_run_display_list(ctx, list, dev, fz_identity, bbox, NULL);
  fz_close_device(ctx, dev);
  fz_drop_device(ctx, dev);

  fz_drop_context(ctx);

  /* fz_try(ctx) */
  /*   pix = fz_new_pixmap_from_page_number(ctx, fz_data_struct->doc, page, fz_data_struct->ctm, fz_device_rgb(ctx), 0); */
  /* fz_catch(ctx) { */
  /*   printf("cannot render page: %s\n", fz_caught_message(ctx)); */
  /*   // fz_drop_document(fz_data_struct->ctx, fz_data_struct->doc); */
  /*   fz_drop_context(ctx); */

  /*   return 0; */
  /* } */

  /* if(pix->h < fz_data_struct->window_height) { */
  /*   float zoom = (float)fz_data_struct->window_height / (float)pix->h; */


  /*   fz_matrix ctm = fz_scale(zoom, zoom); */
  /*   fz_drop_pixmap(ctx, pix); */
  /*   pix = fz_new_pixmap_from_page_number(fz_data_struct->ctx, fz_data_struct->doc, page, ctm, fz_device_rgb(fz_data_struct->ctx), 0); */
  /* } */

  /* printf("current page: %ld, width: %d, height: %d\n", page, pix->w, pix->h); */

  /*   /\* fz_bitmap *bitmap = fz_new_bitmap_from_pixmap(fz_data_struct->ctx, pix, NULL); *\/ */
  /*   /\* fz_drop_bitmap(fz_data_struct->ctx, bitmap); *\/ */

  /*   // fz_drop_pixmap(fz_data_struct->ctx, pix); */

  /* fz_pixmap_collection_struct->fz_pixmap_collection[page] = pix; */
  /* fz_drop_context(ctx); */
  
  return data;
}

int test_open_pdf(const char *filename)
{
  fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
  if(!ctx)
    return 0;

  fz_document *doc;
  fz_try(ctx)
    fz_register_document_handlers(ctx);
  fz_catch(ctx) {
    printf("cannot register fz_data_struct->document handlers: %s\n", fz_caught_message(ctx));
    fz_drop_context(ctx);
    return 0;
  }

  fz_try(ctx)
    doc = fz_open_document(ctx, filename);
  fz_catch(ctx)
  {
    printf("cannot open fz_data_struct->document: %s\n", fz_caught_message(ctx));
    fz_drop_context(ctx);
    return 0;
  }


  fz_drop_document(ctx, doc);
  fz_drop_context(ctx);

  return 1;
}

void clear_fz_pixmap_collection()
{
  if(fz_pixmap_collection_struct != NULL) {

    if(fz_pixmap_collection_struct->page_number > 0) {
      
      for(ulong i = 0; i < fz_pixmap_collection_struct->page_number; i++) {

        if(fz_pixmap_collection_struct->fz_pixmap_collection[i] != NULL) {
          clear_fz_pixmap(fz_pixmap_collection_struct->fz_pixmap_collection[i]);
        }
      }

      free(fz_pixmap_collection_struct->fz_pixmap_collection);
      fz_pixmap_collection_struct->fz_pixmap_collection = NULL;

      fz_pixmap_collection_struct->page_number = 0;
    }


    free(fz_pixmap_collection_struct);
    fz_pixmap_collection_struct = NULL;
  }

}

void fz_clear()
{

  clear_fz_pixmap_collection();

  /* if(fz_data_struct != NULL && result_of_thread != NULL) { */
  /*   for(ulong i = 0; i < fz_data_struct->page_max; i++) { */
  /*     fz_drop_pixmap(fz_data_struct->ctx, result_of_thread[i]->pix); */

  /*     free(result_of_thread[i]); */
  /*     result_of_thread[i] = NULL; */
  /*   } */

  /*   free(result_of_thread); */
  /*   result_of_thread = NULL; */
  /* } */


  if(fz_data_struct != NULL) {

    if(fz_data_struct->ctx != NULL) {
      fz_drop_context(fz_data_struct->ctx);
    }

    free(fz_data_struct);
  }

}

void pdf_loader_lock_mutex(void *user, int lock)
{
  pthread_mutex_t *mutex = (pthread_mutex_t*)user;

  pthread_mutex_lock(&mutex[lock]);
}

void pdf_loader_unlock_mutex(void *user, int lock)
{
  pthread_mutex_t *mutex = (pthread_mutex_t*)user;
  pthread_mutex_unlock(&mutex[lock]);
}


int load_pdf(const char *filename, const int width, const int height)
{
  fz_locks_context locks;
  pthread_mutex_t mutex[FZ_LOCK_MAX];
  
  for(int i = 0; i < FZ_LOCK_MAX; i++)
    {
      pthread_mutex_init(&mutex[i], NULL);
    }
  
  locks.user = mutex;
  locks.lock = pdf_loader_lock_mutex;
  locks.unlock = pdf_loader_unlock_mutex;
    
  
  fz_data_struct = (fz_data_t*)malloc(sizeof(fz_data_t));

  fz_data_struct->rotate = 0.0;
  fz_data_struct->zoom = 1.0;

  fz_data_struct->ctx = fz_new_context(NULL, &locks, FZ_STORE_UNLIMITED);

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

  int check = set_fz_pixmap_collection(width, height);

  // printf("pages count of that fz_data_struct->document: %ld\n", fz_data_struct->page_max);

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

int set_fz_pixmap_collection(const int width, const int height)
{
  if(fz_pixmap_collection_struct->fz_pixmap_collection != NULL)
    clear_fz_pixmap_collection();

  pthread_t *thread = (pthread_t*)malloc(sizeof(pthread_t) * fz_data_struct->page_max);

  // int page_max = fz_data_struct->page_max;
  size_t fz_collection_size = sizeof(fz_pixmap*) * fz_data_struct->page_max;
  fz_pixmap_collection_struct->fz_pixmap_collection = (fz_pixmap**)malloc(fz_collection_size);

  fz_data_struct->ctm = fz_scale(fz_data_struct->zoom, fz_data_struct->zoom);
  fz_data_struct->ctm = fz_pre_rotate(fz_data_struct->ctm, fz_data_struct->rotate);

  int isMinimum = 0;

  for(int i = 0; i < (int)fz_data_struct->page_max; i++) {
      fz_page *page = fz_load_page(fz_data_struct->ctx, fz_data_struct->doc, i);

      fz_rect bbox = fz_bound_page(fz_data_struct->ctx, page);

      fz_display_list *list = fz_new_display_list(fz_data_struct->ctx, bbox);
 
      fz_device *dev = fz_new_list_device(fz_data_struct->ctx, list);

      fz_run_page(fz_data_struct->ctx, page, dev, fz_identity, NULL);
      fz_close_device(fz_data_struct->ctx, dev);
      fz_drop_device(fz_data_struct->ctx, dev);

      fz_pixmap *pix;
      // pix = fz_new_pixmap_with_bbox(fz_data_struct->ctx, fz_device_rgb(fz_data_struct->ctx), fz_round_rect(bbox), NULL, 0);
      // fz_clear_pixmap_with_value(fz_data_struct->ctx, pix, 0xff);
      pix = fz_new_pixmap_from_display_list(fz_data_struct->ctx, list, fz_data_struct->ctm, fz_device_rgb(fz_data_struct->ctx), 0);
      fz_clear_pixmap_with_value(fz_data_struct->ctx, pix, 0xff);
      // pix = fz_new_pixmap_from_page(fz_data_struct->ctx, page, fz_data_struct->ctm, fz_device_rgb(fz_data_struct->ctx), 0);

      if(pix->h < height && !isMinimum) {

        float zoom = (float)height / (float)pix->h;
        fz_drop_pixmap(fz_data_struct->ctx, pix);
        fz_data_struct->ctm = fz_scale(zoom, zoom);
        
        // bbox = fz_transform_rect(bbox, fz_data_struct->ctm);

        // fz_drop_display_list(fz_data_struct->ctx, list);
        // list = fz_new_display_list(fz_data_struct->ctx, bbox);

        // dev = fz_new_list_device(fz_data_struct->ctx, list); 
        
        // fz_run_page(fz_data_struct->ctx, page, dev, fz_identity, NULL);
        // fz_close_device(fz_data_struct->ctx, dev); 
        // fz_drop_device(fz_data_struct->ctx, dev);


        
        // pix = fz_new_pixmap_from_page(fz_data_struct->ctx, page, fz_data_struct->ctm, fz_device_rgb(fz_data_struct->ctx), 0);
        pix = fz_new_pixmap_from_display_list(fz_data_struct->ctx, list, fz_data_struct->ctm, fz_device_rgb(fz_data_struct->ctx), 0);
        fz_clear_pixmap_with_value(fz_data_struct->ctx, pix, 0xff);

        isMinimum = 1;
      }

      fz_drop_page(fz_data_struct->ctx, page);


      // fz_drop_page(fz_data_struct->ctx, page);

      set_fz_pix_t *data = (set_fz_pix_t*)malloc(sizeof(set_fz_pix_t));
      data->page = page;
      data->ctx = fz_data_struct->ctx;
      data->page_number = i;
      data->list = list;
      data->bbox = bbox;
      data->pix = pix;
      data->window_height = height;
      data->ctm = fz_data_struct->ctm;

      pthread_create(&thread[i], NULL, set_fz_pixmap_with_thread, data);
      
    }

  result_of_thread = (set_fz_pix_t**)malloc(sizeof(set_fz_pix_t*) * fz_data_struct->page_max);
  for(ulong i = 0; i < fz_data_struct->page_max; i++) {
    
    pthread_join(thread[i], (void**)&result_of_thread[i]);

    fz_rect rec = fz_transform_rect(result_of_thread[i]->bbox, result_of_thread[i]->ctm);
    fz_irect irect = fz_round_rect(rec);
    
    fz_pixmap_collection_struct->fz_pixmap_collection[result_of_thread[i]->page_number] = fz_new_pixmap_from_pixmap(fz_data_struct->ctx, result_of_thread[i]->pix, &irect);
    // fz_pixmap_collection_struct->fz_pixmap_collection[result_of_thread[i]->page_number] = result_of_thread[i]->pix;

    fz_drop_display_list(fz_data_struct->ctx, result_of_thread[i]->list);
    fz_drop_pixmap(fz_data_struct->ctx, result_of_thread[i]->pix);

    free(result_of_thread[i]);
    result_of_thread[i] = NULL;
  }

  free(result_of_thread);
  result_of_thread = NULL;

  free(thread);
  thread = NULL;

  fz_drop_document(fz_data_struct->ctx, fz_data_struct->doc);
  fz_drop_context(fz_data_struct->ctx);
  fz_data_struct->ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);


  // free(fz_data_struct);
  // fz_data_struct = NULL;

  return 1;
}

void clear_fz_pixmap(fz_pixmap *pix)
{
  if(pix != NULL) {
    fz_drop_pixmap(fz_data_struct->ctx, pix);
  }
}
