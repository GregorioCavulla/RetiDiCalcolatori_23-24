#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define WORD_LENGTH 64  // lunghezza massima di una parola
#define MAX_INPUT 128   // lunghezza massima di un input
#define LINE_LENGTH 256 // lunghezza massima di una linea

int main(int argc, char const *argv[])
{
    /* inizializzazioni variabili per la comunicazione */
    struct hostent *host;
    struct sockaddr_in servAddr;
    int port, sd, nread;
    char datoInput[MAX_INPUT], datoOutput[MAX_INPUT];

    // inizializzazioni variabili
    int ris, fd, numberOfFiles;
    char dirName[WORD_LENGTH], bufferChar, fileName[WORD_LENGTH], response;
    long fileLength;

    /* CONTROLLO ARGOMENTI ---------------------------------- */
    if (argc != 3)
    {
        printf("[ERR]:%s serverAddress serverPort\n", argv[0]);
        exit(1);
    }
    printf("[DEBUG] Client avviato\n");

    /* PREPARAZIONE INDIRIZZO SERVER ----------------------------- */
    memset((char *)&servAddr, 0, sizeof(struct sockaddr_in));
    servAddr.sin_family = AF_INET;
    host = gethostbyname(argv[1]);

    // VERIFICA INTERO porta
    nread = 0;
    while (argv[2][nread] != '\0')
    {
        if ((argv[2][nread] < '0') || (argv[2][nread] > '9'))
        {
            printf("Secondo argomento non intero\n");
            printf("Error:%s serverAddress serverPort\n", argv[0]);
            exit(2);
        }
        nread++;
    }
    port = atoi(argv[2]);
    // Verifico port
    if (port < 1024 || port > 65535)
    {
        printf("%s = porta scorretta...\n", argv[2]);
        exit(2);
    }
    // Verifico host
    if (host == NULL)
    {
        printf("%s not found in /etc/hosts\n", argv[1]);
        exit(2);
    }
    else
    {
        servAddr.sin_addr.s_addr = ((struct in_addr *)(host->h_addr))->s_addr;
        servAddr.sin_port = htons(port);
    }

    // CREAZIONE SOCKET
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
    {
        perror("[ERR] apertura socket ");
        exit(3);
    }
    printf("[DEBUG] Creata la socket sd=%d\n", sd);
    // CONNECT e BIND IMPLICITA
    if (connect(sd, (struct sockaddr *)&servAddr, sizeof(struct sockaddr)) < 0)
    {
        perror("[ERR] Errore in connect");
        exit(4);
    }
    printf("[DEBUG] Connect ok\n");

    /**
     * CICLO INTERAZIONE
     */

    printf("Inserire  ---------- , Ctrl+D(Linux) o Ctrl+Z(Windows)  per terminare: ");

    while (gets(datoInput))
    {
        datoInput[strcspn(datoInput, "\n")] = '\0';
        // invio richiesta
        if (write(sd, datoInput, strlen(datoInput)) < 0)
        {
            perror("write");
            printf("Inserire  ---------- , Ctrl+D(Linux) o Ctrl+Z(Windows)  per terminare: ");
        }

        // ricezione risultato
        if (read(sd, &datoOutput, sizeof(datoOutput)) < 0)
        {
            perror("read");
            printf("Inserire  ---------- , Ctrl+D(Linux) o Ctrl+Z(Windows)  per terminare: ");
        }
    }

    // FINE CICLO INTERAZIONE

    // LIBERO LE RISORSE
    printf("[DEBUG] \nClient: termino...\n");
    shutdown(sd, SHUT_WR);
    shutdown(sd, SHUT_RD);
    close(sd);
    exit(0);
}
