#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <gtk/gtk.h>

#define PORT_NUMBER     5000
#define SERVER_ADDRESS  "192.168.2.4"
#define FILENAME        "receive.odt"

int client();

GtkApplication *app;

GtkWidget* window;
GtkWidget* grid;
GtkWidget* blank1;
GtkWidget* screen;
GtkWidget* blank2;
GtkWidget* nodes;
GtkWidget* file;
GtkWidget* format;
GtkWidget* size;
GtkWidget* request;
GtkWidget* blank3;
GtkWidget* exit_client;

char screen_text[128];
int client_socket;
int len;
struct sockaddr_in remote_addr;
char buffer[BUFSIZ];
int transfer_file_size;
FILE *received_file;
int remain_data = 0;
const gchar* file_name;
const gchar* file_format;
const gchar* file_size_gchar;
int max_file_size = 0;
char *ptr;

// gcc `pkg-config --cflags gtk+-3.0` client.c -o client `pkg-config --libs gtk+-3.0`
// ./client

void get_file_name(GtkWidget* widget, GtkWidget* file, gpointer data)
{
    file_name = gtk_entry_get_text(GTK_ENTRY(file));
}

void get_file_format(GtkWidget* widget, GtkWidget* format, gpointer data)
{
    file_format = gtk_entry_get_text(GTK_ENTRY(format));
}

void get_file_size(GtkWidget* widget, GtkWidget* size, gpointer data)
{
    file_size_gchar = gtk_entry_get_text(GTK_ENTRY(size));
    max_file_size = atoi(file_size_gchar);
    //printf("%d\n", max_file_size);
}

void request_service()
{
    client();
}

void destroy(GtkWidget* widget, gpointer data)
{
    gtk_main_quit();
}

void init_GUI (int argc, char **argv)
{
  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 350);
  gtk_window_set_position(GTK_WINDOW (window),GTK_WIN_POS_CENTER_ALWAYS);
  g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);
  gtk_container_set_border_width(GTK_CONTAINER(window), 20);

  grid = gtk_grid_new ();
  gtk_container_add (GTK_CONTAINER (window), grid);
  gtk_grid_set_column_homogeneous(GTK_GRID (grid), TRUE);
  gtk_grid_set_row_homogeneous(GTK_GRID (grid), TRUE);

  blank1 = gtk_label_new ("");
  gtk_grid_attach (GTK_GRID (grid), blank1, 0, 0, 2, 1);

  screen = gtk_label_new ("SCREEN");
  gtk_grid_attach (GTK_GRID (grid), screen, 0, 1, 2, 1);

  blank2 = gtk_label_new ("");
  gtk_grid_attach (GTK_GRID (grid), blank2, 0, 2, 2, 1);

  nodes = gtk_combo_box_text_new();
  gtk_grid_attach (GTK_GRID (grid), nodes, 0, 3, 2, 1);
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(nodes), "NODE 1");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(nodes), "NODE 2");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(nodes), "NODE 3");

  file = gtk_entry_new();
  gtk_grid_attach (GTK_GRID (grid), file, 0, 4, 2, 1);
  gtk_entry_set_placeholder_text(GTK_ENTRY(file), "Insert file name");

  format = gtk_entry_new();
  gtk_grid_attach (GTK_GRID (grid), format, 0, 5, 1, 1);
  gtk_entry_set_placeholder_text(GTK_ENTRY(format), "Insert file format");

  size = gtk_entry_new();
  gtk_grid_attach (GTK_GRID (grid), size, 1, 5, 1, 1);
  gtk_entry_set_placeholder_text(GTK_ENTRY(size), "Insert max size in Mb");

  request = gtk_button_new_with_label("Request");
  gtk_grid_attach (GTK_GRID (grid), request, 0, 6, 2, 1);
  g_signal_connect (request, "clicked", G_CALLBACK (get_file_name), file);
  g_signal_connect (request, "clicked", G_CALLBACK (get_file_format), format);
  g_signal_connect (request, "clicked", G_CALLBACK (get_file_size), size);
  g_signal_connect (request, "clicked", G_CALLBACK (request_service), file);

  blank3 = gtk_label_new ("");
  gtk_grid_attach (GTK_GRID (grid), blank3, 0, 7, 2, 1);

  exit_client = gtk_button_new_with_label("EXIT");
  gtk_grid_attach (GTK_GRID (grid), exit_client, 1, 8, 1, 1);
  g_signal_connect (exit_client, "clicked", G_CALLBACK (destroy), file);

  gtk_widget_show_all(window);
  gtk_main();
}

int client()
{
  int sd;     // descriptorul de socket
  struct sockaddr_in server;  // structura folosita pentru conectare 
      // mesajul trimis
  int nr=0;
  char buf[10];
  int client_socket;
        int len;
        struct sockaddr_in remote_addr;
        char buffer[BUFSIZ];
        int file_size;
        FILE *received_file;
        int remain_data = 0;

  

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
  /* portul de conectare */
  int new_port = 5000;
  server.sin_port = htons (new_port);
  
  /* ne conectam la server */
  while (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      if (errno != ECONNREFUSED)
        return errno;
      else {
        new_port++;
        server.sin_port = htons (new_port);
      }
    }

    printf("connected to port [%d]", new_port);

  /* citirea mesajului 
  printf ("[client]Introduceti un numar: ");
  fflush (stdout);
  int numread;
  numread = read (0, buf, sizeof(buf));
  //nr=atoi(buf);
  buf[numread]=0;
  char name[20] = "";
  name[0] = '\0';
  strcpy(name, buf);
  //scanf("%d",&nr);
  
  printf("[client] Am citit %s\n",name);


  if (write (sd,&name,strlen(name)) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }*/

    //printf("%s : %s : %d\n", file_name, file_format, max_file_size);

   
    /* Receiving file size */
        recv(sd, buffer, BUFSIZ, 0);
        file_size = atoi(buffer);
        printf("\nFile size : %s\n", buffer);

        received_file = fopen(FILENAME, "w");
        if (received_file == NULL)
        {
                fprintf(stderr, "Failed to open file foo --> %s\n", strerror(errno));

                exit(EXIT_FAILURE);
        }

        remain_data = file_size;

        while ((remain_data > 0) && ((len = recv(sd, buffer, BUFSIZ, 0)) > 0))
        {
                fwrite(buffer, sizeof(char), len, received_file);
                remain_data -= len;
                printf("Receive %d bytes and we hope :- %d bytes\n", len, remain_data);
        }
        fclose(received_file);
        close(sd);
  
}

int main (int argc, char **argv)
{
	init_GUI(argc, argv);
	
  	return 0;
}