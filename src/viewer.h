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

#include "utils.h"
#include "loader.h"

#include "pdf_loader.h"

#define LIST_BUFFER 1024
#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 960

int status;

char *arg_file_name;

const char *right_to_left_name;
const char *left_to_right_name;

GtkWidget *change_direction_button;

void *cursor_observer_in_fullscreen_mode(void *data);
pthread_t thread_of_curosr_observer;

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

gboolean my_detect_click_function(GtkWidget *widget, GdkEventButton *event, gpointer data);


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
  int isPriorityToFrontCover;
  int isCover;
  int isAcceptOverflow;
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

typedef struct
{
  guint x;
  guint y;
} Cursor_Position_t;

Cursor_Position_t cursor_pos;

Image_button_t image_button;

Image_Container_t **image_container_list;

uncompress_data_set_t *uncompressed_file_list;

DrawingArea_t draw_area;

int isCompressFile;

int isPDFfile;

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
  int button_menu_width;
  int button_menu_height;
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
  GtkWidget *set_covermode;
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

GtkWidget *button_menu;


static void change_covermode()
{
  if(pages->isPriorityToFrontCover) {
    pages->isPriorityToFrontCover = FALSE;
    pages->isCover = FALSE;
  } else {
    pages->isPriorityToFrontCover = TRUE;
    pages->isCover = FALSE;
  }
  
  if(pages != NULL) {
    if(pages->isPriorityToFrontCover) {
  
      if(pages->current_page != 0 && pages->current_page % 2 == 0) {
        pages->current_page++;
      }

      if(pages->current_page == 0) {
        pages->current_page = 1;
      }

      update_page(FALSE);

    } else {
  
      if(pages->current_page > 0 && pages->current_page % 2 == 0) {
        pages->current_page--;
      }

      if(pages->current_page == 0) {
        pages->isCover = TRUE;
      } 

      update_page(FALSE);
    }
  }
}

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
  if(pages->page_direction_right) {
    pages->page_direction_right = FALSE;
    gtk_button_set_label(GTK_BUTTON(change_direction_button), left_to_right_name);
  } else {
    pages->page_direction_right = TRUE;
    gtk_button_set_label(GTK_BUTTON(change_direction_button), right_to_left_name);
  }

  if(pages->left != NULL) {
    update_page(FALSE);
  }
  
}

static int open_file(const char *file_name)
{
  char* tmp;
  int is_dir = FALSE;

  /*
    int check_compress_file = detect_compress_file(file_name);
    if(check_compress_file) {
    isCompressFile = TRUE;
    } else {
    int check = detect_image_from_file(file_name);

    if(check) {
      
    tmp = get_directory_path_from_filename(file_name);
    if(tmp != NULL) {
    is_dir = TRUE;
    }
      
    } else {

    if(test_open_pdf(file_name)) {
    isPDFfile = TRUE;
    } else {
    return FALSE;
    }

    }

    isCompressFile = FALSE;

    // printf("%s\n", file_name);
    }
  */

  int check = detect_image_from_file(file_name);
  if(check) {
    tmp = get_directory_path_from_filename(file_name);
    if(tmp != NULL) {
      is_dir = TRUE;
      isCompressFile = FALSE;
      isPDFfile = FALSE;
    }
  } else {
    if(test_open_pdf(file_name)) {
      is_dir = FALSE; 
      isPDFfile = TRUE;
      isCompressFile = FALSE;
    } else {
      if(detect_compress_file(file_name)) {
        is_dir = FALSE;
        isPDFfile = FALSE;
        isCompressFile = TRUE;
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
    update_grid();
  } else {
    printf("init image error\n");
    return FALSE;
  }

  return TRUE;
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

  if(pages != NULL) {

    GdkScrollDirection direction;
    gboolean result = gdk_event_get_scroll_direction(event, &direction);


    if(direction == GDK_SCROLL_UP) {
      if(pages->page_direction_right) {
        move_right();
      } else {
        move_left();
      }
    } else if(direction == GDK_SCROLL_DOWN) {
      if(pages->page_direction_right) {
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

  /* gtk_widget_add_events(window.window, GDK_TOUCH_MASK); */
  /* g_signal_connect(G_OBJECT(window.window), "touch-event", G_CALLBACK(my_detect_touch_function), NULL); */


  isFirstLoad = TRUE;

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
  pages = (Pages*)malloc(sizeof(Pages));
  memset(pages, 0, sizeof(Pages));

  pages->page_direction_right = TRUE;
  pages->isSingle = FALSE;
  pages->isFinalPage = FALSE;
  pages->isCover = FALSE;
  pages->isPriorityToFrontCover = FALSE;
  pages->isAcceptOverflow = FALSE;
  pages-> current_page = -1;

  isCompressFile = TRUE;

  isPDFfile = FALSE;

  gtk_widget_show_all(window.window);

  // get menubar size;
  GtkAllocation *alloc = g_new(GtkAllocation, 1);
  gtk_widget_get_allocation(window.menubar, alloc);
  window.menubar_height = alloc->height;
  window.menubar_width = alloc->width;
  g_free(alloc);

}

#endif // VIEWER_H
