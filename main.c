#include "mainwindow.h"

int main(int argc, char **argv)
{
    /*
    if(argc > 2)
    {
        if(!strcmp(argv[1], "--dir"))
        {
            printf("%s\n", argv[2]);
        }
    }
    */

    app = gtk_application_new("org.gtk.comics_viewer", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);

    Close();
    g_object_unref(app);

    return status;
}
