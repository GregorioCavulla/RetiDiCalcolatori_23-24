#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define WORD_LENGTH 64  // lunghezza massima di una parola
#define MAX_INPUT 128   // lunghezza massima di un input
#define LINE_LENGTH 256 // lunghezza massima di una linea
#define QUEUE_TCP 10240 // lunghezza massima della coda di connessioni pendenti
#define N 10            // numero massimo di noleggi

#define max(a, b) ((a) > (b) ? (a) : (b)) // massimo tra due numeri

static int inizializzato = 0;

typedef struct /*da modificare in base alle necessita*/ // struttura per la ricezione di una richiesta UDP
{
    char Identificatore[6];
    char dataString[WORD_LENGTH];
    int giorni;
    char modello[WORD_LENGTH];
    float costo_giornaliero;
    char nome_file_foto[WORD_LENGTH];

} Noleggio;

static Noleggio table[N]; // tabella dei noleggi

void inizializza()
{
    if (inizializzato == 1)
    {
        return;
    }

    inizializzato = 1;
    for (int i = 0; i < N; i++)
    {
        strcpy(table[i].Identificatore, "L");
        snprintf(table[i].dataString, WORD_LENGTH, "%d/%d/%d", -1, -1, -1);
        table[i].giorni = -1;
        strcpy(table[i].modello, "-1");
        table[i].costo_giornaliero = -1;
        strcpy(table[i].nome_file_foto, "-1");
    }

    strcpy(table[0].Identificatore, "ABCDE");
    snprintf(table[0].dataString, WORD_LENGTH, "%d/%d/%d", 8, 12, 2023);
    table[0].giorni = 5;
    strcpy(table[0].modello, "VolkRacetigerSL");
    table[0].costo_giornaliero = 43.5;
    strcpy(table[0].nome_file_foto, "Volkl_Racetiger_SL.jpg");

    strcpy(table[1].Identificatore, "12345");
    snprintf(table[1].dataString, WORD_LENGTH, "%d/%d/%d", 31, 1, 2024);
    table[1].giorni = 3;
    strcpy(table[1].modello, "AtomicRedsterX9");
    table[1].costo_giornaliero = 45.5;
    strcpy(table[1].nome_file_foto, "Atomic_Redster_X9.jpg");

    strcpy(table[2].Identificatore, "AB123");
    snprintf(table[2].dataString, WORD_LENGTH, "%d/%d/%d", 4, 2, 2024);
    table[2].giorni = 2;
    strcpy(table[2].modello, "RossignolHeroEliteST");
    table[2].costo_giornaliero = 40.5;
    strcpy(table[2].nome_file_foto, "Rossignol_Hero_Elite_ST.jpg");

    strcpy(table[3].Identificatore, "CDEFG");
    snprintf(table[3].dataString, WORD_LENGTH, "%d/%d/%d", 15, 3, 2024);
    table[3].giorni = 4;
    strcpy(table[3].modello, "FischerRC4WorldcupSC");
    table[3].costo_giornaliero = 42.5;
    strcpy(table[3].nome_file_foto, "Fischer_RC4_Worldcup_SC.jpg");

    strcpy(table[4].Identificatore, "11111");
    snprintf(table[4].dataString, WORD_LENGTH, "%d/%d/%d", 31, 1, 2024);
    table[4].giorni = 3;
    strcpy(table[4].modello, "SalomonS/MaxBlast");
    table[4].costo_giornaliero = 44.5;
    strcpy(table[4].nome_file_foto, "Salomon_S_Max_Blast.jpg");

    printf("inizializzato\n");

    for (int i = 0; i < N; i++)
    {
        printf("%s|%s|%d|%s|%f|%s\n", table[i].Identificatore, table[i].dataString, table[i].giorni, table[i].modello, table[i].costo_giornaliero, table[i].nome_file_foto);
    }
}

void gestore(int signo)
{
    int stato;
    printf("esecuzione gestore di SIGCHLD \n");
    wait(&stato);
}

int main(int argc, char **argv)
{
    int socket_udp, socket_tcp, socket_conn, port, nfds, nread, serv_len; // variabili per la gestione dei socket, della porta, del numero di file descriptor, del numero di caratteri letti e della lunghezza dell'indirizzo
    struct hostent *hostUDP, *hostTCP;                                    // struttura per la gestione degli host, contiene informazioni come l'indirizzo IP dell'host e il suo nome
    struct sockaddr_in cliAddr, servAddr;                                 // struttura per la gestione degli indirizzi, contiene informazioni come l'indirizzo IP e la porta, sia del client che del server
    char buf[LINE_LENGTH]; /*da modificare in base alle necessita*/       // buffer per la ricezione e l'invio di dati
    Noleggio noleggio; /*da modificare in base alle necessita */          // struttura per la ricezione di una richiesta UDP
    const int on = 1;                                                     // variabile per la gestione delle opzioni del socket
    fd_set rset;                                                          // maschera per la gestione dei file descriptor
    char identificatore[6];

    /*inizializzare i dati necessari ai servizio*/

    /* CONTROLLO ARGOMENTI ---------------------------------- */
    if (argc != 2)
    {
        printf("Error: %s port \n", argv[0]);
        exit(1);
    }
    else
    {
        nread = 0;
        // controllo che il secondo argomento sia un intero
        while (argv[1][nread] != '\0')
        {
            if ((argv[1][nread] < '0') || (argv[1][nread] > '9'))
            {
                printf("Secondo argomento non intero\n");
                exit(2);
            }
            nread++;
        }
        port = atoi(argv[1]);
        if (port < 1024 || port > 65535)
        {
            printf("Porta scorretta...");
            exit(2);
        }
    }

    inizializza(); // inizializzazione dati

    /* inizializzazione indirizzo server */
    memset((char *)&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(port);

    /*
     * creazione socket TCP di ascolto
     *
     *  // q: perchè lo crei due volte?
     *  // a: perchè il primo è sbagliato, non è stato controllato il valore di ritorno
     *  // q: ma non si può includere anche il secondo e il terzo controllo nel primo if?
     *  // a: si, ma è più chiaro così
     *  // q: ok perfetto, grazie
     *  // a: prego
     *
     */

    socket_tcp = socket(AF_INET, SOCK_STREAM, 0); // creazione socket
    if (socket_tcp < 0)
    {
        printf("errore creazione socket TCP");
        exit(3);
    }
    printf("[DEBUG] creata socket di ascolto TCP, fd=%d \n", socket_tcp);

    socket_tcp = socket(AF_INET, SOCK_STREAM, 0);

    if (setsockopt(socket_tcp, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        printf("errore di opzione socket di ascolto TCP");
        exit(3);
    }
    printf("[DEBUG] set opzioni socket di ascolto OK \n");
    if (bind(socket_tcp, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        printf("errore di bind socket di ascolto TCP");
        exit(3);
    }
    printf("[DEBUG] bind socket di ascolto TCP OK \n");
    if (listen(socket_tcp, QUEUE_TCP) < 0)
    {
        printf("errore di listen socket di ascolto TCP");
        exit(3);
    }
    printf("[DEBUG] listen socket di ascolto TCP OK \n");

    /*
     * creazione socket UDP di ascolto
     */
    socket_udp = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_udp < 0)
    {
        printf("errore creazione socket UDP");
        exit(3);
    }
    printf("[DEBUG] creata socket di ascolto UDP, fd=%d \n", socket_udp);

    if (setsockopt(socket_udp, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        printf("errore di opzione socket di ascolto UDP");
        exit(3);
    }
    printf("[DEBUG] set opzioni socket di ascolto OK \n");
    if (bind(socket_udp, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        printf("errore di bind socket di ascolto UDP");
        exit(3);
    }
    printf("[DEBUG] bind socket di ascolto UDP OK \n");

    // gestore signal
    signal(SIGCHLD, gestore);

    // pulizia e settaggio maschera dei file descriptor
    FD_ZERO(&rset);
    nfds = max(socket_tcp, socket_udp) + 1;

    // il server si mette in attesa di richieste
    printf("[DEBUG] Server: mi metto in attesa\n");

    /*
     * ciclo di ricezione richieste
     */
    for (;;)
    {
        FD_SET(socket_udp, &rset);
        FD_SET(socket_tcp, &rset);
        if ((nread = select(nfds, &rset, NULL, NULL, NULL)) < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                printf("select");
                exit(5);
            }
        }

        /*
         *   GESTIONE RICHIESTE UDP
         */
        if (FD_ISSET(socket_udp, &rset))
        {
            serv_len = sizeof(struct sockaddr_in);
            printf("[DEBUG] ricevuta una richiesta di UDP \n");
            /*da modificare in base alle necessita */

            if (recvfrom(socket_udp, &identificatore, sizeof(identificatore), 0, (struct sockaddr *)&cliAddr, &serv_len) < 0)
            { // 1
                perror("recvfrom");
                continue;
            }

            printf("[DEBUG] ricevuto identificatore %s\n", identificatore);

            printf("[DEBUG] operazione Datagram \n");

            hostUDP = gethostbyaddr((char *)&cliAddr.sin_addr, sizeof(cliAddr.sin_addr), AF_INET);
            if (hostUDP == NULL)
            {
                printf("no info client host \n");
            }
            else
            {
                printf("Operazione richiesta da: %s %i\n", hostUDP->h_name, (unsigned)ntohs(cliAddr.sin_port));
            }

            /* CODICE DEL SERVER PER RICHIESTE UDP*/
            int esito = 0;

            for (int i = 0; i < N; i++)
            {
                printf("[DEBUG] %s \n", table[i].Identificatore);
                if (strcmp(table[i].Identificatore, identificatore) == 0)
                {
                    esito = 1;
                    break;
                }
            }

            float costoTotale = 0;

            if (esito >= 1)
            {
                printf("[DEBUG] esito positivo\n");
                printf("[DEBUG] invio esito al client\n");

                if (sendto(socket_udp, &esito, sizeof(esito), 0, (struct sockaddr *)&cliAddr, serv_len) < 0)
                { // 2
                    perror("sendto");
                    continue;
                }

                for (int i = 0; i < N; i++)
                {
                    printf("[DEBUG] %s \n", table[i].Identificatore);
                    if (strcmp(table[i].Identificatore, identificatore) == 0)
                    {
                        costoTotale += table[i].costo_giornaliero * table[i].giorni;
                    }
                }
            }
            else
            {
                printf("[DEBUG] esito negativo\n");
            }

            costoTotale = htonl(costoTotale);

            if (sendto(socket_udp, &costoTotale, sizeof(costoTotale), 0, (struct sockaddr *)&cliAddr, serv_len))
            { // 3
                printf("sendto");
                continue;
            }
            printf("[DEBUG] ho mandato l'costoTotale al client\n ");
        }

        /*
         *  GESTIONE RICHIESTE TCP
         *
         *  server: manda n risposte, manda struct risposta, manda lunghezza file
         *
         */

        if (FD_ISSET(socket_tcp, &rset))
        {
            serv_len = sizeof(struct sockaddr_in);
            printf("[DEBUG] ho ricevuto una richiesta di TCP\n");
            if ((socket_conn = accept(socket_tcp, (struct sockaddr *)&cliAddr, &serv_len)) < 0)
            {
                if (errno == EINTR)
                {
                    perror("Forzo la continuazione della accept");
                    continue;
                }
                else
                    exit(6);
            }
            hostTCP = gethostbyaddr((char *)&cliAddr.sin_addr, sizeof(cliAddr.sin_addr), AF_INET);

            if (hostTCP == NULL)
            {
                printf("no info client host\n");
                close(socket_conn);
                exit(6);
            }
            else
                printf("[DEBUG ]Server (figlio): host client e' %s \n", hostTCP->h_name);
            // processo figlio che gestisce la richiesta
            if (fork() == 0)
            { // server: manda n risposte, manda struct risposta, manda lunghezza file
                close(socket_tcp);
                printf("[DEBUG]Server (figlio): eseguo pid=%i\n", getpid());

                // dichiarazione variabili

                int esito;
                int dato;
                char modello[WORD_LENGTH], buff[255];
                int nRisposte, img_fd, fileLength;

                if (read(socket_conn, &modello, sizeof(modello)) < 0)
                {
                    perror("read");
                    nRisposte = -1;
                    if (write(socket_conn, &nRisposte, sizeof(nRisposte)) < 0)
                    {
                        perror("write");
                        continue;
                    }
                }

                printf("[DEBUG] Server TCP (figlio): ricevuto modello %s\n", modello);

                nRisposte = 0;

                for (int i = 0; i < N; i++)
                {
                    if (strcmp(table[i].modello, modello) == 0)
                    {
                        nRisposte++;
                    }
                }

                printf("[DEBUG] Server TCP (figlio): trovate %d prenotazioni per il modello %s\n", nRisposte, modello);

                if (write(socket_conn, &nRisposte, sizeof(nRisposte)) < 0)
                {
                    perror("write");
                    continue;
                }

                for (int i = 0; i < N; i++)
                {
                    if (strcmp(table[i].modello, modello) == 0)
                    {

                        if (write(socket_conn, &table[i], sizeof(table[i])) < 0)
                        {
                            perror("write");
                            continue;
                        }

                        char path[LINE_LENGTH];
                        strcpy(path, "imgs/");
                        strcat(path, table[i].nome_file_foto);

                        printf("[DEBUG] Server TCP (figlio): invio file %s\n", path);

                        img_fd = open(path, O_RDONLY);

                        if (img_fd < 0)
                        {
                            perror("open");

                            fileLength = -1;
                            if (write(socket_conn, &fileLength, sizeof(fileLength)) < 0)
                            {
                                perror("write");
                                continue;
                            }

                            continue;
                        }

                        fileLength = lseek(img_fd, 0, SEEK_END);
                        lseek(img_fd, 0, SEEK_SET);

                        // filelength = htonl(fileLenght);

                        if (write(socket_conn, &fileLength, sizeof(fileLength)) < 0)
                        {
                            perror("write");
                            continue;
                        }

                        if (write(socket_conn, table[i].nome_file_foto, sizeof(table[i].nome_file_foto)) < 0)
                        {
                            perror("write");
                            continue;
                        }

                        while ((nread = read(img_fd, buff, sizeof(buff))) > 0)
                        {
                            write(socket_conn, buff, sizeof(buff));
                        }
                        close(img_fd);

                        printf("[DEBUG] Server TCP (figlio): inviato file %s\n", path);
                    }
                }

                // write(socket_conn, &zero, 1); invio di un zero binario
                /*
                 *  // q: perchè invii un zero binario?
                 *  // a: per segnalare al client che la trasmissione è terminata
                 *  // q: perchè in questo codice è commentato?
                 *  // a: perchè non è necessario, il client sa che la trasmissione è terminata quando il server chiude la connessione
                 *  // q: ok, grazie
                 *  // a: prego
                 */

                // chiusura di socket connessione
                shutdown(socket_conn, SHUT_RD);
                shutdown(socket_conn, SHUT_WR);
                printf("[DEBUG] Server TCP (figlio): chiudo, pid=%i\n", getpid());
                close(socket_conn);
                exit(1);
            } // figlio
            // padre chiude il socket di connessione
            close(socket_conn);
        }
    }
}