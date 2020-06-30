#include "viewer.h"

int main(int argc, char **argv)
{
  // app = gtk_application_new("org.gtk.comics_viewer", G_APPLICATION_FLAGS_NONE);

  //declared variable from src/viewer.h
  app = gtk_application_new("org.gtk.comics_viewer", G_APPLICATION_HANDLES_COMMAND_LINE);
  
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  g_signal_connect(G_APPLICATION(app), "command-line", G_CALLBACK(run_cmd_argument), NULL);

  GOptionEntry entries[] = {{"file", 'f', 0, G_OPTION_ARG_FILENAME, &arg_file_name, "Open File", "zip or jpg or png"}};
  g_application_add_main_option_entries(G_APPLICATION(app), entries);

  // declared variable from src/viewer.h
  status = g_application_run(G_APPLICATION(app), argc, argv);

  close_variables();

  if(app != NULL) {
    g_object_unref(app);
  }


  return status;
}
