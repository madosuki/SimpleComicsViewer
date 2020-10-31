#include "viewer.h"

int status = 0;

char *arg_file_name = NULL;

pthread_t thread_of_curosr_observer;

Pages *pages = NULL;
DirectoryDetail_t *detail = NULL;

Cursor_Position_t cursor_pos = {};

Image_button_t image_button = {};

Image_Container_t **image_container_list = NULL;

uncompress_data_set_t *uncompressed_file_list = NULL;

DrawingArea_t draw_area = {};

int isCompressFile = 0;

int isPDFfile = 0;

int isFirstLoad = 0;

GtkWidget *grid = NULL;

GtkApplication *app = NULL;

main_window_data_t window = {};

file_menu_t file_menu_struct = {};

view_menu_t view_menu_struct = {};

help_menu_t help_menu_struct = {};

GtkWidget *button_menu = NULL;



void free_array_with_alloced(void **list, const int size)
{
  if(list != NULL) {
    for(int i = 0; i < size; ++i) {

      if(list[i] != NULL) {
        free(list[i]);
        list[i] = NULL;
      }

    }

    free(list);
    list = NULL;

  }

}

int set_image_from_pdf_file(const char *file_name)
{
  int check = load_pdf(file_name, window.width, window.height);

  if(!check) {
    return FALSE;
  }

  if(detail == NULL)
    detail = (DirectoryDetail_t*)calloc(sizeof(DirectoryDetail_t), sizeof(DirectoryDetail_t));

  detail->image_count = get_pdf_page_size();

  return TRUE;
}

int set_image_from_compressed_file(const char *file_name)
{
  uncompressed_file_list = (uncompress_data_set_t*)calloc(sizeof(uncompress_data_set_t), sizeof(uncompress_data_set_t));
  int ret = load_from_zip(file_name, uncompressed_file_list);

  if(!ret) {
    uncompressed_file_list = NULL;
    return FALSE;
  }


  if(detail == NULL) {
    detail = (DirectoryDetail_t*)calloc(sizeof(DirectoryDetail_t), sizeof(DirectoryDetail_t));
  }

  detail->image_count = uncompressed_file_list->size;

  if(detail->image_count % 2) {
    detail->isOdd = TRUE;

  } else {
    detail->isOdd = FALSE;
  }

  return TRUE;
}

int get_image_file_count_from_directory(struct dirent **src, const int size, int *dst, const char *dirname)
{
  int *number_list = (int*)malloc(sizeof(int) * LIST_BUFFER);
  memset(number_list, 0, sizeof(int) * LIST_BUFFER);

  int dirname_length = strlen(dirname) + 1;

  int count = 1;
  for(int i = 0; i < size; ++i) {

    if(count < LIST_BUFFER) {

      int src_length = strlen(src[i]->d_name) + 1;
      char *final_path = (char*)calloc(dirname_length + src_length + 1, 1);
      if(final_path == NULL) {
        free(final_path);
        break;
      }

      strcat(final_path, dirname);
      char slash[1] = "/";
      strcat(final_path, slash);
      strcat(final_path, src[i]->d_name);

      if(detect_image_from_file(final_path)) {
        number_list[count - 1] = i;
        count++;
        int *tmp = (int*)realloc(number_list, sizeof(int) * count);
        if(tmp != NULL) {
          number_list = tmp;
        }
      }

      free(final_path);

    }

  }


  int *tmp = (int*)realloc(dst, sizeof(int) * count);
  if(tmp != NULL) {
    dst = tmp;
    memset(dst, 0, sizeof(int) * count);
    memcpy(dst, number_list, sizeof(int) * count);
  } else {
    free(tmp);
  }

  free(number_list);

  return count - 1;
}

int create_image_path_list(char **image_path_list, const char *dirname)
{
  struct dirent **file_list;

  int r = scandir(dirname, &file_list, NULL, alphasort);
  if(r < 1) {

    if(file_list != NULL) {
      free(file_list);
    }
    file_list = NULL;

    return 0;
  }

  int *number_list = (int*)malloc(sizeof(int) * LIST_BUFFER);
  memset(number_list, 0, sizeof(int) * LIST_BUFFER);

  int count = get_image_file_count_from_directory(file_list, r, number_list, dirname);

  if(count < 1) {

    free(number_list);
    number_list = NULL;

    free_array_with_alloced((void**)file_list, r);

    return 0;
  }

  if(count < LIST_BUFFER) {
    size_t image_path_list_size = sizeof(char*) * count;
    char **tmp = (char**)realloc(image_path_list, image_path_list_size);

    if(tmp != NULL) {
      image_path_list = tmp;

      for(int i = 0; i < count; ++i) {
        const int target = number_list[i];

        int dirname_length = strlen(dirname) + 1;
        const size_t target_length = strlen(file_list[target]->d_name) + 1;

        const size_t final_length = dirname_length + target_length + 1;
        char *final_path = (char*)calloc(final_length, 1);
        if(final_path == NULL) {
          free(final_path);
          break;
        }

        strcat(final_path, dirname);
        char slash[1] = "/";
        strcat(final_path, slash);
        strcat(final_path, file_list[target]->d_name);

        image_path_list[i] = (char*)calloc(dirname_length + target_length + 1, 1);

        strncpy(image_path_list[i], final_path, final_length);

        free(final_path);
      }
    } else {
      free(tmp);
    }
  }

  free(number_list);

  free_array_with_alloced((void**)file_list, r);

  return count;
}

int set_image_path_list(const char *dirname)
{
  if(detail == NULL) {
    detail = (DirectoryDetail_t*)malloc(sizeof(DirectoryDetail_t));

    if(detail != NULL) {
      memset(detail, 0, sizeof(DirectoryDetail_t));
    } else {
      return FALSE;
    }
  }


  if(detail->image_path_list == NULL) {
    detail->image_path_list = (char**)calloc(LIST_BUFFER, 1);

    if(detail->image_path_list == NULL) {
      free_array_with_alloced((void**)detail->image_path_list, LIST_BUFFER);
      return FALSE;
    }
  }

  int count = create_image_path_list(detail->image_path_list, dirname);
  if(count < 1) {
    free(detail->image_path_list);
    detail->image_path_list = NULL;

    return FALSE;
  }

  detail->image_count = count;

  if(detail->image_count % 2) {
    detail->isOdd = TRUE;
  } else {
    detail->isOdd = FALSE;
  }


  return TRUE;
}

void unref_dst()
{
  if(pages != NULL && image_container_list != NULL) {
    printf("\npages->current_page: %d\n\n", pages->current_page);
    
    if(image_container_list[pages->current_page] != NULL) {
      if(pages->isSingle) {

        if(image_container_list[pages->current_page]->dst != NULL) {
          g_object_unref(G_OBJECT(image_container_list[pages->current_page]->dst));
        }

      } else {

        if(image_container_list[pages->current_page]->dst != NULL) {
          g_object_unref(G_OBJECT(image_container_list[pages->current_page]->dst));
        }

        if((pages->current_page - 1) > -1 && image_container_list[pages->current_page - 1] != NULL && image_container_list[pages->current_page - 1]->dst != NULL) {
          g_object_unref(G_OBJECT(image_container_list[pages->current_page - 1]->dst));
        }
      }
    }
  }
}

void next_image(int isForward)
{
  if(pages->isSingle) {
    unref_dst();

    set_image_container(pages->current_page);

    gtk_image_clear((GtkImage*)pages->left);

    gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page]->dst);    

  } else {

    update_page(FALSE);

  }

}

gboolean my_key_press_function(GtkWidget *widget, GdkEventKey *event, gpointer data)
{

  GdkModifierType consumed;
  const int ALL_ACCESS_MASK = GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK;

  GdkKeymap *keymap = gdk_keymap_get_for_display(gdk_display_get_default());
  guint keyval;
  gdk_keymap_translate_keyboard_state(keymap, event->hardware_keycode, event->state, event->group, &keyval, NULL, NULL, &consumed);

  int isCtrl = (event->state & ~consumed & ALL_ACCESS_MASK) == GDK_CONTROL_MASK; 
  int isAlt = (event->state & ~consumed & ALL_ACCESS_MASK) == GDK_MOD1_MASK; 
  int isShift = (event->state & ~consumed & ALL_ACCESS_MASK) == GDK_SHIFT_MASK;

  if(keyval == GDK_KEY_o && isCtrl) {
    open_file_on_menu();

    return TRUE;
  }

  if(keyval == GDK_KEY_q && isCtrl) {

    CloseWindow();

    return TRUE;
  }

  if(image_container_list == NULL) {
    return FALSE;
  }

  if((keyval == GDK_KEY_Home) || (keyval == GDK_KEY_a && isCtrl) || (keyval == GDK_KEY_0)) {
    
    move_top_page();
    
    return TRUE;
  }

  if((keyval == GDK_KEY_End) || (keyval == GDK_KEY_e && isCtrl) || (keyval == GDK_KEY_dollar && isShift)) {

    move_end_page();
    
    return TRUE;
  }

  if(keyval == GDK_KEY_c) {
    change_covermode();
    return TRUE;
  }

  if ((keyval == GDK_KEY_Escape || (keyval == GDK_KEY_Return && isAlt))
      && window.isFullScreen) {
    cancel_fullscreen();
    return TRUE;
  }

  if(keyval == GDK_KEY_Return && isAlt) {
    fullscreen();

    return TRUE;
  }

  if(keyval == GDK_KEY_s && isCtrl) {
    change_spread_to_single();

    return TRUE;
  }

  if(keyval == GDK_KEY_d && isCtrl) {
    change_single_to_spread();

    return TRUE;
  }


  if((keyval == GDK_KEY_f && isCtrl) || event->keyval == GDK_KEY_Right || event->keyval == GDK_KEY_l) {
      move_right();

    return TRUE;
  }

  if((keyval == GDK_KEY_b && isCtrl) || keyval == GDK_KEY_Left || keyval == GDK_KEY_h) {

      move_left();

    return TRUE;

  }

  return FALSE;
}

gboolean my_detect_click_function(GtkWidget *widget, GdkEventButton *event, gpointer data)
{

  guint x = (guint)event->x;
  guint y = (guint)event->y;

  printf("coordinate x: %d, y: %d, half: %d\n", x, y, (window.width / 2));

  if(event->type == GDK_BUTTON_PRESS) {
    
    if (x < (window.width) / 2) {
      move_left();
    } else {
      move_right();
    }

  }
  

}


void free_image_container()
{
  if(image_container_list != NULL) {
    for(int i = 0; i < detail->image_count; i++) {
      if(image_container_list[i] != NULL) {

        if(image_container_list[i]->aspect_raito != NULL) {
          free(image_container_list[i]->aspect_raito);
          image_container_list[i]->aspect_raito = NULL;
        }

        if(image_container_list[i]->src != NULL) {
          g_object_unref(G_OBJECT(image_container_list[i]->src));
        }

        if(image_container_list[i]->dst != NULL) {
          g_object_unref(image_container_list[i]->dst);
        }

        free(image_container_list[i]);
        image_container_list[i] = NULL;
      }
    }

    free(image_container_list);
    image_container_list = NULL;
  }

}

void close_variables()
{
  if(!window.isClose) {
    if(detail != NULL && detail->image_path_list != NULL) 
    {                                                                                
      free_array_with_alloced((void**)detail->image_path_list, detail->image_count);    
    }                                                                                

    free_image_container();

    free_uncompress_data_set(uncompressed_file_list);

    if(detail != NULL) {
      free(detail);
      detail = NULL;
    }

    if(pages != NULL) {
      free(pages);
      pages = NULL;
    }

    fz_clear();

    if(window.isFullScreen)
      pthread_detach(thread_of_curosr_observer);

    window.isClose = TRUE;
  }
}


void set_image_container(ulong position)
{
  if(image_container_list[position] == NULL) {
    image_container_list[position] = malloc(sizeof(Image_Container_t));
    memset(image_container_list[position], 0, sizeof(Image_Container_t));

    image_container_list[position]->err = NULL;

    if(!isCompressFile && !isPDFfile) {

      if(detail->image_path_list[position] != NULL) {
        image_container_list[position]->src = gdk_pixbuf_new_from_file(detail->image_path_list[position], &image_container_list[position]->err);
      }

    } else if(isPDFfile && !isCompressFile) {

      fz_pixmap *tmp_fz_pixmap = get_pdf_data_from_page(position);

      if(tmp_fz_pixmap == NULL) {
        return;
      }


      GdkPixbuf *tmp_gdk_pixbuf = gdk_pixbuf_new_from_data(tmp_fz_pixmap->samples,
                                                           GDK_COLORSPACE_RGB, FALSE, 8, tmp_fz_pixmap->w, tmp_fz_pixmap->h, tmp_fz_pixmap->stride,
                                                           NULL, NULL);

      image_container_list[position]->src = tmp_gdk_pixbuf;



    } else {

      GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
      gboolean check = gdk_pixbuf_loader_write(loader, uncompressed_file_list->uncompress_data_list[position]->data, uncompressed_file_list->uncompress_data_list[position]->file_size, NULL);
      if(!check) {
        printf("GdkPixbufLoader write error\n");
      }

      free(uncompressed_file_list->uncompress_data_list[position]->data);
      uncompressed_file_list->uncompress_data_list[position]->data = NULL;

      free(uncompressed_file_list->uncompress_data_list[position]->file_name);
      uncompressed_file_list->uncompress_data_list[position]->file_name = NULL;

      free(uncompressed_file_list->uncompress_data_list[position]);
      uncompressed_file_list->uncompress_data_list[position] = NULL;


      image_container_list[position]->src = gdk_pixbuf_loader_get_pixbuf(loader);

      GError *err;
      if(loader != NULL) {
        gdk_pixbuf_loader_close(loader, NULL);
      }

    }

    image_container_list[position]->src_width = gdk_pixbuf_get_width(image_container_list[position]->src);
    image_container_list[position]->src_height = gdk_pixbuf_get_height(image_container_list[position]->src);

    int width = image_container_list[position]->src_width;
    int height = image_container_list[position]->src_height;

    image_container_list[position]->aspect_raito = calc_aspect_raito(width, height, mygcd(width, height));

  }

  if(pages->isSingle)
  {
    resize_when_single(position);
  }
}

void resize_when_single(int position)
{
  gint window_width = 0;
  gint window_height = 0;
  gtk_window_get_size((GtkWindow*)window.window, &window_width, &window_height);
  window.width = window_width;
  window.height = window_height;

  int width = image_container_list[position]->src_width;
  int height = image_container_list[position]->src_height;

  double w_aspect = (double)image_container_list[position]->aspect_raito[0];
  double h_aspect = (double)image_container_list[position]->aspect_raito[1];

  int diff_height_between_windown_and_menubar_height = window_height - window.menubar_height;
  if(window.isFullScreen) {
    diff_height_between_windown_and_menubar_height = window_height;
  }

  if(height > diff_height_between_windown_and_menubar_height) {
    int diff = height - diff_height_between_windown_and_menubar_height;
    height = height - diff;
    int result = (int)ceil((double)height * (w_aspect / h_aspect));
    width = result;
  }


  image_container_list[position]->dst = gdk_pixbuf_scale_simple(image_container_list[position]->src, width, height, GDK_INTERP_BILINEAR);
  image_container_list[position]->dst_width = width;
  image_container_list[position]->dst_height = height;
}

gboolean detect_resize_window(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  if(!isFirstLoad) {
    gint width = 0;
    gint height = 0;
    gtk_window_get_size((GtkWindow*)window.window, &width, &height);

    if(width != window.width || height != window.height) {
      if(pages->isSingle) {

        unref_dst();

        resize_when_single(pages->current_page);

        gtk_image_clear((GtkImage*)pages->left);

        gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page]->dst);


      } else {
        if(detail->isOdd && ((pages->isPriorityToFrontCover && pages->current_page == 0) || (!pages->isPriorityToFrontCover && detail->image_count == pages->current_page - 1)) ) {

          unref_dst();

          resize_when_single(pages->current_page);

          gtk_image_clear((GtkImage*)pages->left);
          gtk_image_clear((GtkImage*)pages->right);

          gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page]->dst);

        } else {

          unref_dst();

          int isOverHeight = resize_when_spread(pages->current_page);

          gtk_image_clear((GtkImage*)pages->left);
          gtk_image_clear((GtkImage*)pages->right);

          if(pages->page_direction_right) {
            gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page]->dst);
            gtk_image_set_from_pixbuf((GtkImage*)pages->right, image_container_list[pages->current_page - 1]->dst);
          } else {
            gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page - 1]->dst);
            gtk_image_set_from_pixbuf((GtkImage*)pages->right, image_container_list[pages->current_page]->dst);
          }

          set_margin_left_page(pages->current_page, isOverHeight, FALSE);


        }
      }

      return TRUE;
    }
  }


  return FALSE;
}

void set_image(GtkWidget **img, int position)
{
  *img = gtk_image_new_from_pixbuf(image_container_list[position]->dst);
}

void scale_when_oversize(int *x, int *y, int window_width, int window_height, double w_aspect, double h_aspect, int isOverWidth)
{
  if(isOverWidth)
  {
    int diff = *x - window_width;
    int width = *x - diff;
    int height = (int)ceil((double)width * (h_aspect / w_aspect));

    *x = width;
    *y = height;
  }
  else
  {
    int diff = *y - window_height;
    int height = *y - diff;
    int width = (int)ceil((double)height * (w_aspect / h_aspect));

    *x = width;
    *y = height;
  }


}

int resize_when_spread(int page)
{
  int left_src_width = image_container_list[page - 1]->src_width;
  int left_src_height = image_container_list[page - 1]->src_height;
  double left_x_aspect = (double)image_container_list[page - 1]->aspect_raito[0];
  double left_y_aspect = (double)image_container_list[page - 1]->aspect_raito[1];

  int right_src_width = image_container_list[page]->src_width;
  int right_src_height = image_container_list[page]->src_height;
  double right_x_aspect = (double)image_container_list[page]->aspect_raito[0];
  double right_y_aspect = (double)image_container_list[page]->aspect_raito[1];

  gint window_width = 0;
  gint window_height = 0;
  gtk_window_get_size((GtkWindow*)window.window, &window_width, &window_height);
  window.width = window_width;
  window.height = window_height;


  int diff_height_between_windown_and_menubar = window_height - window.menubar_height;
  if(window.isFullScreen) {
    diff_height_between_windown_and_menubar = window_height;
  }

  int half_width = window_width / 2;
  int left_width = half_width;
  int left_height = 0;
  left_height = (int)ceil((double)left_width * (left_y_aspect / left_x_aspect));
  int isOverHeight = FALSE;
  if(left_height > diff_height_between_windown_and_menubar) {
    scale_when_oversize(&left_width, &left_height, window_width, diff_height_between_windown_and_menubar, left_x_aspect, left_y_aspect, FALSE);
    isOverHeight = TRUE;
  }

  image_container_list[page - 1]->dst_width = left_width;
  image_container_list[page - 1]->dst_height = left_height;

  int right_width = half_width;
  int right_height = 0;
  right_height = (int)ceil((double)right_width * (right_y_aspect / right_x_aspect));

  if(right_height > diff_height_between_windown_and_menubar) {
    scale_when_oversize(&right_width, &right_height, window_width, diff_height_between_windown_and_menubar, right_x_aspect, right_y_aspect, FALSE);
  }

  image_container_list[page]->dst_width = right_width;
  image_container_list[page]->dst_height = right_height;

  image_container_list[page - 1]->dst = gdk_pixbuf_scale_simple(image_container_list[page - 1]->src, left_width, left_height, GDK_INTERP_BILINEAR);
  image_container_list[page]->dst = gdk_pixbuf_scale_simple(image_container_list[page]->src, right_width, right_height, GDK_INTERP_BILINEAR);

  return isOverHeight;
}

void set_margin_left_page(int position, int isOverHeight, int isFinalPage)
{
  if(isOverHeight) {
    int mix_width = (image_container_list[position - 1]->dst_width + image_container_list[position]->dst_width);
    int margin_left = (fmax(mix_width, window.width) - fmin(mix_width, window.width)) / 2;

    if(isFinalPage) {
      gtk_widget_set_margin_start(pages->right, image_container_list[position]->dst_width);
    } else {
      gtk_widget_set_margin_start(pages->left, (gint)margin_left);
    }
  } else {

    if(isFinalPage) {
      gtk_widget_set_margin_start(pages->right, 0);
    } else {
      gtk_widget_set_margin_start(pages->left, 0);
    }
  }

}

int init_image_object(const char *file_name, int startpage)
{
  pages->current_page = 0;
  // set image file path
  if(!isFirstLoad) {
    free_array_with_alloced((void**)detail->image_path_list, detail->image_count);    
    detail->image_path_list = NULL;

    free_uncompress_data_set(uncompressed_file_list);
    uncompressed_file_list = NULL;

    fz_clear();

    free_image_container();
    image_container_list = NULL;

    detail->image_count = 0;

    if(pages->left != NULL) {
      gtk_image_clear(GTK_IMAGE(pages->left));
    }

    if(pages->right != NULL) {
      gtk_image_clear(GTK_IMAGE(pages->right));
    }


  }


  if(isCompressFile) {

    if(!set_image_from_compressed_file(file_name)) {
      return FALSE;
    }

  } else if(!isCompressFile && isPDFfile) {

    if(!set_image_from_pdf_file(file_name)) {
      return FALSE;
    }

  } else {

    if(!set_image_path_list(file_name)) {
      return FALSE;
    }
  }



  if(detail->image_count > 0) {
    image_container_list = (Image_Container_t**)calloc(detail->image_count, sizeof(Image_Container_t*));

    if(pages->isSingle || detail->image_count == 1) {

      if(!pages->isSingle) {
        pages->isSingle = TRUE;
      }

      set_image_container(0);

      resize_when_single(0);

      set_image(&pages->left, 0);

    } else {
      set_image_container(0);

      set_image_container(1);
      resize_when_spread(1);


      if(pages->page_direction_right) {
        set_image(&pages->right, 0);

        set_image(&pages->left, 1);

      } else {
        set_image(&pages->left, 0);

        set_image(&pages->right, 1);
      }

      pages->current_page = 1;

      if(pages->isPriorityToFrontCover) {
        pages->current_page = 0;
      }

    }

    update_page(FALSE);

    return TRUE;
  }

  return FALSE;
}

void cancel_fullscreen()
{
    gtk_window_unfullscreen(GTK_WINDOW(window.window));
    show_menu();
    show_mouse();
    window.isFullScreen = FALSE;

    int error = pthread_join(thread_of_curosr_observer, NULL);
}

void fullscreen()
{
  if(window.isFullScreen) {
    cancel_fullscreen();
  } else {
    gtk_window_fullscreen(GTK_WINDOW(window.window));
    
    hide_menu();
    
    hide_mouse();
    
    window.isFullScreen = TRUE;

    int error = pthread_create(&thread_of_curosr_observer, NULL, cursor_observer_in_fullscreen_mode, NULL);

    
  }

}

void update_page(int isSingleChange)
{
  if(!isFirstLoad) {

    if(isSingleChange) {

      if(pages->isSingle) {

        if(detail->isOdd && pages->current_page == (detail->image_count - 1)) {

          gtk_image_clear(GTK_IMAGE(pages->right));

        } else {

          if(pages->left != NULL) {
            gtk_image_clear(GTK_IMAGE(pages->left));
          }

          if(pages->right != NULL) {
            gtk_image_clear(GTK_IMAGE(pages->right));
          }

        }

        pages->current_page -= 1;

        if(pages->current_page < 0) {
          pages->current_page = 0;
        }

        set_image_container(pages->current_page);

        resize_when_single(pages->current_page);

        set_image(&pages->left, pages->current_page);

        update_grid();

      } else {

        gtk_image_clear(GTK_IMAGE(pages->left));

        pages->current_page++;

        int latest_page = detail->image_count - 1;

        if(pages->current_page >= latest_page) {
          pages->current_page = latest_page - 1;
        }

        set_image_container(pages->current_page);
        set_image_container(pages->current_page - 1);


        int isOverHeight = resize_when_spread(pages->current_page);
        set_margin_left_page(pages->current_page, isOverHeight, FALSE);

        set_image(&pages->right, pages->current_page - 1);
        set_image(&pages->left, pages->current_page);

        update_grid();
      }

    } else {

      set_image_container(pages->current_page);
      set_image_container(pages->current_page - 1);


      if(pages->right != NULL) {
        gtk_image_clear(GTK_IMAGE(pages->right));
      }

      if(pages->left != NULL) {
        gtk_image_clear(GTK_IMAGE(pages->left));
      }

      if(pages->isPriorityToFrontCover) {
        if(pages->isCover && pages->current_page > 0) {
          if(pages->page_direction_right) {
            gtk_widget_show(pages->left);
          } else {
            gtk_widget_show(pages->right);
          }
          pages->isCover = FALSE;

        } else if(pages->current_page == 0){
          pages->isCover = TRUE;

        }

      }


      if(detail->isOdd && ((pages->isPriorityToFrontCover && pages->current_page == 0) || (!pages->isPriorityToFrontCover && pages->current_page == detail->image_count - 1)) ) {
        unref_dst();

        int isOverHeight = resize_when_spread(pages->current_page);

        if(pages->page_direction_right) {

          gtk_image_set_from_pixbuf(GTK_IMAGE(pages->right), image_container_list[pages->current_page]->dst);

        } else {

          gtk_image_set_from_pixbuf(GTK_IMAGE(pages->left), image_container_list[pages->current_page]->dst);
        }

      } else {

        unref_dst();

        int isOverHeight = FALSE;
        if(pages->isCover) {
          resize_when_single(0);
        } else {
          isOverHeight = resize_when_spread(pages->current_page);
        }

        if(pages->page_direction_right) {

          if(!pages->isCover) {
            gtk_image_set_from_pixbuf(GTK_IMAGE(pages->right), image_container_list[pages->current_page - 1]->dst);
          }

          gtk_image_set_from_pixbuf(GTK_IMAGE(pages->left), image_container_list[pages->current_page]->dst);

        } else {

          if(!pages->isCover) {
            gtk_image_set_from_pixbuf(GTK_IMAGE(pages->left), image_container_list[pages->current_page - 1]->dst);
          }

          gtk_image_set_from_pixbuf(GTK_IMAGE(pages->right), image_container_list[pages->current_page]->dst);
        }


        if(!pages->isCover) {
          set_margin_left_page(pages->current_page, isOverHeight, FALSE);
        }
      }

    }
  }
}

void update_grid()
{
  if(pages->isSingle) {
    if(!isFirstLoad) {
      gtk_grid_remove_column(GTK_GRID(grid), 0);
    }

    gtk_widget_set_vexpand(pages->left, TRUE);
    gtk_widget_set_hexpand(pages->left, TRUE);
    gtk_grid_attach(GTK_GRID(grid), pages->left, 0, 0, 1, 1);

    gtk_widget_show(pages->left);

  } else {


    if(!isFirstLoad) {
      gtk_grid_remove_column(GTK_GRID(grid), 0);
      gtk_grid_remove_column(GTK_GRID(grid), 0);
    }

    gtk_widget_set_vexpand(pages->left, TRUE);

    gtk_widget_set_vexpand(pages->right, TRUE);

    gtk_grid_attach(GTK_GRID(grid), pages->left, 0, 0, 1, 1);

    gtk_grid_attach_next_to(GTK_GRID(grid), pages->right, pages->left, GTK_POS_RIGHT, 1, 1);

    gtk_widget_show(pages->left);
    gtk_widget_show(pages->right);

    if(pages->isPriorityToFrontCover && pages->current_page == 0) {
      gtk_widget_hide(pages->left);
    }



  }

  if(isFirstLoad) {
    isFirstLoad = FALSE;
  }
}

GtkWidget *create_menu_bar()
{

  // settings menubar
  GtkWidget *menubar = gtk_menu_bar_new();

  // File Menu
  file_menu_struct.body = gtk_menu_new();
  file_menu_struct.root = gtk_menu_item_new_with_label("File");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_struct.root), file_menu_struct.body);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file_menu_struct.root);

  file_menu_struct.load = gtk_menu_item_new_with_label("Load");
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu_struct.body), file_menu_struct.load);
  g_signal_connect(G_OBJECT(file_menu_struct.load), "activate", G_CALLBACK(open_file_on_menu), NULL);

  file_menu_struct.quit = gtk_menu_item_new_with_label("Quit");
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu_struct.body), file_menu_struct.quit);
  g_signal_connect(G_OBJECT(file_menu_struct.quit), "activate", G_CALLBACK(CloseWindow), NULL);

  // View Menu
  view_menu_struct.body = gtk_menu_new();
  view_menu_struct.root = gtk_menu_item_new_with_label("View");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_menu_struct.root), view_menu_struct.body);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), view_menu_struct.root);

  view_menu_struct.page_direction = gtk_menu_item_new_with_label("Change Page Dpirection");
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu_struct.body), view_menu_struct.page_direction);
  g_signal_connect(G_OBJECT(view_menu_struct.page_direction), "activate", G_CALLBACK(change_direction), NULL);


  view_menu_struct.set_single_mode = gtk_menu_item_new_with_label("Set Single Mode");
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu_struct.body), view_menu_struct.set_single_mode);
  g_signal_connect(G_OBJECT(view_menu_struct.set_single_mode), "activate", G_CALLBACK(change_spread_to_single), NULL);

  view_menu_struct.set_spread_mode = gtk_menu_item_new_with_label("Set Spread Mode");
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu_struct.body), view_menu_struct.set_spread_mode);
  g_signal_connect(G_OBJECT(view_menu_struct.set_spread_mode), "activate", G_CALLBACK(change_single_to_spread), NULL);

  view_menu_struct.set_covermode = gtk_menu_item_new_with_label("Set Cover Mode");
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu_struct.body), view_menu_struct.set_covermode);
  g_signal_connect(G_OBJECT(view_menu_struct.set_covermode), "activate", G_CALLBACK(change_covermode), NULL);

  // Help Menu
  help_menu_struct.body = gtk_menu_new();
  help_menu_struct.root = gtk_menu_item_new_with_label("Help");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_menu_struct.root), help_menu_struct.body);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help_menu_struct.root);

  help_menu_struct.about = gtk_menu_item_new_with_label("About");
  gtk_menu_shell_append(GTK_MENU_SHELL(help_menu_struct.body), help_menu_struct.about);

  return menubar;
}


void move_left()
{
  if (pages->current_page > -1) {

    if(pages->isSingle) {
      int tmp = pages->current_page + 1;

      if(tmp < detail->image_count)
        pages->current_page = tmp;

      if(tmp >= detail->image_count && pages->isAcceptOverflow) {
        pages->current_page = 0;
      }

    } else if(pages->page_direction_right) {

      int tmp = pages->current_page + 2;

      if(tmp < detail->image_count)
        pages->current_page = tmp;

      if(tmp >= detail->image_count) {
        if(tmp != 0 && tmp % 2 != 0) {
          pages->current_page = tmp - 1;
        } 

      }

      if(pages->isAcceptOverflow && pages->current_page >= detail->image_count) {
        pages->current_page = 1;

        if(pages->isPriorityToFrontCover) {
          pages->current_page = 0;
        }
      }

    } else {

        int tmp = pages->current_page - 2;
        if(tmp < 1) {
          tmp = 1;
        }

        pages->current_page = tmp;

        if(pages->isAcceptOverflow)
          pages->current_page = detail->image_count - 1;

    }

    if(pages->isPriorityToFrontCover && pages->current_page == 0) {
      pages->isCover = TRUE;
    }

    if(!pages->isSingle && !pages->isPriorityToFrontCover && pages->current_page != 0 && pages->current_page % 2 == 0) {
      pages->current_page++;
    }

    if(pages->current_page == detail->image_count) {
      pages->current_page--;
    }

    if(pages->current_page > detail->image_count) {
      pages->current_page = detail->image_count - 1;
    }

    if(pages->current_page < 0 && pages->isSingle) {
      pages->current_page = detail->image_count - 1;
    }


    if(pages->page_direction_right) {
      next_image(TRUE);
    } else {
      next_image(FALSE);
    }
  }

}


void move_right()
{
  if (pages->current_page > -1) {

    if(pages->isSingle) {

      int tmp = pages->current_page - 1;
      if(tmp >= 0)
        pages->current_page = tmp;

      if(tmp < 0 && pages->isAcceptOverflow)
        pages->current_page = detail->image_count - 1;


    } else if(pages->page_direction_right) {

      int tmp = pages->current_page - 2;

      if(tmp < 0) {

        if(pages->isAcceptOverflow)
          pages->current_page = detail->image_count - 1;

      } else {
        pages->current_page = tmp;
      }

    } else {

      int tmp = pages->current_page + 2;
      if(tmp < detail->image_count) {
        pages->current_page = tmp;
      }

      if(pages->current_page >= detail->image_count) {
        pages->current_page--;
      }

      if(pages->current_page > detail->image_count && pages->isAcceptOverflow) {
        pages->current_page = 1;
      }
    }

    if(pages->current_page >= detail->image_count) {
      if(pages->isSingle) {

        pages->current_page = 0;

      } else {

        if(pages->isPriorityToFrontCover) {

          pages->current_page = 0;
          pages->isCover = TRUE;

        } else {

          pages->current_page = 1;
        }
      }
    }

    if(pages->isPriorityToFrontCover && pages->current_page == 0) {
      pages->isCover = TRUE;
    }

    if(!pages->isSingle && pages->current_page != 0 && !pages->isPriorityToFrontCover && pages->current_page % 2 == 0 && pages->current_page != detail->image_count - 1) {
      pages->current_page++;
    }

    if(pages->page_direction_right) {
      next_image(FALSE);
    } else {
      next_image(TRUE);
    }

  }
}

void move_top_page()
{
  if (pages->current_page > -1) {

    pages->current_page = 1;

    if(pages->isSingle) {
      pages->current_page = 0;
    }

    if(pages->page_direction_right) {
      next_image(FALSE);
    } else {
      next_image(TRUE);
    }
  }
}

void move_end_page()
{
  if (pages->current_page > -1) {

    pages->current_page = detail->image_count - 1;
  
    if(pages->page_direction_right) {
      next_image(FALSE);
    } else {
      next_image(TRUE);
    }

  }
}

void show_mouse()
{
  GdkDisplay *display = gdk_display_get_default();
  GdkWindow *gdk_window = gtk_widget_get_window(window.window);
  GdkCursor *cursor;

  cursor = gdk_cursor_new_from_name(display, "default");

  gdk_window_set_cursor(gdk_window, cursor);

  g_object_unref(cursor);
  
}

void hide_mouse()
{
  GdkDisplay *display = gdk_display_get_default();
  GdkWindow *gdk_window = gtk_widget_get_window(window.window);
  GdkCursor *cursor;

  /* if(window.isFullScreen) { */
  /*   cursor = gdk_cursor_new_from_name(display, "default"); */
  /* } else { */
  /*   cursor = gdk_cursor_new_for_display(display, GDK_BLANK_CURSOR); */
  /* } */

  cursor = gdk_cursor_new_for_display(display, GDK_BLANK_CURSOR);
  
  gdk_window_set_cursor(gdk_window, cursor);

  g_object_unref(cursor);

}

void *cursor_observer_in_fullscreen_mode(void *data)
{
  if(!window.isFullScreen)
    return NULL;

  time_t t = time(NULL);
  guint x = cursor_pos.x;
  guint y = cursor_pos.y;

  while (TRUE) {

    if(!window.isFullScreen)
      return NULL;
    
    time_t current = time(NULL);

    if (cursor_pos.x == x && cursor_pos.y == y && cursor_pos.y > (window.button_menu_height +  window.menubar_height)) {
      if (current > t) {
        hide_mouse();
      }
    } else if (cursor_pos.x != x || cursor_pos.y != y) {
      show_mouse();
    }

    t = current;
    x = cursor_pos.x;
    y = cursor_pos.y;


  }
  
}

void show_menu()
{
  gtk_widget_show(window.menubar);
  gtk_widget_show(button_menu);  
}

void hide_menu()
{
  gtk_widget_hide(window.menubar);
  gtk_widget_hide(button_menu);
}
