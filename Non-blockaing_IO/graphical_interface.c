#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <gtk/gtk.h>
// gcc `pkg-config --cflags gtk+-3.0` -o graphical_interface graphical_interface.c `pkg-config --libs gtk+-3.0`
// ./graphical_interface 

GtkApplication *app;
  int status;
  int GUI = 1;

  GtkWidget* window;
  GtkWidget* button;
  GtkWidget* table;
  GtkWidget* entry;
  GtkWidget* label;
  GtkWidget* grid;
  GtkWidget* combobox;
  GtkWidget* button2;

  pthread_mutex_t locked;
  pthread_t black, red, blue;

void destroy(GtkWidget* widget, gpointer data)
{
    gtk_main_quit();
}

GMutex mutex_interface;

gboolean update_gui(gpointer data) {
    while(1) {
  g_mutex_lock(&mutex_interface);
  printf("GUI:%d\n", GUI);
  if (GUI) {
    gtk_widget_set_sensitive (button, TRUE);
  }
  else gtk_widget_set_sensitive (button, FALSE);
  g_mutex_unlock(&mutex_interface);
  sleep(1);
}
  return NULL;
}

void change(GtkButton * button, gpointer data) {

    g_mutex_lock(&mutex_interface);
  if (GUI) {
    GUI = 0;
  }
  else GUI = 1;
  g_mutex_unlock(&mutex_interface);
}

void init_GUI(int argc, char **argv) {
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

  button = gtk_button_new_with_label("EXIT");
  gtk_grid_attach (GTK_GRID (grid), button, 0, 1, 2, 1);
  g_signal_connect (button, "clicked", G_CALLBACK (destroy), combobox);

   button2 = gtk_button_new_with_label("Click Me!");
  gtk_grid_attach (GTK_GRID (grid), button2, 0, 3, 2, 1);
  g_signal_connect (button2, "clicked", G_CALLBACK (change), combobox);
}

void* start_GUI(void* var) {
  gtk_widget_show_all(window);
  gtk_main();
}

void* test (void* var) {
    while(1) {
    sleep(2);
    printf("myGUI:%d\n", 2);
    if (GUI) {
    GUI = 0;
    }
    else GUI = 1;
    }
    return NULL;
}

int main (int argc, char **argv)
{
    black = 1;
    red = 2;
    blue = 3;

    if(pthread_mutex_init(&locked, NULL) != 0) {
        printf("Mutex creation was unsuccessful !");
        exit(1);
    }

    init_GUI(argc, argv);
    pthread_create(&black, NULL, start_GUI, NULL);
    pthread_create(&red, NULL, update_gui, NULL);
    pthread_create(&blue, NULL, test, NULL);
    pthread_join(black, NULL);
    pthread_detach(red);  
    pthread_detach(blue);

  return status;
}