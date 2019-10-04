#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gtk/gtk.h>
#include <dirent.h>
#include <math.h>
#include "utils.h"

#define LIST_BUFFER 1024

GtkApplication *app;
int status;

void free_array_with_alloced(void **list, const int size);

int get_image_file_count(struct dirent **src, const int size, int *dst);

int create_image_list(char **image_list);

void set_image_list();

void set_image_container(int position);

gboolean my_key_press_function(GtkWidget *widget, GdkEventKey *event, gpointer data);

void Close();

GtkWidget *image;

typedef struct
{
    int image_count;
    char **image_list;
} DirectoryDetail_t;

DirectoryDetail_t *detail;

typedef struct
{
    GtkWidget *window;
    int width;
    int height;
} window_data_t;

typedef struct
{
    GdkPixbuf *src;
    GdkPixbuf *dst;
    GError *err;
    int src_width;
    int src_height;
    int dst_width;
    int dst_height;
} Image_Container_List_t;

Image_Container_List_t **image_container_list;

window_data_t window;

static void print_hello(GtkWidget *widget, gpointer data)
{
    g_print("Hello World\n");
}

static void activate(GtkApplication* app, gpointer user_data)
{
    // Window settings
    window.window = gtk_application_window_new(app);
    window.width = 1024;
    window.height = 768;
    // g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(Close), NULL);

    // Set Window Title
    gtk_window_set_title(GTK_WINDOW(window.window), "Simple Comix Viewer");
    // Set Window Size
    gtk_window_set_default_size(GTK_WINDOW(window.window), window.width, window.height);

    gtk_widget_add_events(window.window, GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(window.window), "key-press-event", G_CALLBACK(my_key_press_function), NULL);

    // Create Vertical Box
    // GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    // add vbox to container
    // gtk_container_add(GTK_CONTAINER(vbox), hbox);
    // gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_container_add(GTK_CONTAINER(window.window), hbox);

    // Initial Scroll Window
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);

    // set image file
    set_image_list();
    if(detail->image_count > 0)
    {
        image_container_list = (Image_Container_List_t**)calloc(detail->image_count, sizeof(Image_Container_List_t*));

        set_image_container(0);

        image = gtk_image_new_from_pixbuf(image_container_list[0]->dst);

        gtk_container_add(GTK_CONTAINER(scrolled_window), image);
    }


    // GtkWidget *second_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    // gtk_container_add(GTK_CONTAINER(second_scrolled_window), right);

    // concatenate between vbox and scroll window.
    gtk_box_pack_start(GTK_BOX(hbox), scrolled_window, TRUE, TRUE, 0);
    // gtk_box_pack_start(GTK_BOX(hbox), second_scrolled_window, TRUE, TRUE, 0);

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
