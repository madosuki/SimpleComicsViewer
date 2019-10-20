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
#include "mainwindow.h"

#define LIST_BUFFER 1024

int status;

void free_array_with_alloced(void **list, const int size);

int get_image_file_count_from_directory(struct dirent **src, const int size, int *dst);

int create_image_path_list(char **image_path_list);

void set_image_path_list();

void set_image_from_compressed_file(const char* file_name);

void set_image_container(int position);

void set_image(GtkWidget **img, int position);

void resize_when_single(int position);

int init_image_object();

void next_image(int isForward);

// return value is TRUE or FALSE
int resize_when_spread(int page);

void scale_when_oversize(int *x, int *y, int window_width, int window_height, double w_aspect, double h_aspect, int isOverWidth);

// isOverHeight is only accept TRUE or FALSE
void set_margin_left_page(int position, int isOverHeight);

gboolean my_key_press_function(GtkWidget *widget, GdkEventKey *event, gpointer data);

gboolean detect_resize_window(GtkWidget *widget, GdkEvent *event, gpointer data);

void Close();

void FullScreen();

void UpdateGrid();

typedef struct
{
    GtkWidget *left;
    GtkWidget *right;
    int isSingle;
    int page_direction_right;
    int current_page;
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

Image_Container_t **image_container_list;

uncompress_data_set_t *uncompressed_file_list;

DrawingArea_t draw_area;

int isCompressFile;

int isFirstLoad;

GtkWidget *grid;

#endif // VIEWER_H
