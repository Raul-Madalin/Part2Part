#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <gtk/gtk.h>
#include <sqlite3.h>

// SERVER

#define PORT_NUMBER     5000
#define SERVER_ADDRESS  "192.168.2.4"
#define FILE_TO_SEND    "text.odt"
#define FILENAME        "receive.odt"

int server();
int client();
int add_file();
int delete_file();
int sql_query();
void update_screen();

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
GtkWidget* client_btn;
GtkWidget* nodes;
GtkWidget* node_label;

char screen_text[128];

pthread_mutex_t locked;
  pthread_t black, red, blue, green, white;

  GMutex mutex_interface;

int GUI = 1;
const gchar* file_name;
const gchar* file_format;
const gchar* file_size_gchar;
int file_size;
int file_exists;
int file_available;
int db_size;
int network_node;
int new_request = 1;
int clients = 0;
char global_file_name[20];
char global_file_format[20];
char global_file_size[20];


typedef struct thData {
    int idThread; //id-ul thread-ului tinut in evidenta de acest program
    int cl;             //descriptorul intors de accept
}thData;

// gcc `pkg-config --cflags gtk+-3.0` server.c -o server `pkg-config --libs gtk+-3.0` -lsqlite3 -std=c99
// ./server 

/* Get file name input */
void get_file_name(GtkWidget* widget, GtkWidget* file, gpointer data) {
    file_name = gtk_entry_get_text(GTK_ENTRY(file));
    strcpy(global_file_name, file_name);
}

/* Get file format input */
void get_file_format(GtkWidget* widget, GtkWidget* format, gpointer data) {
    file_format = gtk_entry_get_text(GTK_ENTRY(format));
    strcpy(global_file_format, file_format);
}

/* Get file size input */
void get_file_size(GtkWidget* widget, GtkWidget* size, gpointer data) {
    file_size_gchar = gtk_entry_get_text(GTK_ENTRY(size));
    file_size = atoi(file_size_gchar);
    strcpy(global_file_size, file_size_gchar);
}

void get_network_node(GtkWidget* widget, GtkWidget* file, gpointer data) {
    network_node = atoi(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(nodes)));
    printf("[CLIENT]: %d\n", network_node);
}

/* Server function call */
void file_service(GtkWidget* widget, GtkWidget* file, gpointer data) {
	server();
}

void client_service(GtkWidget* widget, GtkWidget* file, gpointer data) {
    clients++;
    pthread_create(&green, NULL, client, NULL);
    pthread_detach(green);
}

void database_add(GtkWidget* widget, GtkWidget* file, gpointer data) {
    pthread_create(&white, NULL, add_file, NULL);
    pthread_detach(white);
    //pthread_exit(1);
}

void database_delete(GtkWidget* widget, GtkWidget* file, gpointer data) {
    pthread_create(&white, NULL, delete_file, NULL);
    pthread_detach(white);
    //pthread_exit(1);
}


int mycallback(void *NotUsed, int argc, char **argv, 
                    char **azColName) {
    
    NotUsed = 0;
    pthread_mutex_lock(&locked);
    for (int i = 0; i < argc; i++) {
        db_size = atoi(argv[i]);
        //printf("[Callback]: %d | %s\n", db_size, argv[i]);
    }
    pthread_mutex_unlock(&locked);
    printf("\n");
    
    return 0;
}

/* FILE AVALABLE CALLBACK DESTROYED, MUST BE REWRITTEN */
int mycheckcallback(void *NotUsed, int argc, char **argv, 
                    char **azColName) {
    
    NotUsed = 0;
    //pthread_mutex_lock(&locked);
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "1") == 0)
            file_available = TRUE;
        else 
            file_available = FALSE;
    }
    //printf("[SQL]: %d | %d\n", atoi(argv[i]), file_exists);
    //pthread_mutex_unlock(&locked); ????
    printf("\n");
    
    return 0;
}

/* Checks if database entry exists */
int addfilecallback(void *NotUsed, int argc, char **argv, 
                    char **azColName) {
    
    NotUsed = 0;
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "1") == 0)
            file_exists = TRUE;
        else 
            file_exists = FALSE;
    }
    
    printf("\n");
    
    return 0;
}

/* Checks if database entry exists */
int deletefilecallback(void *NotUsed, int argc, char **argv, 
                    char **azColName) {
    
    NotUsed = 0;
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "1") == 0)
            file_exists = TRUE;
        else 
            file_exists = FALSE;
    }
    
    printf("\n");
    
    return 0;
}

/* Add button function */
int add_file() {
    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;

    /* Open database */
    int rc = sqlite3_open("test.db", &db);
    
    if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Cannot open database: %s\n", 
                sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return 1;
    }
    
    /* Check if file exists */
    sprintf(screen_text, "%s", "Checking database...");
    update_screen();

    char sql[128];
    sprintf(sql, "select count(*) from test where name = '%s' and format = '%s';", file_name, file_format);
        
    rc = sqlite3_exec(db, sql, addfilecallback, 0, &err_msg);

    if (file_exists) {
        sprintf(screen_text, "%s", "File found in database...");
        update_screen();

        /* Update availability */
        sprintf(sql, "update test set available = 1 where name = '%s' and format = '%s';", file_name, file_format);
        rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
        sprintf(screen_text, "%s", "Database updated...");
        update_screen();
    }
    else {
        /* Add file to the database */
        sprintf(sql, "select count(*) from test;");
        rc = sqlite3_exec(db, sql, mycallback, 0, &err_msg);

        sprintf(screen_text, "%s", "File not found in database...");
        update_screen();
        sprintf(screen_text, "%s", "Adding file in database...");
        update_screen();

        if (!file_name || !file_format || !file_size) {
            sprintf(screen_text, "%s", "Missing input...");
            update_screen();
        }
        else {
            pthread_mutex_lock(&locked);
            sprintf(sql, "insert into test values(%d, '%s', '%s', %d, 1);", db_size + 1, file_name, file_format, file_size);
            rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
            //printf("[DB]: %d\n", db_size);
            pthread_mutex_unlock(&locked);
            sprintf(screen_text, "%s", "Database updated...");
            update_screen();
        }
        
    }
    
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        
        return 1;
    }
    //sprintf(screen_text, "%s", "Database updated...");
    //update_screen();

    sprintf(screen_text, "%s", "Enter name, format and (optional) size");
    update_screen();

    sqlite3_close(db);
}

/* Delete button function */
int delete_file() {
    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;

    /* Open database */
    int rc = sqlite3_open("test.db", &db);
    sprintf(screen_text, "%s", "Checking database...");
    update_screen();
    
    if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Cannot open database: %s\n", 
                sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return 1;
    }
    /* Check if file exists */
    char sql[128];
    //sprintf(sql, "update test set available = 0 where name = '%s';", file_name);
    sprintf(sql, "select count(*) from test where name = '%s' and format = '%s';", file_name, file_format);
        
    rc = sqlite3_exec(db, sql, deletefilecallback, 0, &err_msg);

    if (file_exists) {
        sprintf(screen_text, "%s", "File found in database...");
        update_screen();

        /* Update availability */
        sprintf(sql, "update test set available = 0 where name = '%s' and format = '%s';", file_name, file_format);
        rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    }
    else {
        sprintf(screen_text, "%s", "File not found in database...");
        update_screen();
        printf("File doesn't exist in the database\n");
    }
    
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        
        return 1;
    }
    sprintf(screen_text, "%s", "Database updated...");
    update_screen();

    //sleep(2);

    sprintf(screen_text, "%s", "Enter name, format and (optional) size");
    update_screen();

    sqlite3_close(db);
    pthread_exit(1);
}

/* General SQL query */
int sql_query() {
    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;

    /* Open database */
    int rc = sqlite3_open("test.db", &db);
    
    if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Cannot open database: %s\n", 
                sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return 1;
    }
    
    /* Enter SQL query */
    char sql[128];
    //sprintf(file_name, "%s", "text");
    //sprintf(sql, "select * from test;");
    sprintf(sql, "select available from test where name = '%s' and format = '%s';", file_name, file_format);
       
    /* For select, use a callback */
    rc = sqlite3_exec(db, sql, mycheckcallback, 0, &err_msg);
    
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        
        return 1;
    }
    
    sqlite3_close(db);
}

/* Exit GUI */
void destroy(GtkWidget* widget, gpointer data) {
    gtk_main_quit();
}

void update_screen() {
    pthread_mutex_lock(&locked);
    gtk_label_set_label(GTK_LABEL(screen), screen_text);
    sleep(2);
    pthread_mutex_unlock(&locked);
}

static void *treat(void * arg) {       
    struct thData tdL; 
    tdL= *((struct thData*)arg);    
    printf ("\n[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush (stdout);         
    pthread_detach(pthread_self());     
    raspunde((struct thData*)arg);
    GUI = 1;
    /* am terminat cu acest client, inchidem conexiunea */
    close ((intptr_t)arg);
    return(NULL);   
        
}

void raspunde(void *arg) {
    int nr, i=0;
    char name[20];
    struct thData tdL; 
    tdL= *((struct thData*)arg);
    ssize_t len;
    int fd;
    int sent_bytes = 0;
    char my_file_size[256];
    struct stat file_stat;
    off_t offset;
    int remain_data;
    char server_file_name[20];
    char server_file_format[20];
    char server_file_size_string[20];
    int server_file_size;
    char final_name[100];
    int file_obtained = 0;
    char new_request[20];
/*
    if (read (tdL.cl, new_request, sizeof(new_request)) <= 0) {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
    }
    printf("FIRST MESSAGE: %s", new_request);
*/
    if (read (tdL.cl, server_file_name, sizeof(server_file_name)) <= 0) {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
    }
    
    if (read (tdL.cl, server_file_format, sizeof(server_file_format)) <= 0) {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
    }
    
    sprintf(final_name, "%s.%s", server_file_name, server_file_format);
    printf ("[Thread %d]: Mesajul a fost receptionat...%s\n",tdL.idThread, final_name);

    if (read (tdL.cl, server_file_size_string, sizeof(server_file_size_string)) <= 0) {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
    }
    
    server_file_size = atoi(server_file_size_string);
    //printf ("[Thread %d]: Mesajul a fost receptionat...%d\n",tdL.idThread, file_size_number);

    /* ONLY CHECKING NAME (NOT SURE), NOT FORMAT */
    
    sprintf(screen_text, "%s", "Checking database...");
    update_screen();
    sql_query();
    //printf("\n%d\n", file_available);

    if (file_available == 1) {

    fd = open(final_name, O_RDONLY);
        if (fd == -1)
        {
                fprintf(stderr, "Error opening file --> %s\n", strerror(errno));
                if (write (tdL.cl, "File doesn't exist", sizeof("File doesn't exist")) <= 0)
                {
                    printf("[Thread %d] ",tdL.idThread);
                    perror ("[Thread]: Eroare la write() catre client.\n");
                }
                sleep(1);
                sprintf(screen_text, "%s", "File not found...");
                update_screen();

                sprintf(screen_text, "%s\n%s",  "Server: Add or delete from database",
                                                "Client: Request file from starting node");
                update_screen();
                return(NULL);
        }
        else {
            sprintf(screen_text, "%s", "Found file...");
            update_screen();

            if (write (tdL.cl, "Found file", sizeof("Found file")) <= 0)
                {
                    printf("[Thread %d] ",tdL.idThread);
                    perror ("[Thread]: Eroare la write() catre client.\n");
                }
            sleep(1);
        }

        /* Get file stats */
        if (fstat(fd, &file_stat) < 0)
        {
                fprintf(stderr, "Error fstat --> %s", strerror(errno));

                exit(EXIT_FAILURE);
        }

        fprintf(stdout, "[Server]: File Size: \n%d bytes\n", file_stat.st_size);

        /* Sending file size */
        do {
            sprintf(screen_text, "%s", "Sending file size...");
            update_screen();

            sprintf(my_file_size, "%d", file_stat.st_size);
            len = send(tdL.cl, my_file_size, sizeof(my_file_size), 0);
            if (len < 0)
            {
                fprintf(stderr, "Error on sending greetings --> %s", strerror(errno));
                exit(EXIT_FAILURE);
            }

            fprintf(stdout, "Server sent %d bytes for the size\n", len);

            offset = 0;
            remain_data = file_stat.st_size;

            sprintf(screen_text, "%s", "Sending file...");
            update_screen();

            /* Sending file data */
            while (((sent_bytes = sendfile(tdL.cl, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0))
            {
                fprintf(stdout, "1. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
                remain_data -= sent_bytes;
                fprintf(stdout, "2. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
            }
        } while (remain_data != 0);
        file_obtained = 1;

    }
    else {
        printf("%s\n", "File not shareable");
        sprintf(screen_text, "%s", "File not shareable...");
        update_screen();
        if (write (tdL.cl, "File not shareable", sizeof("File not shareable")) <= 0)
                {
                    printf("[Thread %d] ",tdL.idThread);
                    perror ("[Thread]Eroare la write() catre client.\n");
                }
    }

    if (file_obtained) {
        sprintf(screen_text, "%s", "File obtained...");
        update_screen();
    }
    else {
        sprintf(screen_text, "%s", "Searching other nodes...");
        update_screen();
        /*
        pthread_mutex_lock(&locked);
        new_request = 0;
        pthread_mutex_unlock(&locked);
        */
        //client();
    }

    

    sprintf(screen_text, "%s\n%s",  "Server: Add or delete from database",
                                    "Client: Request file from starting node");
    update_screen();


    close(tdL.cl);
}

int server()
{
    struct sockaddr_in server;  // structura folosita de server
  struct sockaddr_in from;  
  int nr;       //mesajul primit de trimis la client 
  int sd;       //descriptorul de socket 
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

    int new_port = 5000;
    //
    //server.sin_port = htons (PORT_NUMBER);
    server.sin_port = htons (new_port);
  
  /* atasam socketul */
  while (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
        if (errno != EADDRINUSE)
            return errno;
        else {
            new_port++;
            server.sin_port = htons (new_port);
        }
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }

    sprintf(screen_text, "%s%d", "Port: ", new_port);
    update_screen();

    sprintf(screen_text, "%s\n%s",  "Server: Add or delete from database",
                                    "Client: Request file from starting node");
    update_screen();

  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",new_port);
      fflush (stdout);

      // client= malloc(sizeof(int));
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
    {
      perror ("[server]Eroare la accept().\n");
      continue;
    }
    GUI = 0;
    
        /* s-a realizat conexiunea, se astepta mesajul */
    
    // int idThread; //id-ul threadului
    // int cl; //descriptorul intors de accept

    td=(struct thData*)malloc(sizeof(struct thData));   
    td->idThread=i++;
    td->cl=client;

    pthread_create(&th[i], NULL, &treat, td);         
                
    }//while    
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
        int file_size_c;
        FILE *received_file;
        int remain_data = 0;
        char operation[10];

  printf("[CLIENT]: thread %d\n", clients);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

    int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  //server.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
  /* portul de conectare */

  //pthread_mutex_lock(&locked);
  int new_port;
  if (new_request == 1) {
    printf("[CLIENT]: new_request %d\n", new_request);
    new_port = 4999 + network_node;
    server.sin_port = htons (new_port);
    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1) {
        sprintf(screen_text, "%s%d", "Connecting to port: ", new_port);
        update_screen();
        if (errno != ECONNREFUSED) {
            printf("[CLIENT]: %d\n", errno);
            //pthread_mutex_unlock(&locked);
            return errno;
        }
        else {
            printf("[CLIENT]: ECONNREFUSED\n");
            sprintf(screen_text, "%s", "Port unavailable...");
            update_screen();
            sprintf(screen_text, "%s\n%s",  "Server: Add or delete from database",
                                            "Client: Request file from starting node");
            update_screen();

            //pthread_mutex_unlock(&locked);
            pthread_exit(1);
        }
    }
  }
  else {
    printf("[CLIENT]: new_request %d\n", new_request);
    new_port = 5001;
    server.sin_port = htons (new_port);
    while (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1 && new_port < 5011 /* add avoidance first port*/) {
        sprintf(screen_text, "%s%d", "Connecting to port: ", new_port);
        update_screen();
      if (errno != ECONNREFUSED) {
        //pthread_mutex_unlock(&locked);
        return errno;
    }
      else {
        sprintf(screen_text, "%s", "Port unavailable...");
        update_screen();
        new_port++;
        server.sin_port = htons (new_port);
      }
    }
  }
  if (new_port == 5011) {
    printf("[CLIENT]: not found port\n");
    sprintf(screen_text, "%s\n%s",  "Server: Add or delete from database",
                                    "Client: Request file from starting node");
    update_screen();
    return 0;
  }
  //pthread_mutex_unlock(&locked);
  printf("[CLIENT]: found port\n");
  //sprintf(screen_text, "%s%d", "Connecting to port: ", new_port);
  //update_screen();
  
  /* ne conectam la server */ /*
  while (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1 && new_port < 5010)
    {
      if (errno != ECONNREFUSED)
        return errno;
      else {
        new_port++;
        server.sin_port = htons (new_port);
      }
    }*/
    sprintf(screen_text, "%s%d", "Connecting to port: ", new_port);
    update_screen();
/*
    char temp[20]="TEST REQUEST";
    pthread_mutex_lock(&locked);
    if (new_request == 1) {
        if (write (sd, "new_request", sizeof("new_request")) <= 0)
        {
            perror ("[Client]Eroare la write() spre server.\n");
            return errno;
        }
    }
    else {
        if (write (sd, "old_request", sizeof("old_request")) <= 0)
        {
            perror ("[Client]Eroare la write() spre server.\n");
            return errno;
        }
    }
    */
    if (write (sd, global_file_name, sizeof(global_file_name)) <= 0)
    {
      perror ("[Client]Eroare la write() spre server.\n");
      return errno;
    }
    sleep(1);
    if (write (sd, global_file_format, sizeof(global_file_format)) <= 0)
    {
      perror ("[Client]Eroare la write() spre server.\n");
      return errno;
    }
    sleep(1);
    if (write (sd, global_file_size, sizeof(global_file_size)) <= 0)
    {
      perror ("[Client]Eroare la write() spre server.\n");
      return errno;
    }
    sleep(1);
    pthread_mutex_unlock(&locked);

    //printf("[Client] %s\n", file_name);

    char client_response[20];
    if (read (sd, client_response, sizeof(client_response)) <= 0)
            {
              printf("[Thread %d]\n",sd);
              perror ("Eroare la read() de la client.\n");
            
            }
            else printf("\n[SERVER] : %s\n", client_response);

    if (strcmp(client_response, "Found file") != 0) {
        new_request = 0;
        client();
        close(sd);
    }
    /* Receiving file size */
        recv(sd, buffer, BUFSIZ, 0);
        file_size_c = atoi(buffer);
        //printf("\nFile size : %s\n", buffer);

        received_file = fopen(FILENAME, "w");
        if (received_file == NULL)
        {
                fprintf(stderr, "Failed to open file foo --> %s\n", strerror(errno));

                exit(EXIT_FAILURE);
        }

        remain_data = file_size_c;

        while ((remain_data > 0) && ((len = recv(sd, buffer, BUFSIZ, 0)) > 0))
        {
                fwrite(buffer, sizeof(char), len, received_file);
                remain_data -= len;
                printf("Receive %d bytes and we hope :- %d bytes\n", len, remain_data);
        }
        fclose(received_file);
        close(sd);
  
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
        gtk_grid_attach (GTK_GRID (grid), blank3, 0, 6, 2, 1);

  	add = gtk_button_new_with_label("Add");
  		gtk_grid_attach (GTK_GRID (grid), add, 0, 7, 1, 1);
        g_signal_connect (add, "clicked", G_CALLBACK (get_file_name), file);
        g_signal_connect (add, "clicked", G_CALLBACK (get_file_format), format);
        g_signal_connect (add, "clicked", G_CALLBACK (get_file_size), size);
  		g_signal_connect (add, "clicked", G_CALLBACK (database_add), file);

  	delete = gtk_button_new_with_label("Delete");
  		gtk_grid_attach (GTK_GRID (grid), delete, 1, 7, 1, 1);
        g_signal_connect (delete, "clicked", G_CALLBACK (get_file_name), file);
        g_signal_connect (delete, "clicked", G_CALLBACK (get_file_format), format);
  		g_signal_connect (delete, "clicked", G_CALLBACK (database_delete), file);

  	blank4 = gtk_label_new ("");
  		gtk_grid_attach (GTK_GRID (grid), blank4, 0, 9, 2, 1);

  	exit_server = gtk_button_new_with_label("EXIT");
  		gtk_grid_attach (GTK_GRID (grid), exit_server, 1, 10, 1, 1);
  		g_signal_connect (exit_server, "clicked", G_CALLBACK (destroy), file);

    sql = gtk_button_new_with_label("SQL");
        //gtk_grid_attach (GTK_GRID (grid), sql, 1, 9, 1, 1);
        g_signal_connect (sql, "clicked", G_CALLBACK (get_file_name), file);
        g_signal_connect (sql, "clicked", G_CALLBACK (sql_query), file);

    server_btn = gtk_button_new_with_label("server");
        //gtk_grid_attach (GTK_GRID (grid), server_btn, 1, 10, 1, 1);
        g_signal_connect (server_btn, "clicked", G_CALLBACK (server), file);

    client_btn = gtk_button_new_with_label("Request");
        gtk_grid_attach (GTK_GRID (grid), client_btn, 0, 8, 2, 1);
        g_signal_connect (client_btn, "clicked", G_CALLBACK (get_file_name), file);
        g_signal_connect (client_btn, "clicked", G_CALLBACK (get_file_format), format);
        g_signal_connect (client_btn, "clicked", G_CALLBACK (get_file_size), size);
        g_signal_connect (client_btn, "clicked", G_CALLBACK (get_network_node), nodes);
        g_signal_connect (client_btn, "clicked", G_CALLBACK (client_service), file);

    node_label = gtk_label_new ("NODE:");
        gtk_grid_attach (GTK_GRID (grid), node_label, 0, 5, 1, 1);

    nodes = gtk_combo_box_text_new();
        gtk_grid_attach (GTK_GRID (grid), nodes, 1, 5, 1, 1);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(nodes), "1");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(nodes), "2");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(nodes), "3");
}

void* start_GUI(void* var) {
  gtk_widget_show_all(window);
  gtk_main();
}

gboolean update_gui(gpointer data) {
    //sprintf(screen_text, "%s", "Enter name, format and (optional) size");
    //update_screen();

    while(1) {
        g_mutex_lock(&mutex_interface);
        //printf("GUI:%d\n", GUI);
    if (GUI) {
        gtk_widget_set_sensitive (add, TRUE);
        gtk_widget_set_sensitive (delete, TRUE);
        gtk_widget_set_sensitive (client_btn, TRUE);
    }
    else {
        gtk_widget_set_sensitive (add, FALSE);
        gtk_widget_set_sensitive (delete, FALSE);
        gtk_widget_set_sensitive (client_btn, FALSE);
  }
  g_mutex_unlock(&mutex_interface);
  sleep(1);
}
  return NULL;
}

int main (int argc, char **argv) {
    black = 1;
    red = 2;
    blue = 3;
    green = 4;
    white = 5;

    if(pthread_mutex_init(&locked, NULL) != 0) {
        printf("Mutex creation was unsuccessful !");
        exit(1);
    }

    init_GUI(argc, argv);
    pthread_create(&black, NULL, start_GUI, NULL);
    pthread_create(&red, NULL, update_gui, NULL);
    pthread_create(&blue, NULL, server, NULL);
    pthread_join(black, NULL);
    pthread_detach(red);  
    pthread_detach(blue);

  	return 0;
}
