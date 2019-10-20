#include "mainwindow.h"

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
    g_signal_connect(G_OBJECT(file_menu_struct.load), "activate", G_CALLBACK(OpenFile), NULL);

    file_menu_struct.quit = gtk_menu_item_new_with_label("Quit");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu_struct.body), file_menu_struct.quit);
    g_signal_connect(G_OBJECT(file_menu_struct.quit), "activate", G_CALLBACK(CloseWindow), NULL);

    // Help Menu
    help_menu_struct.body = gtk_menu_new();
    help_menu_struct.root = gtk_menu_item_new_with_label("Help");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_menu_struct.root), help_menu_struct.body);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help_menu_struct.root);

    help_menu_struct.about = gtk_menu_item_new_with_label("About");
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu_struct.body), help_menu_struct.about);

    return menubar;
}

