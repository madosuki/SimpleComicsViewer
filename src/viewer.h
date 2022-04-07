#ifndef VIEWER_H
#define VIEWER_H

#include <gtk/gtk.h>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <dirent.h>
#include <openssl/sha.h>
#include <sys/file.h>

#include "utils.h"
#include "loader.h"

#include "pdf_loader.h"

#include "database_utils.h"

#define LIST_BUFFER 1024
#define DEFAULT_WINDOW_WIDTH 1024
#define DEFAULT_WINDOW_HEIGHT 768

extern const char *db_name;
extern const ssize_t db_name_size;

extern const char *app_dir;
extern const ssize_t app_dir_size;

extern char *db_path_under_dot_local_share;
extern ssize_t db_path_under_dot_local_share_size;

extern char *temporary_db_path;
extern ssize_t temporary_db_path_size;

extern int status;

extern char *arg_file_name;

extern const char *right_to_left_name;
extern const char *left_to_right_name;

extern GtkWidget *change_direction_button;

extern void *cursor_observer_in_fullscreen_mode(void *data);
extern pthread_t thread_of_curosr_observer;

extern db_s db_info;

extern GtkWidget *file_history_internal_list;

extern file_history_s *history;

void set_file_history_on_menu();

void show_menu();
void hide_menu();

void move_right();
void move_left();
void move_top_page();
void move_end_page();

void show_mouse();
void hide_mouse();

void free_array_with_alloced(void **list, const int size);

int get_file_count_and_set_image_path_list(struct dirent **src, const int size, char **dst_image_path_list, const char *dirname);

int create_image_path_list(char **image_path_list, const char *dirname);

int set_image_path_list(const char *dirname);

int set_image_from_compressed_file(const char *file_name);

int set_image_from_pdf_file(const char *file_name);

void set_image_container(ulong position);

void set_image(GtkWidget **img, int position);

int resize_when_single(int position);

int init_image_object(const char *file_name, uint startpage);

void next_image(int isForward);

void free_image_container();

// return value is TRUE or FALSE
int resize_when_spread(int page);

void scale_when_oversize(int *x, int *y, int window_width, int window_height, double w_aspect, double h_aspect, int isOverWidth);

// isOverHeight is only accept TRUE or FALSE
void set_margin_left_page(int position, int isOverHeight, int isFinalPage);

gboolean my_key_press_function(GtkWidget *widget, GdkEventKey *event, gpointer data);

gboolean detect_resize_window(GtkWidget *widget, GdkEvent *event, gpointer data);

gboolean my_detect_click_function(GtkWidget *widget, GdkEventButton *event,
                                  gpointer data);

int check_valid_cover_mode();


void close_variables();

void cancel_fullscreen();

void fullscreen();

void update_grid();

void update_page(int isSingleChange);

typedef struct
{
  GtkWidget *left;
  GtkWidget *right;
  int isSingle;
  int page_direction_right;
  int current_page;
  int isFinalPage;
  int isAcceptOverflow;
} Pages;

typedef struct
{
  int isOdd;
  int image_count;
  char **image_path_list;
} DirectoryDetail_t;


typedef struct
{
  GdkPixbuf *src;
  GdkPixbuf *dst;
  GError *err;
  int src_width;
  int src_height;
  int dst_width;
  int dst_height;
  double *aspect_raito;
} Image_Container_t;


typedef struct
{
  GtkWidget *scrolled_window;
  gint width;
  gint height;
} DrawingArea_t;

typedef struct
{
  GtkWidget *left_image_button;
  GtkWidget *right_image_button;
} Image_button_t;

typedef struct
{
  guint x;
  guint y;
} Cursor_Position_t;

typedef struct
{
  Pages *pages; // pages settings.
  DirectoryDetail_t *detail; // a directory and file and there's count info.
  Image_Container_t **image_container_list;
  uncompress_data_set_t *uncompressed_file_list;
  
  int isCompressFile; // file mode when open
  int isPDFfile; // file mode when open
  int isFirstLoad; // whether file opened or not.
  int isCoverMode; // cover mode on or off.
} comic_container_t;
extern comic_container_t *comic_container;


extern Cursor_Position_t cursor_pos;

extern Image_button_t image_button;

extern DrawingArea_t draw_area;


extern GtkWidget *grid;

extern GtkApplication *app;

GtkWidget *create_menu_bar();

typedef struct
{
  GtkWidget *window;
  GtkWidget *menubar;
  int menubar_width;
  int menubar_height;
  int button_menu_width;
  int button_menu_height;
  int width;
  int height;
  int isFullScreen;
  int isClose;
} main_window_data_t;

typedef struct
{
  GtkWidget **list;
  int size;
} file_history_on_menu_t;

typedef struct
{
  GtkWidget *body;
  GtkWidget *root;
  GtkWidget *load;
  GtkWidget *quit;
  GtkWidget *file_history;
} file_menu_t; 

typedef struct
{
  GtkWidget *body;
  GtkWidget *root;
  GtkWidget *page_direction;
  GtkWidget *set_single_mode;
  GtkWidget *set_spread_mode;
} view_menu_t;

typedef struct
{
  GtkWidget *body;
  GtkWidget *root;
  GtkWidget *about;
} help_menu_t;

extern main_window_data_t window;

extern file_menu_t file_menu_struct;

extern file_history_on_menu_t file_history_on_menu_struct;

extern view_menu_t view_menu_struct;

extern help_menu_t help_menu_struct;

extern GtkWidget *button_menu;


static void change_spread_to_single()
{
  if(comic_container->pages != NULL && !comic_container->pages->isSingle) {
    comic_container->pages->isSingle = TRUE;

    update_page(TRUE);
  }
}

static void change_single_to_spread()
{
  if(comic_container->pages != NULL && comic_container->pages->isSingle) {
    comic_container->pages->isSingle = FALSE;

    update_page(TRUE);
  }
}


static void change_direction()
{
  if(comic_container->pages->page_direction_right) {
    comic_container->pages->page_direction_right = FALSE;
    gtk_button_set_label(GTK_BUTTON(change_direction_button), left_to_right_name);
  } else {
    comic_container->pages->page_direction_right = TRUE;
    gtk_button_set_label(GTK_BUTTON(change_direction_button), right_to_left_name);
  }

  if(comic_container->pages->left != NULL || comic_container->pages->right != NULL) {
    update_page(FALSE);
  }
  
}

static int open_file(const char *file_name)
{
  char* tmp;
  int is_dir = FALSE;

  int check = detect_image_from_file(file_name);
  if(check) {
    tmp = get_directory_path_from_filename(file_name);
    if(tmp != NULL) {
      is_dir = TRUE;
      comic_container->isCompressFile = FALSE;
      comic_container->isPDFfile = FALSE;
    }
  } else {
    if(detect_compress_file(file_name)) {
      
      is_dir = FALSE;
      comic_container->isPDFfile = FALSE;
      comic_container->isCompressFile = TRUE;
      
    } else {

      if(test_open_pdf(file_name)) {
        is_dir = FALSE; 
        comic_container->isPDFfile = TRUE;
        comic_container->isCompressFile = FALSE;
      }
    }

  }

  int init_check;
  if (is_dir) {
    init_check = init_image_object(tmp, 0);
  } else {
    init_check =  init_image_object(file_name, 0);
  }

  if(init_check) {

    time_t t = time(NULL);
    long unixtime = t;
    insert_or_udpate_file_history(&db_info, file_name, strlen(file_name), unixtime);

    set_file_history_on_menu();
    
    update_grid();
  } else {
    printf("init image error\n");
    return FALSE;
  }

  return TRUE;
}

static void open_file_in_file_history(GtkWidget *widget, gpointer n)
{
  if(history != NULL) {
   int i = GPOINTER_TO_INT(n);

   if(history->file_path_name_list != NULL && history->file_path_name_list[i]->data != NULL) {
     int err = open_file(history->file_path_name_list[i]->data);
     if(!err) {
       printf("failed open file in open_file_in_file_history\n");
     }
   }
  }
  
}

static void open_file_on_menu()
{
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;

  GtkWidget *dialog = gtk_file_chooser_dialog_new("Select Open File", GTK_WINDOW(window.window), action, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);

  gint res = gtk_dialog_run(GTK_DIALOG(dialog));

  if(res == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    char *file_name = gtk_file_chooser_get_filename(chooser);
    if(file_name == NULL) {
      goto end;
    }

    int check = open_file(file_name);

    if (!check)
      printf("open_file error\n");


    g_free(file_name);

  }

 end:
  gtk_widget_destroy(dialog);
}

static void CloseWindow()
{
  close_variables();
  gtk_window_close(GTK_WINDOW(window.window));

}

static void get_draw_area_widget_size(GtkWidget *widget, GtkAllocation *allocation, void *data)
{
  draw_area.width = allocation->width;
  draw_area.height = allocation->height;
}

static void get_button_menu_widget_size(GtkWidget *widget, GtkAllocation *allocation, void *data)
{
  window.button_menu_width = allocation->width;
  window.button_menu_height = allocation->height;
}

static void get_menu_bar_widget_size(GtkWidget *widget, GtkAllocation *allocation, void *data)
{
  window.menubar_width = allocation->width;
  window.menubar_height = allocation->height;
}


static void print_hello(GtkWidget *widget, gpointer data)
{
  g_print("Hello World\n");
}

static gboolean my_detect_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
  guint x = (guint)event->x;
  guint y = (guint)event->y;

  cursor_pos.x = x;
  cursor_pos.y = y;

  if (window.isFullScreen) {

    if (y == 0) {
      show_menu();
      show_mouse();
    } else if (y > (window.button_menu_height + window.menubar_height)) {
      hide_menu();
    }
    
  }

  return TRUE;
}

static gboolean my_detect_wheel_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{

  if(comic_container->pages != NULL) {

    GdkScrollDirection direction;
    gboolean result = gdk_event_get_scroll_direction(event, &direction);


    if(direction == GDK_SCROLL_UP) {
      if(comic_container->pages->page_direction_right) {
        move_right();
      } else {
        move_left();
      }
    } else if(direction == GDK_SCROLL_DOWN) {
      if(comic_container->pages->page_direction_right) {
        move_left();
      } else {
        move_right();
      }
    }
  }

  return TRUE;
}

static gint run_cmd_argument(GApplication *app, GApplicationCommandLine *app_cmd, int *argc)
{
  g_application_activate(app);

  if (arg_file_name != NULL) {

    // char **argv = g_application_command_line_get_arguments(app_cmd, argc);

    /* GFile *file = g_file_new_for_commandline_arg(argv[1]); */


    int check = open_file(arg_file_name);

    if (check) {
      return 1;
    } else {
      printf("failed open file\n");
      return 0;
    }

  }

  return 1;
}

static int check_hash_from_cp(const char *dst_file_path, const ssize_t dst_byte_size, uint8_t *src_bytes, const ssize_t src_byte_size)
{

  uint8_t *dst_bytes = (uint8_t*)calloc(dst_byte_size, 1);
  FILE *dst_fp = fopen(dst_file_path, "rb");
  if(dst_fp == NULL) {
    free(dst_bytes);
    dst_bytes = NULL;

    return FALSE;
  }


    
  int count = fread(dst_bytes, 1, dst_byte_size, dst_fp);
  if(count < dst_byte_size) {
    fclose(dst_fp);

    free(dst_bytes);
    dst_bytes = NULL;
    
    return FALSE;
  }
  fclose(dst_fp);



  uint8_t *src_sha256 = (uint8_t*)calloc(SHA256_DIGEST_LENGTH, 1);
  if(src_sha256 == NULL) {
    free(dst_bytes);
    dst_bytes = NULL;

    return FALSE;
  }

  int check = get_hash(src_bytes, src_byte_size, src_sha256);
  if(!check) {
    free(dst_bytes);
    dst_bytes = NULL;

    free(src_sha256);
    src_sha256 = NULL;

    return FALSE;
    
  }
  
  uint8_t *dst_sha256 = (uint8_t*)calloc(SHA256_DIGEST_LENGTH, 1);
  if(dst_sha256 == NULL) {
    free(dst_bytes);
    dst_bytes = NULL;
    
    free(src_sha256);
    src_sha256 = NULL;
    return FALSE;
  }

  check = get_hash(dst_bytes, dst_byte_size, dst_sha256);
  if(!check) {
    free(dst_bytes);
    dst_bytes = NULL;

    free(src_sha256);
    src_sha256 = NULL;

    free(dst_sha256);
    dst_sha256 = NULL;

    return FALSE;
    
  }



  int condition = FALSE;
  if(memcmp(src_sha256, dst_sha256, SHA256_DIGEST_LENGTH) == 0)
    condition = TRUE;

  free(dst_bytes);
  dst_bytes = NULL;
    
  free(src_sha256);
  src_sha256 = NULL;

  free(dst_sha256);
  dst_sha256 = NULL;


  return condition;
}

static int cp(const char* src_file_path, const ssize_t src_file_path_size, const char *dst_file_path, const ssize_t dst_file_path_size)
{

  /* printf("%s, %s\n", src_file_path, dst_file_path); */
  
  struct stat src_stat;
  stat(src_file_path, &src_stat);
  if(!(S_ISREG(src_stat.st_mode))) {
    puts("not found src in cp");
    return FALSE;
  }
  ssize_t src_byte_size = src_stat.st_size;

  FILE *src_fp = fopen(src_file_path, "rb");
  if(src_fp == NULL) {
    return FALSE;
  }
  uint8_t *src_bytes = (uint8_t*)calloc(src_byte_size, 1);
  int count = fread(src_bytes, 1, src_byte_size, src_fp);
  if(count < src_byte_size) {
    fclose(src_fp);
    
    free(src_bytes);
    src_bytes = NULL;

    return FALSE;
  }
  fclose(src_fp);

  struct stat dst_stat;
  stat(dst_file_path, &dst_stat);
  if(S_ISREG(dst_stat.st_mode)) {

    int check = check_hash_from_cp(dst_file_path, dst_stat.st_size, src_bytes, src_byte_size);
    if(check) {
      free(src_bytes);
      src_bytes = NULL;

      /* printf("was don't copy because same hash between src and dst.\n"); */

      return FALSE;
    }
  }

  printf("%s\n", dst_file_path);

  FILE *dst_fp = fopen(dst_file_path, "wb");
  if(dst_fp == NULL) {
    free(src_bytes);
    src_bytes = NULL;

    puts("failed fopen dst in cp");

    return FALSE;
  }

  count = fwrite(src_bytes, 1, src_byte_size, dst_fp);
  if(count < src_byte_size) {

    fclose(dst_fp);
    
    free(src_bytes);
    src_bytes = NULL;

    return FALSE;
  }

  free(src_bytes);
  src_bytes = NULL;

  fclose(dst_fp);
  
  return TRUE;
}

static int backup_db()
{
  if(db_path_under_dot_local_share != NULL && temporary_db_path != NULL) {
    int err = cp(temporary_db_path, temporary_db_path_size, db_path_under_dot_local_share, db_path_under_dot_local_share_size);
    /* printf("backup_db: %d\n", err); */
    
    return err;
  }

  return FALSE;
}

static int set_temporary()
{

  int condition = TRUE;
  const char *temporary = "/tmp";
  struct stat temporary_stat;
  int err = stat(temporary, &temporary_stat);
  if(err != 0 || !S_ISDIR(temporary_stat.st_mode)) {
    return FALSE;
  }

  const ssize_t temporary_size = 4;
  const ssize_t temporary_data_dir_size = temporary_size + app_dir_size + 1;
  char *temporary_data_dir = (char*)calloc(temporary_data_dir_size + 1, 1);
  if(temporary_data_dir == NULL) {
    return FALSE;
  }
  
  ssize_t temporary_data_dir_pos = 0;
  memmove(temporary_data_dir, temporary, temporary_size);
  temporary_data_dir_pos += temporary_size;
  memmove(temporary_data_dir + temporary_data_dir_pos, "/", 1);
  ++temporary_data_dir_pos;
  memmove(temporary_data_dir + temporary_data_dir_pos, app_dir, app_dir_size);
  temporary_data_dir[temporary_data_dir_size] = '\0';

  struct stat temporary_data_dir_stat;
  err = stat(temporary_data_dir, &temporary_data_dir_stat);
  /* printf("%d\n", temporary_data_dir_stat.st_mode); */
  if(err != 0 || !S_ISDIR(temporary_data_dir_stat.st_mode)) {

    if(mkdir(temporary_data_dir, 0755) != 0) {
      condition = FALSE;
    
      goto end_temporary_data_dir;
    }
  }

  temporary_db_path_size = temporary_data_dir_size + db_name_size + 1;
  temporary_db_path = calloc(temporary_db_path_size + 1, 1);
  if(temporary_db_path == NULL) {
    condition = FALSE;
    temporary_db_path_size = 0;
    
    goto end_temporary_data_dir;
  }
  ssize_t temporary_db_path_pos = 0;
  memmove(temporary_db_path, temporary_data_dir, temporary_data_dir_size);
  temporary_db_path_pos += temporary_data_dir_size;
  free(temporary_data_dir);
  temporary_data_dir = NULL;

  memmove(temporary_db_path + temporary_db_path_pos, "/", 1);
  ++temporary_db_path_pos;

  
  memmove(temporary_db_path + temporary_db_path_pos, db_name, db_name_size);
  temporary_db_path[temporary_db_path_size] = '\0';

  return condition;


 end_temporary_data_dir:
  free(temporary_data_dir);
  temporary_data_dir = NULL;
      
  return condition;
}

static int set_local_share()
{
  char *home_directory = NULL;
  if((home_directory = getenv("HOME")) == NULL ) {
    puts("missing variable named HOME from env.");
    
    return FALSE;
  } else {
    const ssize_t home_size = strlen(home_directory);

    const char *dot_local = "/.local";
    const int dot_local_size = 7;
    
    const char *share = "/share";
    const int share_size = 6;

    int dot_local_path_size = home_size + dot_local_size;
    char *dot_local_path = (char*)calloc(dot_local_path_size + 1, 1);
    if(dot_local_path == NULL) {
      return FALSE;
    }
    ssize_t dot_local_path_pos = 0;
    memmove(dot_local_path, home_directory, home_size);
    dot_local_path_pos += home_size;
    memmove(dot_local_path + dot_local_path_pos, dot_local, dot_local_size);
    dot_local_path[dot_local_path_size] = '\0';

    
    struct stat stat_dir;
    int err = stat(dot_local_path, &stat_dir);
    if(err != 0 || !S_ISDIR(stat_dir.st_mode)) {
      err = mkdir(dot_local_path, 0755);
      if(err != 0) {
        free(dot_local_path);
        dot_local_path = NULL;
        return FALSE;
      }
      
    }

    const ssize_t local_share_size = dot_local_path_size + share_size;
    char *local_share = (char*)calloc(local_share_size + 1, 1);
    if(local_share == NULL) {
      free(dot_local_path);
      dot_local_path = NULL;
      
      return FALSE;
    }
    
    ssize_t local_share_pos = 0;
    memmove(local_share, dot_local_path, dot_local_path_size);
    local_share_pos += dot_local_path_size;
    free(dot_local_path);
    dot_local_path = NULL;
    
    memmove(local_share + local_share_pos, share, share_size);
    
    local_share[local_share_size] = '\0';

    struct stat local_share_stat;
    err = stat(local_share, &local_share_stat);
    if(err != 0 || !S_ISDIR(local_share_stat.st_mode)) {
      err = mkdir(local_share, 0755);
      if(err != 0) {
        free(local_share);
        local_share = NULL;
        return FALSE;
      }
    }

    const ssize_t app_dir_in_local_share_size = local_share_size + app_dir_size + 1;
    char *app_dir_in_local_share = (char*)calloc(app_dir_in_local_share_size + 1, 1);
    if(app_dir_in_local_share == NULL) {
      free(local_share);
      local_share = NULL;

      return FALSE;
    }
    ssize_t app_dir_in_local_share_pos = 0;
    
    memmove(app_dir_in_local_share, local_share, local_share_size);
    app_dir_in_local_share_pos += local_share_size;
    free(local_share);
    local_share = NULL;
    
    memmove(app_dir_in_local_share + app_dir_in_local_share_pos, "/", 1);
    ++app_dir_in_local_share_pos;
    
    memmove(app_dir_in_local_share + app_dir_in_local_share_pos, app_dir, app_dir_size);
    app_dir_in_local_share[app_dir_in_local_share_size] = '\0';


    struct stat app_dir_in_local_share_stat;
    err = stat(app_dir_in_local_share, &app_dir_in_local_share_stat);
    if(err != 0 || !S_ISDIR(app_dir_in_local_share_stat.st_mode)) {
      err = mkdir(app_dir_in_local_share, 0755);
      if(err != 0) {
        free(app_dir_in_local_share);
        app_dir_in_local_share = NULL;
        return FALSE;
      }
    }
    
    db_path_under_dot_local_share_size = app_dir_in_local_share_size + db_name_size + 1;
    ssize_t pos = 0;
    db_path_under_dot_local_share = (char*)calloc(db_path_under_dot_local_share_size + 1, 1);
    if(db_path_under_dot_local_share == NULL) {
      free(app_dir_in_local_share);
      app_dir_in_local_share = NULL;

      db_path_under_dot_local_share_size = 0;

      return FALSE;
    }
    
    memmove(db_path_under_dot_local_share, app_dir_in_local_share, app_dir_in_local_share_size);
    pos += app_dir_in_local_share_size;
    free(app_dir_in_local_share);
    app_dir_in_local_share = NULL;

    memmove(db_path_under_dot_local_share + pos, "/", 1);
    ++pos;

    memmove(db_path_under_dot_local_share + pos, db_name, db_name_size);
    db_path_under_dot_local_share[db_path_under_dot_local_share_size] = '\0';


  }

  return TRUE;
}

static void activate(GtkApplication* app, gpointer user_data)
{
  
  // Window settings
  window.window = gtk_application_window_new(app);
  window.width = DEFAULT_WINDOW_WIDTH;
  window.height = DEFAULT_WINDOW_HEIGHT;
  window.isFullScreen = FALSE;
  window.isClose = FALSE;
  window.menubar_height = 0;
  window.menubar_width = 0;
  window.button_menu_width = 0;
  window.button_menu_height = 0;
  // g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(Close), NULL);

  // Set Window Title
  gtk_window_set_title(GTK_WINDOW(window.window), "Simple Comix Viewer");

  // Set Window Size
  gtk_window_set_default_size(GTK_WINDOW(window.window), window.width, window.height);

  gtk_widget_add_events(window.window, GDK_KEY_PRESS_MASK);
  g_signal_connect(G_OBJECT(window.window), "key-press-event", G_CALLBACK(my_key_press_function), NULL);

  g_signal_connect(G_OBJECT(window.window), "configure-event", G_CALLBACK(detect_resize_window), NULL);
  g_signal_connect(G_OBJECT(window.window), "window-state-event", G_CALLBACK(detect_resize_window), NULL);

  /* gtk_widget_add_events(window.window, GDK_TOUCH_MASK); */
  /* g_signal_connect(G_OBJECT(window.window), "touch-event", G_CALLBACK(my_detect_touch_function), NULL); */

  comic_container = malloc(sizeof(comic_container_t));
  comic_container->image_container_list = NULL;
  comic_container->uncompressed_file_list = NULL;
  comic_container->detail = NULL;
  comic_container->pages = NULL;
  comic_container->isPDFfile = FALSE;
  comic_container->isFirstLoad = TRUE;
  comic_container->isCompressFile = TRUE;
  comic_container->isCoverMode = FALSE;


  if(!set_local_share()) {
    puts("failed set_local_share");
  }

  if(!set_temporary()) {
    puts("failed set_temporay");
  }
  

  
  file_history_on_menu_struct.size = 0;

  if(temporary_db_path != NULL) {

    if(db_path_under_dot_local_share != NULL) {
      int check = cp(db_path_under_dot_local_share, db_path_under_dot_local_share_size, temporary_db_path, temporary_db_path_size);

      db_info.file_path = temporary_db_path;
      create_file_history_table(&db_info);

    }
    
  }

  cursor_pos.x = 0;
  cursor_pos.y = 0;

  // create menu bar base
  GtkWidget *top_grid = gtk_grid_new();
  g_object_set(top_grid, "expand", TRUE, NULL); 
  gtk_container_add(GTK_CONTAINER(window.window), top_grid);

  // Create Vertical Box
  // GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  // GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

  // gtk_container_add(GTK_CONTAINER(window.window), vbox);

  // settings menubar
  window.menubar = create_menu_bar();
  gtk_grid_attach(GTK_GRID(top_grid), window.menubar, 0, 0, 1, 1);
  g_signal_connect(window.menubar, "size-allocate", G_CALLBACK(get_menu_bar_widget_size), NULL);

  // add menubar
  // gtk_box_pack_start(GTK_BOX(vbox), window.menubar, FALSE, FALSE, 0);

  // add page area
  // gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

  button_menu = gtk_grid_new();
  // g_object_set(button_menu, "expand", TRUE, NULL);
  gtk_grid_attach_next_to(GTK_GRID(top_grid), button_menu, window.menubar, GTK_POS_BOTTOM, 1, 1);
  g_signal_connect(button_menu, "size-allocate", G_CALLBACK(get_button_menu_widget_size), NULL);

  GtkWidget *goto_left_button = gtk_button_new_with_label("Left");
  gtk_grid_attach(GTK_GRID(button_menu), goto_left_button, 0, 0, 1, 1);
  g_signal_connect(G_OBJECT(goto_left_button), "clicked", G_CALLBACK(move_left), NULL);
  /* gtk_widget_set_hexpand(goto_left_button, TRUE); */

  GtkWidget *goto_right_button = gtk_button_new_with_label("Right");
  gtk_grid_attach_next_to(GTK_GRID(button_menu), goto_right_button, goto_left_button, GTK_POS_RIGHT, 1, 1);
  g_signal_connect(G_OBJECT(goto_right_button), "clicked", G_CALLBACK(move_right), NULL);

  GtkWidget *goto_top_page_button = gtk_button_new_with_label("Go to Top page");
  gtk_grid_attach_next_to(GTK_GRID(button_menu),  goto_top_page_button, goto_right_button, GTK_POS_RIGHT, 1, 1);
  g_object_set( goto_top_page_button, "margin-left", 20, NULL);
  g_signal_connect(G_OBJECT(goto_top_page_button), "clicked", G_CALLBACK(move_top_page), NULL);

  GtkWidget *goto_end_page_button = gtk_button_new_with_label("Go to End page");
  gtk_grid_attach_next_to(GTK_GRID(button_menu),  goto_end_page_button, goto_top_page_button, GTK_POS_RIGHT, 1, 1);
  g_signal_connect(G_OBJECT(goto_end_page_button), "clicked", G_CALLBACK(move_end_page), NULL);


  // GtkWidget *change_direction_button = gtk_button_new_with_label("Change Page Direction");
  change_direction_button = gtk_button_new_with_label(right_to_left_name);
  gtk_grid_attach_next_to(GTK_GRID(button_menu),  change_direction_button, goto_end_page_button, GTK_POS_RIGHT, 1, 1);
  g_object_set(change_direction_button, "margin-left", 20, NULL);
  g_signal_connect(G_OBJECT(change_direction_button), "clicked", G_CALLBACK(change_direction), NULL);


  GtkWidget *goto_fullscreen_mode_button = gtk_button_new_with_label("FullScreenMode");
  gtk_grid_attach_next_to(GTK_GRID(button_menu), goto_fullscreen_mode_button, change_direction_button, GTK_POS_RIGHT, 1, 1);
  g_object_set(goto_fullscreen_mode_button, "margin-left", 20, NULL);
  g_signal_connect(G_OBJECT(goto_fullscreen_mode_button), "clicked", G_CALLBACK(fullscreen), NULL);

  GtkWidget *open_file_button = gtk_button_new_with_label("Open");
  gtk_grid_attach_next_to(GTK_GRID(button_menu), open_file_button, goto_fullscreen_mode_button, GTK_POS_RIGHT, 1, 1);
  g_object_set(open_file_button, "margin-left", 20, NULL);
  g_signal_connect(G_OBJECT(open_file_button), "clicked", G_CALLBACK(open_file_on_menu), NULL);

  GtkWidget *quit_button = gtk_button_new_with_label("Quit Application");
  gtk_grid_attach_next_to(GTK_GRID(button_menu), quit_button, open_file_button, GTK_POS_RIGHT, 1, 1);
  g_object_set(quit_button, "margin-left", 20, NULL);
  g_signal_connect(G_OBJECT(quit_button), "clicked", G_CALLBACK(CloseWindow), NULL);

  
  // Initial Scroll Window
  draw_area.scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  draw_area.width = 0;
  draw_area.height = 0;

  g_object_set(draw_area.scrolled_window, "expand", TRUE, NULL);
  // gtk_box_pack_end(GTK_BOX(hbox), draw_area.scrolled_window, TRUE, TRUE, 0);

  // gtk_grid_attach_next_to(GTK_GRID(top_grid), draw_area.scrolled_window, window.menubar, GTK_POS_BOTTOM, 1, 1);
  gtk_grid_attach_next_to(GTK_GRID(top_grid), draw_area.scrolled_window, button_menu, GTK_POS_BOTTOM, 1, 1);

  // pages of grid
  grid = gtk_grid_new();
  // gtk_container_add(GTK_CONTAINER(draw_area.scrolled_window), grid);

  GtkWidget *event_box_on_pages_grid = gtk_event_box_new();
  gtk_container_add(GTK_CONTAINER(event_box_on_pages_grid), grid);
  gtk_container_add(GTK_CONTAINER(draw_area.scrolled_window), event_box_on_pages_grid);
  gtk_widget_add_events(event_box_on_pages_grid, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(G_OBJECT(event_box_on_pages_grid), "button-press-event", G_CALLBACK(my_detect_click_function), NULL);
  
  gtk_widget_add_events(event_box_on_pages_grid, GDK_POINTER_MOTION_MASK);
  g_signal_connect(G_OBJECT(event_box_on_pages_grid), "motion-notify-event", G_CALLBACK(my_detect_motion_notify), NULL);

  gtk_widget_add_events(event_box_on_pages_grid, GDK_SCROLL_MASK);
  g_signal_connect(G_OBJECT(event_box_on_pages_grid), "scroll-event", G_CALLBACK(my_detect_wheel_event), NULL);
  

  // init pages struct
  comic_container->pages = (Pages*)malloc(sizeof(Pages));
  comic_container->pages->page_direction_right = TRUE;
  comic_container->pages->isSingle = FALSE;
  comic_container->pages->isFinalPage = FALSE;
  comic_container->pages->isAcceptOverflow = FALSE;
  comic_container->pages-> current_page = -1;

  gtk_widget_show_all(window.window);

  // get menubar size;
  GtkAllocation *alloc = g_new(GtkAllocation, 1);
  gtk_widget_get_allocation(window.menubar, alloc);
  window.menubar_height = alloc->height;
  window.menubar_width = alloc->width;
  g_free(alloc);

}

#endif // VIEWER_H
