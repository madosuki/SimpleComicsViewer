#include "mainwindow.h"

int main(int argc, char **argv)
{
    app = gtk_application_new("org.gtk.comics_viewer", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);

    Close();
    g_object_unref(app);

    return status;
}
