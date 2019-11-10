#ifndef VIEWER_H
#define VIEWER_H

#include <gtk/gtk.h>
#include <dirent.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "utils.h"
#include "loader.h"

#define LIST_BUFFER 1024
#define DEFAULT_WINDOW_WIDTH 1024
#define DEFAULT_WINDOW_HEIGHT 768

int status;

void move_right();
void move_left();

void hide_mouse();

void free_array_with_alloced(void **list, const int size);

int get_image_file_count_from_directory(struct dirent **src, const int size, int *dst, const char *dirname);

int create_image_path_list(char **image_path_list, const char *dirname);

int set_image_path_list(const char *dirname);

int set_image_from_compressed_file(const char *file_name);

void set_image_container(int position);

void set_image(GtkWidget **img, int position);

void resize_when_single(int position);

int init_image_object();

void next_image(int isForward);

void free_image_container();

// return value is TRUE or FALSE
int resize_when_spread(int page);

void scale_when_oversize(int *x, int *y, int window_width, int window_height, double w_aspect, double h_aspect, int isOverWidth);

// isOverHeight is only accept TRUE or FALSE
void set_margin_left_page(int position, int isOverHeight, int isFinalPage);

gboolean my_key_press_function(GtkWidget *widget, GdkEventKey *event, gpointer data);

gboolean detect_resize_window(GtkWidget *widget, GdkEvent *event, gpointer data);


void close_variables();

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
  int isPriorityToFrontCover;
} Pages;

Pages *pages;

typedef struct
{
  int isOdd;
  int image_count;
  char **image_path_list;
} DirectoryDetail_t;

DirectoryDetail_t *detail;

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

Image_button_t image_button;

Image_Container_t **image_container_list;

uncompress_data_set_t *uncompressed_file_list;

DrawingArea_t draw_area;

int isCompressFile;

int isFirstLoad;

GtkWidget *grid;

GtkApplication *app;

GtkWidget *create_menu_bar();

typedef struct
{
  GtkWidget *window;
  GtkWidget *menubar;
  int menubar_width;
  int menubar_height;
  int width;
  int height;
  int isFullScreen;
  int isClose;
} main_window_data_t;

typedef struct
{
  GtkWidget *body;
  GtkWidget *root;
  GtkWidget *load;
  GtkWidget *quit;
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

main_window_data_t window;

file_menu_t file_menu_struct;

view_menu_t view_menu_struct;

help_menu_t help_menu_struct;

static void change_spread_to_single()
{
  if(pages != NULL && !pages->isSingle) {
    pages->isSingle = TRUE;

    update_page(TRUE);
  }
}

static void change_single_to_spread()
{
  if(pages != NULL && pages->isSingle) {
    pages->isSingle = FALSE;

    update_page(TRUE);
  }
}


static void change_direction()
{
  if(pages->left != NULL) {

    if(pages->page_direction_right) {
      pages->page_direction_right = FALSE;
    } else {
      pages->page_direction_right = TRUE;
    }

    update_page(FALSE);

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

    uint8_t flag = detect_compress_file(file_name);
    if((flag & UTILS_ZIP)) {
      isCompressFile = TRUE;
    } else {
      int check = detect_image_from_file(file_name);

      if(check) {
        char *tmp = get_directory_path_from_filename(file_name);
        if(tmp != NULL) {
          g_free(file_name);

          file_name = tmp;

          printf("%s\n", file_name);
        }
      } else {
        g_free(file_name);

        goto end;
      }

      isCompressFile = FALSE;
    }

    if(init_image_object(file_name, 0)) {
      update_grid();
    } else {
      printf("init image error\n");
    }

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

static void get_widget_size(GtkWidget *widget, GtkAllocation *allocation, void *data)
{
  draw_area.width = allocation->width;
  draw_area.height = allocation->height;
}

static void print_hello(GtkWidget *widget, gpointer data)
{
  g_print("Hello World\n");
}

static void activate(GtkApplication* app, gpointer user_data)
{
  // Window settings
  window.window = gtk_application_window_new(app);
  window.width = DEFAULT_WINDOW_WIDTH;
  window.height = DEFAULT_WINDOW_HEIGHT;
  window.isFullScreen = FALSE;
  window.isClose = FALSE;
  // g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(Close), NULL);

  // Set Window Title
  gtk_window_set_title(GTK_WINDOW(window.window), "Simple Comix Viewer");

  // Set Window Size
  gtk_window_set_default_size(GTK_WINDOW(window.window), window.width, window.height);

  gtk_widget_add_events(window.window, GDK_KEY_PRESS_MASK);
  g_signal_connect(G_OBJECT(window.window), "key-press-event", G_CALLBACK(my_key_press_function), NULL);

  g_signal_connect(G_OBJECT(window.window), "configure-event", G_CALLBACK(detect_resize_window), NULL);

  isFirstLoad = TRUE;

  // Create Vertical Box
  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

  gtk_container_add(GTK_CONTAINER(window.window), vbox);

  // settings menubar
  window.menubar = create_menu_bar();

  // add menubar
  gtk_box_pack_start(GTK_BOX(vbox), window.menubar, FALSE, FALSE, 0);

  // add page area
  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

  // Initial Scroll Window
  draw_area.scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  draw_area.width = 0;
  draw_area.height = 0;
  gtk_box_pack_end(GTK_BOX(hbox), draw_area.scrolled_window, TRUE, TRUE, 0);

  grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(draw_area.scrolled_window), grid);

  // init pages struct
  pages = (Pages*)malloc(sizeof(Pages));
  memset(pages, 0, sizeof(Pages));

  pages->page_direction_right = TRUE;
  pages->isSingle = FALSE;
  pages->isFinalPage = FALSE;
  // pages->isPriorityToFrontCover = TRUE;
  pages->isPriorityToFrontCover = FALSE;

  isCompressFile = TRUE;

  gtk_widget_show_all(window.window);

  // get menubar size;
  GtkAllocation *alloc = g_new(GtkAllocation, 1);
  gtk_widget_get_allocation(window.menubar, alloc);
  window.menubar_height = alloc->height;
  window.menubar_width = alloc->width;
  g_free(alloc);

}

#endif // VIEWER_H
