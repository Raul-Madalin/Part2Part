#include <gtk/gtk.h>
// gcc `pkg-config --cflags gtk+-3.0` -o server_graphical_interface  server_graphical_interface .c `pkg-config --libs gtk+-3.0`
// ./server_graphical_interface 

void greet(GtkWidget* widget, /*GtkWidget* entry,*/GtkWidget* combobox, gpointer data)
{
    // use only one per button
    //const gchar* text1 = gtk_entry_get_text(GTK_ENTRY(entry));
    const gchar* text2 = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combobox));
    g_print("%s\n", text2);
    //gtk_entry_set_text(GTK_ENTRY(entry), "");
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(combobox));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combobox), "hello");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combobox), "world");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combobox), "raul");
}

void destroy(GtkWidget* widget, gpointer data)
{
    gtk_main_quit();
}

int main (int argc, char **argv)
{
  GtkApplication *app;
  int status;

  GtkWidget* window;
  GtkWidget* button;
  GtkWidget* table;
  GtkWidget* entry;
  GtkWidget* screen;
  GtkWidget* grid;
  GtkWidget* combobox;
  GtkWidget* exit;
  GtkWidget* scan;
  GtkWidget* accept;
  GtkWidget* decline;
  GtkWidget* blank1;
  GtkWidget* blank2;
  GtkWidget* blank3;
  GtkWidget* file;
  GtkWidget* add;
  GtkWidget* delete;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 450);
  gtk_window_set_position(GTK_WINDOW (window),GTK_WIN_POS_CENTER_ALWAYS);
  g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);
  gtk_container_set_border_width(GTK_CONTAINER(window), 20);

  grid = gtk_grid_new ();
  gtk_container_add (GTK_CONTAINER (window), grid);
  gtk_grid_set_column_homogeneous(GTK_GRID (grid), TRUE);
  gtk_grid_set_row_homogeneous(GTK_GRID (grid), TRUE);

  screen = gtk_label_new ("SCREEN");
  gtk_grid_attach (GTK_GRID (grid), screen, 0, 0, 2, 1);

  blank1 = gtk_label_new ("");
  gtk_grid_attach (GTK_GRID (grid), blank1, 0, 1, 2, 1);

  accept = gtk_button_new_with_label("Accept");
  gtk_grid_attach (GTK_GRID (grid), accept, 0, 2, 1, 1);
  g_signal_connect (accept, "clicked", G_CALLBACK (greet), combobox);

  decline = gtk_button_new_with_label("Decline");
  gtk_grid_attach (GTK_GRID (grid), decline, 1, 2, 1, 1);
  g_signal_connect (decline, "clicked", G_CALLBACK (greet), combobox);

  blank2 = gtk_label_new ("");
  gtk_grid_attach (GTK_GRID (grid), blank2, 0, 4, 2, 1);

  file = gtk_entry_new();
  gtk_grid_attach (GTK_GRID (grid), file, 0, 6, 2, 1);
  gtk_entry_set_placeholder_text(GTK_ENTRY(file), "Insert file name");

  add = gtk_button_new_with_label("Add");
  gtk_grid_attach (GTK_GRID (grid), add, 0, 7, 1, 1);
  g_signal_connect (add, "clicked", G_CALLBACK (greet), combobox);

  delete = gtk_button_new_with_label("Delete");
  gtk_grid_attach (GTK_GRID (grid), delete, 1, 7, 1, 1);
  g_signal_connect (delete, "clicked", G_CALLBACK (greet), combobox);

  blank3 = gtk_label_new ("");
  gtk_grid_attach (GTK_GRID (grid), blank3, 0, 8, 2, 1);

  exit = gtk_button_new_with_label("EXIT");
  gtk_grid_attach (GTK_GRID (grid), exit, 1, 9, 1, 1);
  g_signal_connect (exit, "clicked", G_CALLBACK (destroy), combobox);

  gtk_widget_show_all(window);
  gtk_widget_hide(accept);
  gtk_widget_show(accept);
  gtk_main();

  return status;
}