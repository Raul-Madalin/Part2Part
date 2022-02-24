#include <gtk/gtk.h>
// gcc `pkg-config --cflags gtk+-3.0` -o graphical_interface graphical_interface.c `pkg-config --libs gtk+-3.0`
// ./graphical_interface 

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
  GtkWidget* label;
  GtkWidget* grid;
  GtkWidget* combobox;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
  gtk_window_set_position(GTK_WINDOW (window),GTK_WIN_POS_CENTER_ALWAYS);
  g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);
  gtk_container_set_border_width(GTK_CONTAINER(window), 20);

  grid = gtk_grid_new ();
  gtk_container_add (GTK_CONTAINER (window), grid);
  gtk_grid_set_column_homogeneous(GTK_GRID (grid), TRUE);
  gtk_grid_set_row_homogeneous(GTK_GRID (grid), TRUE);

  label = gtk_label_new ("Enter some text:" );
  gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);

  entry = gtk_entry_new ();
  gtk_grid_attach (GTK_GRID (grid), entry, 1, 0, 1, 1);

  combobox = gtk_combo_box_text_new();
  gtk_grid_attach (GTK_GRID (grid), combobox, 0, 2, 2, 1);
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combobox), "hello");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combobox), "world");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combobox), "raul");

  button = gtk_button_new_with_label("Click Me!");
  gtk_grid_attach (GTK_GRID (grid), button, 0, 1, 2, 1);
  g_signal_connect (button, "clicked", G_CALLBACK (greet), combobox);

  gtk_widget_show_all(window);
  gtk_main();

  return status;
}