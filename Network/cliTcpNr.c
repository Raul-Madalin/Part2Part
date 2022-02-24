/* cliTCPIt.c - Exemplu de client TCP
   Trimite un nume la server; primeste de la server "Hello nume".
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#define PORT_NUMBER     5000
#define SERVER_ADDRESS  "192.168.2.9"
#define FILENAME        "test.odt"
/* codul de eroare returnat de anumite apeluri */
extern int errno;

void client()
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
  server.sin_port = htons (PORT_NUMBER);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

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
    }
  if (read (sd, &name,sizeof(name)) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }
  printf ("[client]Mesajul primit este: %s\n", name);*/

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

int main ()
{
  client();
}
