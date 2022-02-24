/* servTCPConcTh2.c - Exemplu de server TCP concurent care deserveste clientii
   prin crearea unui thread pentru fiecare client.
   Asteapta un numar de la clienti si intoarce clientilor numarul incrementat.
	Intoarce corect identificatorul din program al thread-ului.
  
   
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

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

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
}thData;

typedef struct thMyData{
	int argc;
	char **argv;
}thMyData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);


GtkApplication *app;

GtkWidget* window;
GtkWidget* grid;
GtkWidget* blank1;      // label
GtkWidget* screen;      // label
GtkWidget* blank2;      // label
GtkWidget* file;        // entry
GtkWidget* add;         // button
GtkWidget* delete;      // button
GtkWidget* blank3;      // label
GtkWidget* format;      // entry
GtkWidget* size;        // entry
GtkWidget* blank4;      // label
GtkWidget* exit_server; // button
GtkWidget* sql;     // button
GtkWidget* server_btn;

/* Exit GUI */
void destroy(GtkWidget* widget, gpointer data)
{
    gtk_main_quit();
}

void init_GUI(int argc, char **argv)
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

  	file = gtk_entry_new();
  		gtk_grid_attach (GTK_GRID (grid), file, 0, 3, 2, 1);
  		gtk_entry_set_placeholder_text(GTK_ENTRY(file), "Insert file name");

    format = gtk_entry_new();
        gtk_grid_attach (GTK_GRID (grid), format, 0, 4, 1, 1);
        gtk_entry_set_placeholder_text(GTK_ENTRY(format), "Insert file format");

    size = gtk_entry_new();
        gtk_grid_attach (GTK_GRID (grid), size, 1, 4, 1, 1);
        gtk_entry_set_placeholder_text(GTK_ENTRY(size), "Insert max size in Mb");

    blank3 = gtk_label_new ("");
        gtk_grid_attach (GTK_GRID (grid), blank3, 0, 5, 2, 1);

  	blank4 = gtk_label_new ("");
  		gtk_grid_attach (GTK_GRID (grid), blank4, 0, 7, 2, 1);

  	exit_server = gtk_button_new_with_label("EXIT");
  		gtk_grid_attach (GTK_GRID (grid), exit_server, 1, 8, 1, 1);
  		g_signal_connect (exit_server, "clicked", G_CALLBACK (destroy), file);

    sql = gtk_button_new_with_label("SQL");
        //gtk_grid_attach (GTK_GRID (grid), sql, 1, 9, 1, 1);
        //g_signal_connect (sql, "clicked", G_CALLBACK (get_file_name), file);
        //g_signal_connect (sql, "clicked", G_CALLBACK (sql_query), file);

    server_btn = gtk_button_new_with_label("server");
        gtk_grid_attach (GTK_GRID (grid), server_btn, 1, 10, 1, 1);
        //g_signal_connect (server_btn, "clicked", G_CALLBACK (server), file);

  	gtk_widget_show_all(window);
  	gtk_main();
}

int main (int argc, char **argv)
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit de trimis la client 
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;
  

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread   
      thMyData * td1;  
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      // client= malloc(sizeof(int));
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	
        /* s-a realizat conexiunea, se astepta mesajul */
    
	// int idThread; //id-ul threadului
	// int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;

	td1=(struct thMyData*)malloc(sizeof(struct thMyData));	
	td1->argc=&argc;
	td1->argv=&argv;

	pthread_create(&th[i], NULL, &treat, td);
	pthread_create(&th[i], NULL, &init_GUI, NULL);	      
				
	}//while    
};				
static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		//pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		close ((intptr_t)arg);
		return(NULL);	
  		
};


void raspunde(void *arg)
{
        int nr, i=0;char name[20];
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	if (read (tdL.cl, &name, strlen(name)) <= 0)
            {
              printf("[Thread %d]\n",tdL.idThread);
              perror ("Eroare la read() de la client.\n");
            
            }
    
    printf ("[Thread %d]Mesajul a fost receptionat...%s\n",tdL.idThread, name);
              
              nr++;      
    printf("[Thread %d]Trimitem mesajul inapoi...%s\n",tdL.idThread, name);
              
              
     if (write (tdL.cl, &name, strlen(name)) <= 0)
        {
         printf("[Thread %d] ",tdL.idThread);
         perror ("[Thread]Eroare la write() catre client.\n");
        }
    else
        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	

}
