#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

GtkApplication *app;

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

void unref_g_object(GtkWidget *object);

void next_image(int isForward);

// return value is TRUE or FALSE
int resize_when_spread(int page);

void scale_when_oversize(int *x, int *y, int window_width, int window_height, double w_aspect, double h_aspect, int isOverWidth);

// isOverHeight is only accept TRUE or FALSE
void set_margin_left_page(int position, int isOverHeight);

gboolean my_key_press_function(GtkWidget *widget, GdkEventKey *event, gpointer data);

gboolean detect_resize_window(GtkWidget *widget, GdkEvent *event, gpointer data);

GtkWidget *create_menu_bar();

void Close();

void FullScreen();

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
    GtkWidget *window;
    GtkWidget *menubar;
    int width;
    int height;
    int isFullScreen;
    int isClose;
} main_window_data_t;

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
    GtkWidget *body;
    GtkWidget *root;
    GtkWidget *load;
    GtkWidget *quit;
} file_menu_t; 


Image_Container_t **image_container_list;

uncompress_data_set_t *uncompressed_file_list;

main_window_data_t window;

DrawingArea_t draw_area;

GtkWidget *grid;

file_menu_t file_menu_struct;

int isCompressFile;

static void CloseWindow()
{
    Close();
    gtk_window_close(GTK_WINDOW(window.window));

    printf("CloseWindow\n");
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

    // Create Vertical Box
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    // add vbox to container
    // gtk_container_add(GTK_CONTAINER(vbox), hbox);
    // gtk_container_add(GTK_CONTAINER(window), vbox);

    gtk_container_add(GTK_CONTAINER(window.window), vbox);

    // settings menubar
    window.menubar = create_menu_bar();

    // gtk_menu_attach(GTK_MENU(help), about, 0, 1, 0, 1);

    // gtk_container_add(GTK_CONTAINER(window.window), menubar);

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

    isCompressFile = TRUE;

    // set_image_from_compressed_file("./tmp.zip");

    // set image file
    if(init_image_object())
    {
        if(pages->isSingle)
        {
            gtk_widget_set_hexpand(pages->left, TRUE);
            gtk_grid_attach(GTK_GRID(grid), pages->left, 0, 0, 1, 1);
            // gtk_grid_attach(GTK_GRID(grid), pages->left, 1, 0, image_container_list[0]->dst_width, image_container_list[0]->dst_height);
            // gtk_container_add(GTK_CONTAINER(scroll_window), pages->left);
        }
        else
        {
            // gtk_widget_set_hexpand(pages->left, TRUE);
            gtk_widget_set_vexpand(pages->left, TRUE);

            // gtk_widget_set_hexpand(pages->right, TRUE);
            gtk_widget_set_vexpand(pages->right, TRUE);

            gtk_grid_attach(GTK_GRID(grid), pages->left, 0, 0, 1, 1);
            // gtk_grid_attach(GTK_GRID(grid), pages->right, 1, 0, 1, 1);

            gtk_grid_attach_next_to(GTK_GRID(grid), pages->right, pages->left, GTK_POS_RIGHT, 1, 1);

            // g_object_set(pages->left, "valign", GTK_ALIGN_CENTER, NULL);
            // g_object_set(pages->right, "valign", GTK_ALIGN_CENTER, NULL);

            // gtk_grid_attach(GTK_GRID(grid), pages->left, 0, 0, image_container_list[0]->dst_width, image_container_list[0]->dst_height);
            // gtk_grid_attach(GTK_GRID(grid), pages->right, image_container_list[0]->dst_width, 0, image_container_list[1]->dst_width, image_container_list[1]->dst_height);


        }
    }

    // kg_signal_connect(draw_area.scrolled_window, "size-allocate", G_CALLBACK(get_widget_size), NULL);
    // concatenate between vbox and scroll window.

    // gtk_box_pack_start(GTK_BOX(hbox), second_scroll_window, TRUE, TRUE, 0);

    /*
    // Create Button box
    GtkWidget *button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);

    // Create Button
    GtkWidget *button = gtk_button_new_with_label("Hello World");

    // Set event when click of button
    g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);

    // Swap
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(print_hello), NULL);

    // Add
    gtk_container_add(GTK_CONTAINER(button_box), button);

    unsigned int padding = 0;
    gtk_box_pack_start(GTK_BOX(vbox), button_box, TRUE, TRUE, padding);
    */

    // gtk_box_set_spacing(GTK_BOX(vbox), 100);

    gtk_widget_show_all(window.window);
}

#endif // MAINWINDOW_H
