/*RPC_Client.c*/

#include "RPC_xFile.h"
#include <rpc/rpc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
  CLIENT *cl;   // gestore del trasporto
  char *server; // nome host

  int *ris;    // Output of the RPC if integer
  Output *out; // Output of the RPC if struct

  char car, buffer[128]; // buffer per leggere stringhe

  // dichiarazioni statiche delle variabili per RPC che non possono essere
  // salvate sullo stack locale, fanno riferimento alle strutture dichiarate in
  // RPC_xFile.x
  static Name fileName;
  static Name dirName;

  // CONTROLLO DEGLI ARGOMENTI
  if (argc != 2)
  {
    fprintf(stderr, "uso: %s nomehost\n", argv[0]);
    exit(1);
  }

  // CREAZIONE GESTORE DI TRASPORTO
  cl = clnt_create(argv[1], FILEPROG, FILEVERS, "udp");
  if (cl == NULL)
  {
    clnt_pcreateerror(argv[1]);
    exit(1);
  }

  // INTERAZIONE CON L'UTENTE
  printf("Operazioni: 1=elimina occorrenze numeriche, 2=lista file direttorio; ^D per terminare\n");

  while (gets(buffer))
  {
    // CONTROLLO OPERAZIONE INSERITA
    if (strcmp(buffer, "1") != 0 && strcmp(buffer, "2") != 0)
    {
      printf("Operazione non disponibile, inserire i valori indicati\n");
      printf(
          "Operazioni: 1=elimina occorrenze numeriche, 2=lista file direttorio; ^D per terminare\n");
      continue;
    }
    else if (strcmp(buffer, "1") == 0)
    { // OPERAZIONE 1
      printf("Richiesta operazione: %s\n", buffer);
      // esempio operazione con input fileName e output intero
      printf("inserisci il nome del file: \n");
      gets(fileName.name);

      ris = elimina_occorrenze_numeriche_1(&fileName, cl);

      if (ris < 0)
      {
        clnt_perror(cl, "E' avvenuto un errore lato server");
      }
      else
      {
        printf("Successo: ho eliminato %d caratteri numerici\n", *ris);
      }
    }
    else if (strcmp(buffer, "2") == 0)
    { // OPERAZIONE 2
      printf("Richiesta operazione: %s\n", buffer);
      // esempio operazione con input fileName e output struct
      printf("inserisci il nome del file: \n");
      gets(dirName.name);

      out = lista_file_direttorio_1(&dirName, cl);

      if (out == NULL)
      {
        clnt_perror(cl, "E' avvenuto un errore lato server");
      }
      else if (out->numeroFiles ==
               -1)
      { // parametro impostato dal server per
        //  segnalare errore, e.g. file o direttorio non presenti
        printf("Errore: %s\n", out->files);
      }
      else
      {
        printf("Successo:\n");
        for (int i = 0; i < out->numeroFiles; i++)
        {
          printf("%d) %s\n", i + 1, out->files[i]);
        }
      }
    } // endif
  }

  // DISTRUZIONE GESTORE DI TRASPORTO
  clnt_destroy(cl);
  printf("Client: termino...\n");
  exit(0);
}