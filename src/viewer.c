#include "viewer.h"

const char *db_name = "simple_comics_viewer.db";
const ssize_t db_name_size = 23;

const char *app_dir = "simple_comics_viewer";
const ssize_t app_dir_size = 20;

char *db_path_under_dot_local_share = NULL;
ssize_t db_path_under_dot_local_share_size = 0;

char *temporary_db_path = NULL;
ssize_t temporary_db_path_size = 0;

int status = 0;

char *arg_file_name = NULL;

pthread_t thread_of_curosr_observer;

Cursor_Position_t cursor_pos = {};

Image_button_t image_button = {};

DrawingArea_t draw_area = {};

GtkWidget *grid = NULL;

GtkApplication *app = NULL;

main_window_data_t window = {};

file_menu_t file_menu_struct = {};

file_history_on_menu_t file_history_on_menu_struct = {};

view_menu_t view_menu_struct = {};

help_menu_t help_menu_struct = {};

GtkWidget *button_menu = NULL;

comic_container_t *comic_container = NULL;

db_s db_info = {};

GtkWidget *file_history_internal_list = NULL;

const char *right_to_left_name = "Current Direction: Right to Left";
const char *left_to_right_name = "Current Direction: Left to Right";

GtkWidget *change_direction_button = NULL;

file_history_s *history = NULL;
int is_file_history_none = FALSE;

void set_file_history_on_menu()
{


  if(history != NULL)
    free_history_array(history);


  int is_reload = FALSE;
  if(file_history_internal_list != NULL) {

    GList *child_list = gtk_container_get_children(GTK_CONTAINER(file_history_internal_list));
    for(GList *l = child_list; l != NULL; l = l->next) {

      gpointer element = l->data;
      gtk_container_remove(GTK_CONTAINER(file_history_internal_list), element);
      
    }

    is_reload = TRUE;
  }


  if(file_history_internal_list == NULL) {
    file_history_internal_list = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_struct.file_history), file_history_internal_list);
  }


  history = (file_history_s*)malloc(sizeof(file_history_s));
  if(history != NULL) {
    int check = get_file_history(&db_info, history);
    if(check) {
      if(history->size == 0) {
        goto none;
      }

      if(is_file_history_none) {
        gtk_widget_set_sensitive(file_menu_struct.file_history, TRUE);
        is_file_history_none = FALSE;
      }
      
      for(ssize_t i = 0; i < history->size; ++i) {
        /* printf("%s\n", history->file_path_name_list[i]->data); */
          
        GtkWidget *widget = gtk_menu_item_new_with_label(history->file_path_name_list[i]->data);

        gtk_menu_shell_append(GTK_MENU_SHELL(file_history_internal_list), widget);
        g_signal_connect(G_OBJECT(widget), "activate", G_CALLBACK(open_file_in_file_history), (gpointer)i);

        if(is_reload)
          gtk_widget_show(widget);
      }

      return;
    }
  }

  
  GtkWidget *none;
  goto none;

 none:
  gtk_widget_set_sensitive(file_menu_struct.file_history, FALSE);
  is_file_history_none = TRUE;

  if(history != NULL) {
    free(history);
    history = NULL;
  }

  return;  

}

int inline check_valid_cover_mode()
{
  if(comic_container->pages == NULL || comic_container->detail == NULL)
    return FALSE;
  
  return !comic_container->pages->isSingle && comic_container->isCoverMode && comic_container->pages->current_page == 0;
}

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

  if(comic_container->detail == NULL)
    comic_container->detail = (DirectoryDetail_t*)calloc(sizeof(DirectoryDetail_t), sizeof(DirectoryDetail_t));

  comic_container->detail->image_count = get_pdf_page_size();

  if(comic_container->detail->image_count % 2) {
    comic_container->detail->isOdd = TRUE;
  } else {
    comic_container->detail->isOdd = FALSE;
  }


  return TRUE;
}

int set_image_from_compressed_file(const char *file_name)
{
  comic_container->uncompressed_file_list = (uncompress_data_set_t*)calloc(sizeof(uncompress_data_set_t), sizeof(uncompress_data_set_t));
  int ret = load_from_compress_file(file_name, comic_container->uncompressed_file_list);
  
  if(!ret) {
    comic_container->uncompressed_file_list = NULL;
    return FALSE;
  }


  if(comic_container->detail == NULL) {
    comic_container->detail = (DirectoryDetail_t*)calloc(sizeof(DirectoryDetail_t), sizeof(DirectoryDetail_t));
  }

  comic_container->detail->image_count = comic_container->uncompressed_file_list->size;

  if(comic_container->detail->image_count % 2) {
    comic_container->detail->isOdd = TRUE;

  } else {
    comic_container->detail->isOdd = FALSE;
  }

  return TRUE;
}

int get_file_count_and_set_image_path_list(struct dirent **src, const int size, char **dst_image_path_list, const char *dirname)
{

  char **image_path_list = (char**)calloc(LIST_BUFFER, 1);

  ssize_t dirname_length = strlen(dirname);

  int count = 0;
  for(int i = 0; i < size; ++i) {

    if(count < LIST_BUFFER) {
      if(strcmp(src[i]->d_name, ".") != 0 && strcmp(src[i]->d_name, "..") != 0) {
        const ssize_t src_length = strlen(src[i]->d_name);
        const ssize_t final_path_size = dirname_length + src_length + 2;
        char *final_path = (char*)calloc(final_path_size, 1);
        ssize_t memmove_pos = 0;
        if(final_path == NULL) {
          puts("failed allocate final_path");
          break;
        }

        memmove(final_path, dirname, dirname_length);
        memmove_pos += dirname_length;
        memmove(final_path + memmove_pos, "/", 1);
        ++memmove_pos;
        memmove(final_path + memmove_pos, src[i]->d_name, src_length);
        final_path[final_path_size - 1] = '\0';

        int check = detect_image_from_file(final_path);
        if(check) {
          ++count;

          image_path_list[count - 1] = (char*)calloc(final_path_size, 1);
          memmove(image_path_list[count - 1], final_path, final_path_size);
        }

        free(final_path);
        final_path = NULL;
      }
    }

  }

  memmove(dst_image_path_list, image_path_list, LIST_BUFFER);

  return count;
}

int create_image_path_list(char **image_path_list, const char *dirname)
{
  struct dirent **file_list;

  int r = scandir(dirname, &file_list, NULL, alphasort);
  if(r < 1) {

    if(file_list != NULL) {
      free(file_list);
      file_list = NULL;
    }

    return 0;
  }

  int *number_list = (int*)calloc(sizeof(int) * LIST_BUFFER, sizeof(int));
  int count = get_file_count_and_set_image_path_list(file_list, r, image_path_list, dirname);

  if(count < 1) {
    free(number_list);
    number_list = NULL;

    if(file_list != NULL) {
      free(file_list);
      file_list = NULL;
    }
    
    free_array_with_alloced((void**)file_list, r);

    return 0;
  }

  free(number_list);
  number_list = NULL;

  free_array_with_alloced((void**)file_list, r);


  return count;
}

int set_image_path_list(const char *dirname)
{
  if(comic_container->detail == NULL) {
    comic_container->detail = (DirectoryDetail_t*)malloc(sizeof(DirectoryDetail_t));

    if(comic_container->detail != NULL) {
      comic_container->detail->image_count = 0;
      comic_container->detail->image_path_list = NULL;
      comic_container->detail->isOdd = 0;
    } else {
      return FALSE;
    }
  }


  if(comic_container->detail->image_path_list == NULL) {
    comic_container->detail->image_path_list = (char**)calloc(LIST_BUFFER + 1, 1);

    if(comic_container->detail->image_path_list == NULL) {
      free_array_with_alloced((void**)comic_container->detail->image_path_list, LIST_BUFFER);
      return FALSE;
    }
  }

  int count = create_image_path_list(comic_container->detail->image_path_list, dirname);
  if(count < 1) {
    free(comic_container->detail->image_path_list);
    comic_container->detail->image_path_list = NULL;

    return FALSE;
  }

  if(count == 1) {
    comic_container->pages->isSingle = TRUE;
  } else {
    comic_container->pages->isSingle = FALSE;
  }

  comic_container->detail->image_count = count;

  if(comic_container->detail->image_count % 2) {
    comic_container->detail->isOdd = TRUE;
  } else {
    comic_container->detail->isOdd = FALSE;
  }

  return TRUE;
}

void unref_dst()
{
  if(comic_container->pages != NULL && comic_container->image_container_list != NULL) {
    
    if(comic_container->image_container_list[comic_container->pages->current_page] != NULL) {
      if(comic_container->pages->isSingle) {

        if(comic_container->image_container_list[comic_container->pages->current_page]->dst != NULL) {
          g_object_unref(G_OBJECT(comic_container->image_container_list[comic_container->pages->current_page]->dst));
        }

      } else {

        if(comic_container->image_container_list[comic_container->pages->current_page]->dst != NULL) {
          g_object_unref(G_OBJECT(comic_container->image_container_list[comic_container->pages->current_page]->dst));
        }

        if((comic_container->pages->current_page - 1) > -1 && comic_container->image_container_list[comic_container->pages->current_page - 1] != NULL && comic_container->image_container_list[comic_container->pages->current_page - 1]->dst != NULL) {
          g_object_unref(G_OBJECT(comic_container->image_container_list[comic_container->pages->current_page - 1]->dst));
        }
      }
    }
  }
}

void next_image(int isForward)
{
  if(comic_container->pages->isSingle) {
    unref_dst();

    set_image_container(comic_container->pages->current_page);

    gtk_image_clear((GtkImage*)comic_container->pages->left);

    resize_when_single(comic_container->pages->current_page);

    gtk_image_set_from_pixbuf((GtkImage*)comic_container->pages->left, comic_container->image_container_list[comic_container->pages->current_page]->dst);    

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

  if(comic_container->image_container_list == NULL) {
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

  // printf("coordinate x: %d, y: %d, half: %d\n", x, y, (window.width / 2));

  if(event->type == GDK_BUTTON_PRESS) {
    
    if (x < (window.width) / 2) {
      move_left();
    } else {
      move_right();
    }

  }

  return TRUE;
}


void free_image_container()
{
  if(comic_container->image_container_list != NULL) {
    for(int i = 0; i < comic_container->detail->image_count; i++) {
      if(comic_container->image_container_list[i] != NULL) {

        if(comic_container->image_container_list[i]->aspect_raito != NULL) {
          free(comic_container->image_container_list[i]->aspect_raito);
          comic_container->image_container_list[i]->aspect_raito = NULL;
        }

        if(comic_container->image_container_list[i]->src != NULL) {
          g_object_unref(G_OBJECT(comic_container->image_container_list[i]->src));
        }

        if(comic_container->image_container_list[i]->dst != NULL) {
          g_object_unref(comic_container->image_container_list[i]->dst);
        }

        free(comic_container->image_container_list[i]);
        comic_container->image_container_list[i] = NULL;
      }
    }

    free(comic_container->image_container_list);
    comic_container->image_container_list = NULL;
  }

}

void close_variables()
{
  if(!window.isClose) {
    
    if(history != NULL) {
      free_history_array(history);
    }

    /* if(file_history_on_menu_struct.list != NULL) { */

    /*   free(file_history_on_menu_struct.list); */
    /*   file_history_on_menu_struct.list = NULL; */
    /* } */

    
    if(comic_container->detail != NULL && comic_container->detail->image_path_list != NULL) 
    {                                                                                
      free_array_with_alloced((void**)comic_container->detail->image_path_list, comic_container->detail->image_count);    
    }                                                                                

    free_image_container();

    free_uncompress_data_set(comic_container->uncompressed_file_list);

    if(comic_container->detail != NULL) {
      free(comic_container->detail);
      comic_container->detail = NULL;
    }

    if(comic_container->pages != NULL) {
      free(comic_container->pages);
      comic_container->pages = NULL;
    }

    free(comic_container);
    comic_container = NULL;

    fz_clear();

    if(db_path_under_dot_local_share != NULL && temporary_db_path != NULL) {
      backup_db();
    }


    if(db_path_under_dot_local_share != NULL) {
      free(db_path_under_dot_local_share);
      db_path_under_dot_local_share = NULL;
    }

    if(temporary_db_path != NULL) {
      free(temporary_db_path);
      temporary_db_path = NULL;
    }

    
    if(window.isFullScreen)
      pthread_detach(thread_of_curosr_observer);
    
    window.isClose = TRUE;
  }
}


void set_image_container(ulong position)
{

  if(comic_container->image_container_list[position] == NULL) {
    comic_container->image_container_list[position] = malloc(sizeof(Image_Container_t));
    memset(comic_container->image_container_list[position], 0, sizeof(Image_Container_t));

    comic_container->image_container_list[position]->err = NULL;

    if(!comic_container->isCompressFile && !comic_container->isPDFfile) {

      if(comic_container->detail->image_path_list[position] != NULL) {
        comic_container->image_container_list[position]->src = gdk_pixbuf_new_from_file(comic_container->detail->image_path_list[position], &comic_container->image_container_list[position]->err);
        
        comic_container->image_container_list[position]->src_width = gdk_pixbuf_get_width(comic_container->image_container_list[position]->src);
        comic_container->image_container_list[position]->src_height = gdk_pixbuf_get_height(comic_container->image_container_list[position]->src);
        int width = comic_container->image_container_list[position]->src_width;
        int height = comic_container->image_container_list[position]->src_height;

        comic_container->image_container_list[position]->aspect_raito = calc_aspect_raito(width, height, mygcd(width, height));

      }

    } else if(comic_container->isPDFfile && !comic_container->isCompressFile) {

      fz_pixmap *tmp_fz_pixmap = get_pdf_data_from_page(position);

      if(tmp_fz_pixmap == NULL) {
        return;
      }


      GdkPixbuf *tmp_gdk_pixbuf = gdk_pixbuf_new_from_data(tmp_fz_pixmap->samples,
                                                           GDK_COLORSPACE_RGB, FALSE, 8, tmp_fz_pixmap->w, tmp_fz_pixmap->h, tmp_fz_pixmap->stride,
                                                           NULL, NULL);

      comic_container->image_container_list[position]->src = tmp_gdk_pixbuf;


    } else {

      GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
      gboolean check = gdk_pixbuf_loader_write(loader, comic_container->uncompressed_file_list->uncompress_data_list[position]->data, comic_container->uncompressed_file_list->uncompress_data_list[position]->file_size, NULL);
      if(!check) {
        printf("GdkPixbufLoader write error\n");
      }

      free(comic_container->uncompressed_file_list->uncompress_data_list[position]->data);
      comic_container->uncompressed_file_list->uncompress_data_list[position]->data = NULL;

      free(comic_container->uncompressed_file_list->uncompress_data_list[position]->file_name);
      comic_container->uncompressed_file_list->uncompress_data_list[position]->file_name = NULL;

      free(comic_container->uncompressed_file_list->uncompress_data_list[position]);
      comic_container->uncompressed_file_list->uncompress_data_list[position] = NULL;


      comic_container->image_container_list[position]->src = gdk_pixbuf_loader_get_pixbuf(loader);

      GError *err;
      if(loader != NULL) {
        gdk_pixbuf_loader_close(loader, NULL);
      }

    }

    comic_container->image_container_list[position]->src_width = gdk_pixbuf_get_width(comic_container->image_container_list[position]->src);
    comic_container->image_container_list[position]->src_height = gdk_pixbuf_get_height(comic_container->image_container_list[position]->src);

    int width = comic_container->image_container_list[position]->src_width;
    int height = comic_container->image_container_list[position]->src_height;

    comic_container->image_container_list[position]->aspect_raito = calc_aspect_raito(width, height, mygcd(width, height));

  }

  /* if(comic_container->pages->isSingle) */
  /* { */
  /*   resize_when_single(position); */
  /* } */
}

int resize_when_single(int position)
{
  gint window_width = 0;
  gint window_height = 0;
  gtk_window_get_size((GtkWindow*)window.window, &window_width, &window_height);
  window.width = window_width;
  window.height = window_height;

  int width = comic_container->image_container_list[position]->src_width;
  int height = comic_container->image_container_list[position]->src_height;

  double w_aspect = (double)comic_container->image_container_list[position]->aspect_raito[0];
  double h_aspect = (double)comic_container->image_container_list[position]->aspect_raito[1];

  int diff_height_between_windown_and_menu_and_button_bar_height = window_height - (window.menubar_height + window.button_menu_height);
  if(window.isFullScreen) {
    diff_height_between_windown_and_menu_and_button_bar_height = window_height;
  }
  
  int isOverHeight = FALSE;
  if(height > diff_height_between_windown_and_menu_and_button_bar_height) {
    int diff = height - diff_height_between_windown_and_menu_and_button_bar_height;
    height = height - diff;

    isOverHeight = TRUE;
    
    int result = (int)ceil((double)height * (w_aspect / h_aspect));
    width = result;
  }

  // GDK_INTERP_BILINEAR

  
  
  comic_container->image_container_list[position]->dst = gdk_pixbuf_scale_simple(comic_container->image_container_list[position]->src, width, height, GDK_INTERP_BILINEAR);
  comic_container->image_container_list[position]->dst_width = width;
  comic_container->image_container_list[position]->dst_height = height;

  return isOverHeight;
}

gboolean detect_resize_window(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  if(!comic_container->isFirstLoad) {
    gint width = 0;
    gint height = 0;
    gtk_window_get_size((GtkWindow*)window.window, &width, &height);

    if(width != window.width || height != window.height) {
      if(comic_container->pages->isSingle) {

        unref_dst();

        int isOverHeight = resize_when_single(comic_container->pages->current_page);

        gtk_image_clear((GtkImage*)comic_container->pages->left);

        gtk_image_set_from_pixbuf((GtkImage*)comic_container->pages->left, comic_container->image_container_list[comic_container->pages->current_page]->dst);

      } else {
        if(comic_container->detail->isOdd)  {

          unref_dst();

          int isOverHeight = resize_when_single(comic_container->pages->current_page);
          
          if(comic_container->pages->left != NULL)
            gtk_image_clear((GtkImage*)comic_container->pages->left);
          
          if(comic_container->pages->right != NULL)
            gtk_image_clear((GtkImage*)comic_container->pages->right);

          gtk_image_set_from_pixbuf((GtkImage*)comic_container->pages->left, comic_container->image_container_list[comic_container->pages->current_page]->dst);

        } else {

          unref_dst();

          int isOverHeight = resize_when_spread(comic_container->pages->current_page);

          if(comic_container->pages->left != NULL)
            gtk_image_clear((GtkImage*)comic_container->pages->left);
          
          if(comic_container->pages->right != NULL)
            gtk_image_clear((GtkImage*)comic_container->pages->right);

          if(comic_container->pages->page_direction_right) {
            if(check_valid_cover_mode()) {
                gtk_image_set_from_pixbuf((GtkImage*)comic_container->pages->left, comic_container->image_container_list[comic_container->pages->current_page]->dst);
            } else {
              gtk_image_set_from_pixbuf((GtkImage*)comic_container->pages->left, comic_container->image_container_list[comic_container->pages->current_page]->dst);
              gtk_image_set_from_pixbuf((GtkImage*)comic_container->pages->right, comic_container->image_container_list[comic_container->pages->current_page - 1]->dst);
            }
          } else {
            if(check_valid_cover_mode()) {
              gtk_image_set_from_pixbuf((GtkImage*)comic_container->pages->right, comic_container->image_container_list[comic_container->pages->current_page]->dst);
            } else {
              gtk_image_set_from_pixbuf((GtkImage*)comic_container->pages->left, comic_container->image_container_list[comic_container->pages->current_page - 1]->dst);
              gtk_image_set_from_pixbuf((GtkImage*)comic_container->pages->right, comic_container->image_container_list[comic_container->pages->current_page]->dst);
            }
          }

          set_margin_left_page(comic_container->pages->current_page, isOverHeight, FALSE);
        }
      }

      return TRUE;
    }
  }


  return FALSE;
}

void set_image(GtkWidget **img, int position)
{
  *img = gtk_image_new_from_pixbuf(comic_container->image_container_list[position]->dst);
}

void scale_when_oversize(int *x, int *y, int window_width, int window_height, double w_aspect, double h_aspect, int isOverWidth)
{
  if(isOverWidth) {
    int diff = *x - window_width;
    int width = *x - diff;
    int height = (int)ceil((double)width * (h_aspect / w_aspect));

    *x = width;
    *y = height;
  } else {
    int diff = *y - window_height;
    int height = *y - diff;
    int width = (int)ceil((double)height * (w_aspect / h_aspect));

    *x = width;
    *y = height;
  }


}

int resize_when_spread(int page)
{
  gint window_width = 0;
  gint window_height = 0;
  gtk_window_get_size((GtkWindow*)window.window, &window_width, &window_height);
  window.width = window_width;
  window.height = window_height;

  int diff_height_between_window_and_menu_and_button_bar = window_height - (window.menubar_height + window.button_menu_height);
  if(window.isFullScreen) {
    diff_height_between_window_and_menu_and_button_bar = window_height;
  }

  int half_width = window_width / 2;
  int left_width = half_width;
  int left_height = 0;

  int left_pos = 0;
  int right_pos = 0;
  
  int isOverHeight = FALSE;

  int left_src_width = 0;
  int left_src_height = 0;
  double left_x_aspect = 0;
  double left_y_aspect = 0;

  int right_src_width = 0;
  int right_src_height = 0;
  double right_x_aspect = 0;
  double right_y_aspect = 0;

  int right_width = 0;
  int right_height = 0;

  int img_src_width = 0;
  int img_src_height = 0;
  double img_x_aspect = 0;
  double img_y_aspect = 0;
  int img_width = 0;
  int img_height = 0;


  if(check_valid_cover_mode()) {
    goto cover;
  } else {
    goto no_cover;
  }

 cover:
  img_src_width = comic_container->image_container_list[page]->src_width;
  img_src_height = comic_container->image_container_list[page]->src_height;
  img_x_aspect = (double)comic_container->image_container_list[page]->aspect_raito[0];
  img_y_aspect = (double)comic_container->image_container_list[page]->aspect_raito[1];

  img_width = half_width;
  img_height = (int)ceil((double)img_width * (img_y_aspect / img_x_aspect));

  if(img_height > diff_height_between_window_and_menu_and_button_bar) {
    scale_when_oversize(&img_width, &img_height, window_width, diff_height_between_window_and_menu_and_button_bar, img_x_aspect, img_y_aspect, FALSE);
    isOverHeight = TRUE;
  }

  comic_container->image_container_list[page]->dst_width = img_width;
  comic_container->image_container_list[page]->dst_height = img_height;

  comic_container->image_container_list[page]->dst = gdk_pixbuf_scale_simple(comic_container->image_container_list[page]->src, img_width, img_height, GDK_INTERP_BILINEAR);

  return isOverHeight;

 no_cover:
  if(comic_container->pages->page_direction_right) {
    left_pos = page;
    right_pos = page - 1;
  } else {
    left_pos = page - 1;
    right_pos = page;
  }

  /* printf("%d, %d\n", left_pos, right_pos); */
  
  left_src_width = comic_container->image_container_list[left_pos]->src_width;
  left_src_height = comic_container->image_container_list[left_pos]->src_height;
  left_x_aspect = (double)comic_container->image_container_list[left_pos]->aspect_raito[0];
  left_y_aspect = (double)comic_container->image_container_list[left_pos]->aspect_raito[1];

  left_width = half_width;
  left_height = (int)ceil((double)left_width * (left_y_aspect / left_x_aspect));

  right_src_width = comic_container->image_container_list[right_pos]->src_width;
  right_src_height = comic_container->image_container_list[right_pos]->src_height;
  right_x_aspect = (double)comic_container->image_container_list[right_pos]->aspect_raito[0];
  right_y_aspect = (double)comic_container->image_container_list[right_pos]->aspect_raito[1];

  /* printf("page: %d\n", page); */
  /* printf("left: %d, %d\nright: %d, %d\n\n", left_src_width, left_src_height, right_src_width, right_src_height); */
  /* printf("diff: %d\n", diff_height_between_window_and_menu_and_button_bar); */
  /* printf("half width: %d\n", half_width); */

  if(left_height > diff_height_between_window_and_menu_and_button_bar) {
    scale_when_oversize(&left_width, &left_height, window_width, diff_height_between_window_and_menu_and_button_bar, left_x_aspect, left_y_aspect, FALSE);
    isOverHeight = TRUE;
  }

  comic_container->image_container_list[left_pos]->dst_width = left_width;
  comic_container->image_container_list[left_pos]->dst_height = left_height;

  right_width = half_width;
  right_height = (int)ceil((double)right_width * (right_y_aspect / right_x_aspect));

  if(right_height > diff_height_between_window_and_menu_and_button_bar) {
    scale_when_oversize(&right_width, &right_height, window_width, diff_height_between_window_and_menu_and_button_bar, right_x_aspect, right_y_aspect, FALSE);
    isOverHeight = TRUE;
  }

  comic_container->image_container_list[right_pos]->dst_width = right_width;
  comic_container->image_container_list[right_pos]->dst_height = right_height;

  /* printf("dst left: %d, %d\ndst right: %d, %d\n\n", left_width, left_height, right_width, right_height); */

  comic_container->image_container_list[right_pos]->dst = gdk_pixbuf_scale_simple(comic_container->image_container_list[right_pos]->src, right_width, right_height, GDK_INTERP_BILINEAR);
  comic_container->image_container_list[left_pos]->dst = gdk_pixbuf_scale_simple(comic_container->image_container_list[left_pos]->src, left_width, left_height, GDK_INTERP_BILINEAR);

  return isOverHeight;
}

void set_margin_left_page(int position, int isOverHeight, int isFinalPage)
{
  // this function can call only when spread mode.
  /* int left_pos = 0; */
  /* int right_pos = 0; */
  if(isOverHeight) {
    int mix_width = 0;

    mix_width = (comic_container->image_container_list[position - 1]->dst_width + comic_container->image_container_list[position]->dst_width);

    int margin_left = (fmax(mix_width, window.width) - fmin(mix_width, window.width)) / 2;

    if(isFinalPage) {
      if(comic_container->pages->page_direction_right)
        gtk_widget_set_margin_start(comic_container->pages->left, comic_container->image_container_list[position]->dst_width);
      
    } else {
        gtk_widget_set_margin_start(comic_container->pages->left, (gint)margin_left);
    }
  } else {

    if(isFinalPage) {
      gtk_widget_set_margin_start(comic_container->pages->left, 0);
    } else {
      gtk_widget_set_margin_start(comic_container->pages->left, 0);
    }
  }

}

int init_image_object(const char *file_name, uint startpage)
{

  comic_container->pages->current_page = startpage;
  if(!comic_container->isFirstLoad && comic_container->pages->current_page >= comic_container->detail->image_count)
    comic_container->pages->current_page -= 1;
  
  if(comic_container->pages->current_page % 2 && !comic_container->pages->isSingle)
    comic_container->pages->current_page -= 1;
  
  if(comic_container->pages->current_page < 0)
    comic_container->pages->current_page = 0;
  
  if(!comic_container->isFirstLoad) {
    free_array_with_alloced((void**)comic_container->detail->image_path_list, comic_container->detail->image_count);
    comic_container->detail->image_path_list = NULL;

    free_uncompress_data_set(comic_container->uncompressed_file_list);
    comic_container->uncompressed_file_list = NULL;

    fz_clear();

    free_image_container();
    comic_container->image_container_list = NULL;

    comic_container->detail->image_count = 0;

    if(comic_container->pages->left != NULL) {
      gtk_image_clear(GTK_IMAGE(comic_container->pages->left));
    }

    if(comic_container->pages->right != NULL) {
      gtk_image_clear(GTK_IMAGE(comic_container->pages->right));
    }
  }


  /* printf("isCompress: %d, isPDF: %d\n", comic_container->isCompressFile, comic_container->isPDFfile); */
  
  if(comic_container->isCompressFile) {
    if(!set_image_from_compressed_file(file_name)) {
      return FALSE;
    }

  } else if(!comic_container->isCompressFile && comic_container->isPDFfile) {

    if(!set_image_from_pdf_file(file_name)) {
      return FALSE;
    }

  } else {

    if(!set_image_path_list(file_name)) {
      return FALSE;
    }
  }

  int is_over_height = FALSE;
  if(comic_container->detail->image_count > 0) {
    comic_container->image_container_list = (Image_Container_t**)calloc(comic_container->detail->image_count, sizeof(Image_Container_t*));

    if(comic_container->pages->isSingle || comic_container->detail->image_count == 1) {

      if(!comic_container->pages->isSingle) {
        comic_container->pages->isSingle = TRUE;
      }

      set_image_container(comic_container->pages->current_page);
      is_over_height = resize_when_single(comic_container->pages->current_page);
      set_image(&comic_container->pages->left, comic_container->pages->current_page);

    } else {

      if(!comic_container->detail->isOdd) {
        set_image_container(comic_container->pages->current_page);

        if(!(check_valid_cover_mode() ) ) {
          set_image_container(comic_container->pages->current_page + 1);
          is_over_height = resize_when_spread(comic_container->pages->current_page + 1);

        } else {
          is_over_height = resize_when_spread(comic_container->pages->current_page);
        }


        if(comic_container->pages->page_direction_right) {
          if(check_valid_cover_mode() ) {
            set_image(&comic_container->pages->left, comic_container->pages->current_page);
          } else {
            set_image(&comic_container->pages->right, comic_container->pages->current_page);
            set_image(&comic_container->pages->left, comic_container->pages->current_page + 1);
            comic_container->pages->current_page++;

            /* set_margin_left_page(comic_container->pages->current_page, is_over_height, FALSE); */
          }
        } else {
          if(check_valid_cover_mode() ) {
            set_image(&comic_container->pages->right, comic_container->pages->current_page);
          } else {
            set_image(&comic_container->pages->left, comic_container->pages->current_page);
            set_image(&comic_container->pages->right, comic_container->pages->current_page + 1);
            comic_container->pages->current_page++;
          }
        }
        
      } else {
        set_image_container(comic_container->pages->current_page);

        int is_over_height = 0;
        if(!(check_valid_cover_mode()) ) {
          set_image_container(comic_container->pages->current_page + 1);
          is_over_height = resize_when_spread(comic_container->pages->current_page + 1);

          if(comic_container->pages->page_direction_right) {
            set_image(&comic_container->pages->left, comic_container->pages->current_page + 1);
            set_image(&comic_container->pages->right, comic_container->pages->current_page);
          } else {
            set_image(&comic_container->pages->left, comic_container->pages->current_page);
            set_image(&comic_container->pages->right, comic_container->pages->current_page + 1);
          }

          comic_container->pages->current_page++;
          
        } else {

          is_over_height = resize_when_spread(comic_container->pages->current_page);

          if(comic_container->pages->page_direction_right) {
            set_image(&comic_container->pages->left, comic_container->pages->current_page);
          } else {
            set_image(&comic_container->pages->right, comic_container->pages->current_page);
          }

        }

      }
    }

    if(is_over_height && !comic_container->pages->isSingle) {
      if(comic_container->pages->current_page == comic_container->detail->image_count)
        set_margin_left_page(comic_container->pages->current_page, TRUE, TRUE);
      else
        set_margin_left_page(comic_container->pages->current_page, TRUE, FALSE);
    }


    int is_final = 0;
    if(comic_container->pages->current_page == comic_container->detail->image_count - 1)
      is_final = TRUE;
    
    if(comic_container->pages->isSingle) {
      if(comic_container->isFirstLoad)
        set_margin_left_page(comic_container->pages->current_page, is_over_height, is_final);
      else
        update_page(TRUE);
    } else {

      if(comic_container->isFirstLoad) {
        set_margin_left_page(comic_container->pages->current_page, is_over_height, is_final);
      } else {
        update_page(FALSE);        
      }

    }

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

// isSingleChange is mode change of spread to single or spread to single.
void update_page(int isSingleChange)
{
  if(!comic_container->isFirstLoad) {
    if(isSingleChange) {
      
      if(comic_container->pages->isSingle) {
        if(comic_container->detail->isOdd && comic_container->pages->current_page == (comic_container->detail->image_count - 1)) {
          if(comic_container->pages->right != NULL)
            gtk_image_clear(GTK_IMAGE(comic_container->pages->right));
        } else {
          
          if(comic_container->pages->left != NULL) {
            gtk_image_clear(GTK_IMAGE(comic_container->pages->left));
          }
          if(comic_container->pages->right != NULL) {
            gtk_image_clear(GTK_IMAGE(comic_container->pages->right));
          }

        }

        comic_container->pages->current_page -= 1;

        if(comic_container->pages->current_page < 0) {
          comic_container->pages->current_page = 0;
        }

        set_image_container(comic_container->pages->current_page);

        resize_when_single(comic_container->pages->current_page);

        set_image(&comic_container->pages->left, comic_container->pages->current_page);

        update_grid();
      } else {

        if(check_valid_cover_mode() && comic_container->pages->current_page == 0) {
          
          set_image_container(comic_container->pages->current_page);
          int isOverHeight = resize_when_spread(comic_container->pages->current_page);
          set_margin_left_page(comic_container->pages->current_page, isOverHeight, FALSE);
          
          if(comic_container->pages->page_direction_right) {
            gtk_image_clear(GTK_IMAGE(comic_container->pages->left));
            set_image(&comic_container->pages->left, comic_container->pages->current_page);
          } else {
            gtk_image_clear(GTK_IMAGE(comic_container->pages->right));
            set_image(&comic_container->pages->right, comic_container->pages->current_page);
          }
          
          
          update_grid();
        } else {
          gtk_image_clear(GTK_IMAGE(comic_container->pages->left));

          comic_container->pages->current_page++;

          int latest_page = comic_container->detail->image_count - 1;
          if(comic_container->pages->current_page >= latest_page) {
            comic_container->pages->current_page = latest_page - 1;
          }
          set_image_container(comic_container->pages->current_page - 1);
          set_image_container(comic_container->pages->current_page);

          int isOverHeight = resize_when_spread(comic_container->pages->current_page);
          set_margin_left_page(comic_container->pages->current_page, isOverHeight, FALSE);

          set_image(&comic_container->pages->right, comic_container->pages->current_page - 1);
          set_image(&comic_container->pages->left, comic_container->pages->current_page);


          update_grid();
        }
      }

    } else {


      if(check_valid_cover_mode() && comic_container->pages->current_page == 0) {
        set_image_container(comic_container->pages->current_page);
      } else {
        set_image_container(comic_container->pages->current_page);
        set_image_container(comic_container->pages->current_page - 1);
      }

      if(comic_container->pages->right != NULL) {
        gtk_image_clear(GTK_IMAGE(comic_container->pages->right));
      }

      if(comic_container->pages->left != NULL) {
        gtk_image_clear(GTK_IMAGE(comic_container->pages->left));
      }



      if(comic_container->detail->isOdd && comic_container->pages->current_page >= comic_container->detail->image_count - 1) {
        unref_dst();

        int isOverHeight;
        isOverHeight = resize_when_spread(comic_container->pages->current_page);

        if(comic_container->pages->page_direction_right) {

          gtk_image_set_from_pixbuf(GTK_IMAGE(comic_container->pages->right), comic_container->image_container_list[comic_container->pages->current_page]->dst);
          set_margin_left_page(comic_container->pages->current_page, isOverHeight, TRUE);

        } else {
          
          gtk_image_set_from_pixbuf(GTK_IMAGE(comic_container->pages->left), comic_container->image_container_list[comic_container->pages->current_page]->dst);
          
        }

      } else {

        unref_dst();

        int isOverHeight = FALSE;
        isOverHeight = resize_when_spread(comic_container->pages->current_page);

        if(comic_container->pages->page_direction_right) {

          if(check_valid_cover_mode() && comic_container->pages->current_page == 0) {
            gtk_image_set_from_pixbuf(GTK_IMAGE(comic_container->pages->left), comic_container->image_container_list[comic_container->pages->current_page]->dst);
            
          } else {
            
            gtk_image_set_from_pixbuf(GTK_IMAGE(comic_container->pages->right), comic_container->image_container_list[comic_container->pages->current_page - 1]->dst);
            gtk_image_set_from_pixbuf(GTK_IMAGE(comic_container->pages->left), comic_container->image_container_list[comic_container->pages->current_page]->dst);
            
          }

        } else {

          if(check_valid_cover_mode() && comic_container->pages->current_page == 0) {
            
            gtk_image_set_from_pixbuf(GTK_IMAGE(comic_container->pages->right), comic_container->image_container_list[comic_container->pages->current_page]->dst);
            
          } else {
            
            gtk_image_set_from_pixbuf(GTK_IMAGE(comic_container->pages->left), comic_container->image_container_list[comic_container->pages->current_page - 1]->dst);
            gtk_image_set_from_pixbuf(GTK_IMAGE(comic_container->pages->right), comic_container->image_container_list[comic_container->pages->current_page]->dst);
            
          }
        }


        set_margin_left_page(comic_container->pages->current_page, isOverHeight, FALSE);
      }
    }

  }
}

void update_grid()
{
  if(comic_container->pages->isSingle) {
    // cause by crash, stll sruvery.
    /* if(!comic_container->isFirstLoad) { */
    /*   gtk_grid_remove_column(GTK_GRID(grid), 0); */
    /* } */

    gtk_widget_set_vexpand(comic_container->pages->left, TRUE);
    gtk_widget_set_hexpand(comic_container->pages->left, TRUE);
    gtk_grid_attach(GTK_GRID(grid), comic_container->pages->left, 0, 0, 1, 1);

    gtk_widget_show(comic_container->pages->left);

  } else {


    if(!comic_container->isFirstLoad) {
      gtk_grid_remove_column(GTK_GRID(grid), 0);
      gtk_grid_remove_column(GTK_GRID(grid), 0);
    }

    if(check_valid_cover_mode() && comic_container->pages->current_page == 0) {

      if(comic_container->pages->page_direction_right) {

        gtk_widget_set_vexpand(comic_container->pages->left, TRUE);
        gtk_grid_attach(GTK_GRID(grid), comic_container->pages->left, 0, 0, 1, 1);
        gtk_widget_show(comic_container->pages->left);
        
      } else {
        
        gtk_widget_set_vexpand(comic_container->pages->right, TRUE);
        gtk_grid_attach(GTK_GRID(grid), comic_container->pages->right, 0, 0, 1, 1);
        gtk_widget_show(comic_container->pages->right);

      }
    } else {

      if(comic_container->detail->isOdd && comic_container->pages->current_page >= comic_container->detail->image_count) {
        if(comic_container->pages->page_direction_right) {
          gtk_widget_set_vexpand(comic_container->pages->right, TRUE);
          gtk_grid_attach(GTK_GRID(grid), comic_container->pages->right, 0, 0, 1, 1);
          gtk_widget_show(comic_container->pages->right);
        } else {
          gtk_widget_set_vexpand(comic_container->pages->left, TRUE);
          gtk_grid_attach(GTK_GRID(grid), comic_container->pages->left, 0, 0, 1, 1);
          gtk_widget_show(comic_container->pages->left);
        }
      } else {
      
        gtk_widget_set_vexpand(comic_container->pages->left, TRUE);

        gtk_widget_set_vexpand(comic_container->pages->right, TRUE);

        gtk_grid_attach(GTK_GRID(grid), comic_container->pages->left, 0, 0, 1, 1);

        gtk_grid_attach_next_to(GTK_GRID(grid), comic_container->pages->right, comic_container->pages->left, GTK_POS_RIGHT, 1, 1);

        gtk_widget_show(comic_container->pages->left);
        gtk_widget_show(comic_container->pages->right);
      }

    }

  }

  if(comic_container->isFirstLoad) {
    comic_container->isFirstLoad = FALSE;
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

  file_menu_struct.file_history = gtk_menu_item_new_with_label("File History");
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu_struct.body), file_menu_struct.file_history);


  set_file_history_on_menu();
  

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
 if (comic_container->pages->current_page > -1) {

    if(comic_container->pages->isSingle) {
      int tmp = comic_container->pages->current_page + 1;

      if(tmp < comic_container->detail->image_count)
        comic_container->pages->current_page = tmp;

      if(tmp >= comic_container->detail->image_count && comic_container->pages->isAcceptOverflow) {
        comic_container->pages->current_page = 0;
      }

    } else if(comic_container->pages->page_direction_right) {

      int tmp = comic_container->pages->current_page + 2;

      if(tmp < comic_container->detail->image_count)
        comic_container->pages->current_page = tmp;

      if(tmp >= comic_container->detail->image_count) {
        if(tmp != 0 && tmp % 2 != 0) {
          comic_container->pages->current_page = tmp - 1;
        } 

      }

      if(comic_container->pages->isAcceptOverflow && comic_container->pages->current_page >= comic_container->detail->image_count) {
        comic_container->pages->current_page = 1;

      }

    } else {

        int tmp = comic_container->pages->current_page - 2;
        if(tmp < 1) {
          tmp = 1;
        }

        comic_container->pages->current_page = tmp;

        if(comic_container->pages->isAcceptOverflow)
          comic_container->pages->current_page = comic_container->detail->image_count - 1;

    }

    if(!comic_container->pages->isSingle && comic_container->pages->current_page != 0 && comic_container->pages->current_page % 2 == 0) {
      comic_container->pages->current_page++;
    }

    if(comic_container->pages->current_page == comic_container->detail->image_count) {
      comic_container->pages->current_page--;
    }

    if(comic_container->pages->current_page > comic_container->detail->image_count) {
      comic_container->pages->current_page = comic_container->detail->image_count - 1;
    }

    if(comic_container->pages->current_page < 0 && comic_container->pages->isSingle) {
      comic_container->pages->current_page = comic_container->detail->image_count - 1;
    }


    if(comic_container->pages->page_direction_right) {
      next_image(TRUE);
    } else {
      next_image(FALSE);
    }
  }

}


void move_right()
{
  if (comic_container->pages->current_page > -1) {

    if(comic_container->pages->isSingle) {

      int tmp = comic_container->pages->current_page - 1;
      if(tmp >= 0)
        comic_container->pages->current_page = tmp;

      if(tmp < 0 && comic_container->pages->isAcceptOverflow)
        comic_container->pages->current_page = comic_container->detail->image_count - 1;


    } else if(comic_container->pages->page_direction_right) {

      int tmp = comic_container->pages->current_page - 2;

      if(tmp < 0) {

        if(comic_container->pages->isAcceptOverflow)
          comic_container->pages->current_page = comic_container->detail->image_count - 1;

      } else {
        comic_container->pages->current_page = tmp;
      }

    } else {

      int tmp = comic_container->pages->current_page + 2;
      if(tmp <= comic_container->detail->image_count) {
        comic_container->pages->current_page = tmp;
      }

      if(comic_container->pages->current_page >= comic_container->detail->image_count) {
        comic_container->pages->current_page--;
      }

      if(comic_container->pages->current_page > comic_container->detail->image_count && comic_container->pages->isAcceptOverflow) {
        comic_container->pages->current_page = 1;
      }

      /* printf("current page: %d, tmp: %d, image count: %d\n", comic_container->pages->current_page, tmp, comic_container->detail->image_count); */
    }

    if(comic_container->pages->current_page >= comic_container->detail->image_count) {
      if(comic_container->pages->isSingle) {

        comic_container->pages->current_page = 0;

      } else {
        
        comic_container->pages->current_page = 1;
        
      }
    }

    if(!comic_container->pages->isSingle &&
       comic_container->pages->current_page != 0 &&
       comic_container->pages->current_page % 2 == 0 &&
       comic_container->pages->current_page != comic_container->detail->image_count - 1) {
      
      comic_container->pages->current_page++;
      
    }

    if(comic_container->pages->page_direction_right) {
      next_image(FALSE);
    } else {
      next_image(TRUE);
    }

  }
}

void move_top_page()
{
  if (comic_container->pages->current_page > -1) {

    comic_container->pages->current_page = 1;

    if(comic_container->pages->isSingle) {
      comic_container->pages->current_page = 0;
    }

    if(comic_container->pages->page_direction_right) {
      next_image(FALSE);
    } else {
      next_image(TRUE);
    }
  }
}

void move_end_page()
{
  if (comic_container->pages->current_page > -1) {

    comic_container->pages->current_page = comic_container->detail->image_count - 1;
  
    if(comic_container->pages->page_direction_right) {
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
