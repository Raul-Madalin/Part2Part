// C program to illustrate
// non I/O blocking calls
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <gtk/gtk.h>
#include <fcntl.h> // library for fcntl function
#define MSGSIZE 6

char* msg1 ="hello";
char* msg2 ="bye !!"; 

int GUI = 1;

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
GtkWidget* trial;
GtkWidget* trial2;

void activate(GtkWidget* trial2, gpointer* data)
{
    printf("Ready to activate button\n");
    gtk_widget_set_sensitive(trial2, TRUE);
}

void deactivate(GtkWidget* trial2, gpointer* data)
{
    printf("Ready to deactivate button\n");
    gtk_widget_set_sensitive(trial2, FALSE);
    sleep(2);
    gtk_widget_set_sensitive(trial2, TRUE);
}

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

    trial = gtk_button_new_with_label("TRIAL");
        gtk_grid_attach (GTK_GRID (grid), trial, 1, 9, 1, 1);
        g_signal_connect (trial, "clicked", G_CALLBACK (activate), trial);

    trial2 = gtk_button_new_with_label("TRIAL2");
        gtk_grid_attach (GTK_GRID (grid), trial2, 1, 10, 1, 1);
        g_signal_connect (trial2, "clicked", G_CALLBACK (deactivate), trial2);
/*
    server_btn = gtk_button_new_with_label("server");
        gtk_grid_attach (GTK_GRID (grid), server_btn, 1, 10, 1, 1);
        g_signal_connect (server_btn, "clicked", G_CALLBACK (server), file);*/
}

void start_GUI (int argc, char **argv) {
    gtk_widget_show_all(window);
    gtk_main();
}

int p[2];
int main(int argc, char **argv)
{
    int i;
            init_GUI(argc, argv);
 
    // error checking for pipe
    if (pipe(p) < 0)
        exit(1);
 
    // error checking for fcntl
    if (fcntl(p[0], F_SETFL, O_NONBLOCK) < 0)
        exit(2);
 
    switch (fork()) {
        case -1:
            exit(5);

        case 0:
            start_GUI(argc, argv);
            break;

        default:
            // continued
            switch (fork()) {
 
            // error
            case -1:
                exit(3);
 
            // 0 for child process
            case 0:
                child_write();
                break;
 
            default:
                parent_read();
                break;
        }
    }

    
    return 0;
}

void FUCK(GtkWidget* trial2, gpointer data) {
    gtk_button_clicked(trial2);
}

void parent_read()
{
    int nread;
    char buf[MSGSIZE];
 
    // write link
    close(p[1]);
 
    while (1) {
 
        // read call if return -1 then pipe is
        // empty because of fcntl
        nread = read(p[0], buf, MSGSIZE);
        switch (nread) {
        case -1:
 
            // case -1 means pipe is empty and errono
            // set EAGAIN
            if (errno == EAGAIN) {
                printf("GUI %d\n", GUI);
                sleep(1);
                break;
            }
 
            else {
                perror("read");
                exit(4);
            }
 
        // case 0 means all bytes are read and EOF(end of conv.)
        case 0:
            printf("End of conversation\n");
 
            // read link
            close(p[0]);
 
            exit(0);
        default:
 
            // text read
            // by default return no. of bytes
            // which read call read at that time
            //printf("GUI paused: %s\n", buf);
            if (strcmp(buf, "Pause") == 0) {
                //G_CALLBACK (deactivate);
                //deactivate(trial2, NULL);
                //FUCK(GTK_BUTTON(trial2), NULL);
                //gtk_widget_set_sensitive(GTK_BUTTON(trial2), FALSE);
                GUI = 0;
            }
            if (strcmp(buf, "Resume") == 0) {
                //show_GUI(server_btn, NULL);
                GUI = 1;
            }
            //G_CALLBACK (show_GUI);
        }
    }
}
void child_write()
{
    int i;
 
    // read link
    close(p[0]);
 
    // write 3 times "hello" in 3 second interval
    while(1) {
        sleep(3);
        write(p[1], "Pause", MSGSIZE);
        sleep(3);
        write(p[1], "Resume", MSGSIZE);
    }
 
    // write "bye" one times
    write(p[1], msg2, MSGSIZE);
 
    // here after write all bytes then write end
    // doesn't close so read end block but
    // because of fcntl block doesn't happen..
    exit(0);
}