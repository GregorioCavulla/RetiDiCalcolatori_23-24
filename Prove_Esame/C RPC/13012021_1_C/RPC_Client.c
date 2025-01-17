/*RPC_Client.c*/

#include "RPC_xFile.h"
#include <rpc/rpc.h>
#include <stdio.h>

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
  printf("Operazioni: 1= ELIMINA_OCCORRENZE_NUMERICI, 2=LISTA_SOTTODIRETTORI; ^D per terminare\n");

  while (gets(buffer))
  {
    // CONTROLLO OPERAZIONE INSERITA
    if (strcmp(buffer, "1") != 0 && strcmp(buffer, "2") != 0)
    {
      printf("Operazione non disponibile, inserire i valori indicati\n");
      printf(
          "Operazioni: 1= ELIMINA_OCCORRENZE_NUMERICI, 2=LISTA_SOTTODIRETTORI; ^D per terminare\n");
      continue;
    }
    else if (strcmp(buffer, "1") == 0)
    { // ELIMINA_OCCORRENZE_NUMERICI
      printf("Richiesta operazione: %s\n", buffer);
      //  esempio operazione con input fileName e output intero
      printf("inserisci il nome del file: \n");
      gets(fileName.name);

      ris = elimina_occorrenze_numerici_1(&fileName, cl);

      if (ris < 0)
      {
        clnt_perror(cl, "E' avvenuto un errore lato server");
      }
      else
      {
        printf("successo: ho eliminato %d caratteri nel file %s", *ris, fileName);
      }
    } // elimina occorrenze

    else if (strcmp(buffer, "2") == 0)
    { // OPERAZIONE 2
      // printf("Richiesta operazione: %s\n", buffer);
      //   esempio operazione con input fileName e output struct
      printf("inserisci il nome del direttorio: \n");
      gets(dirName.name);

      out = lista_sottodirettori_1(&dirName, cl);
      // output previsto: sottoDirettorioBuono, 1
      if (out == NULL)
      {
        clnt_perror(cl, "E' avvenuto un errore lato server");
        d
      }
      else if (out->numeroDirettori == -1)
      { // parametro impostato dal server per
        //  segnalare errore, e.g. file o direttorio non presenti
        printf("Errore: %s\n", out->files);
      }
      else
      {
        printf("Ho trovato %d sottodirettori di %s che contengono almeno 5 file di testo\n", out->numeroDirettori, dirName.name);
        for (int i = 0; i < out->numeroDirettori; i++)
        {
          printf("%s\n", out->files[i].name);
        }
      }
    } // endif
  }

  // DISTRUZIONE GESTORE DI TRASPORTO
  clnt_destroy(cl);
  printf("Client: termino...\n");
  exit(0);
}