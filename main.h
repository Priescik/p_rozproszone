#ifndef GLOBALH
#define GLOBALH

#define _GNU_SOURCE
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
/* odkomentować, jeżeli się chce DEBUGI */
//#define DEBUG 
/* boolean */
#define TRUE 1
#define FALSE 0

/* używane w wątku głównym, determinuje jak często i na jak długo zmieniają się stany */
#define STATE_CHANGE_PROB 50
#define SEC_IN_STATE 2

#define ROOT 0

/* stany procesu */
typedef enum { InRun, InMonitor, InSend, InFinish } state_t;
extern state_t stan;
extern int rank;
extern int size;

/* Ile mamy łoju na składzie? */
extern int tallow;

/* stan globalny wykryty przez monitor */
extern int globalState;
/* ilu już odpowiedziało na GIVEMESTATE */
extern int numberReceived;

/* to może przeniesiemy do global... */
typedef struct {
    int ts;       /* timestamp (zegar lamporta */
    int src;      /* pole nie przesyłane, ale ustawiane w main_loop */
    int typ;
    int data;     /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
} packet_t;


extern MPI_Datatype MPI_PAKIET_T;

/* Typy wiadomości */
int REQzlecenie = 1;
int ACKzlecenie = 2;
int REQslipki = 3;
int ACKslipki = 4;
int REQpralnia = 5;
int ACKpralnia = 6;
int RELEASE = 7;
int ZLECENIE = 8;
int ZADANIE_PRZYJETE = 9;
int ZADANIE_ZAKONCZONE = 10;


int ODPOCZYNEK = 0;
int CHCE_ZLECENIE = 1;
int CHCE_STROJ = 2;
int BIERZE_STROJ = 3;
int PRACA = 4;
int CHCE_PRANIE = 5;
int ROBI_PRANIE = 6;

int KATALOGOWANIE = 11;
int TWORZENIE_ZLECENIA = 12;
int OCZEKIWANIE_NA_WYKONANIE = 13;

/* macro debug - działa jak printf, kiedy zdefiniowano
   DEBUG, kiedy DEBUG niezdefiniowane działa jak instrukcja pusta

   używa się dokładnie jak printfa, tyle, że dodaje kolorków i automatycznie
   wyświetla rank

   w związku z tym, zmienna "rank" musi istnieć.

   w printfie: definicja znaku specjalnego "%c[%d;%dm [%d]" escape[styl bold/normal;kolor [RANK]
                                           FORMAT:argumenty doklejone z wywołania debug poprzez __VA_ARGS__
                       "%c[%d;%dm"       wyczyszczenie atrybutów    27,0,37
                                            UWAGA:
                                                27 == kod ascii escape.
                                                Pierwsze %c[%d;%dm ( np 27[1;10m ) definiuje styl i kolor literek
                                                Drugie   %c[%d;%dm czyli 27[0;37m przywraca domyślne kolory i brak pogrubienia (bolda)
                                                ...  w definicji makra oznacza, że ma zmienną liczbę parametrów

*/
#ifdef DEBUG
#define debug(FORMAT,...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

#define P_WHITE printf("%c[%d;%dm",27,1,37);
#define P_BLACK printf("%c[%d;%dm",27,1,30);
#define P_RED printf("%c[%d;%dm",27,1,31);
#define P_GREEN printf("%c[%d;%dm",27,1,33);
#define P_BLUE printf("%c[%d;%dm",27,1,34);
#define P_MAGENTA printf("%c[%d;%dm",27,1,35);
#define P_CYAN printf("%c[%d;%d;%dm",27,1,36);
#define P_SET(X) printf("%c[%d;%dm",27,1,31+(6+X)%7);
#define P_CLR printf("%c[%d;%dm",27,0,37);

/* printf ale z kolorkami i automatycznym wyświetlaniem RANK. Patrz debug wyżej po szczegóły, jak działa ustawianie kolorków */
#define println(FORMAT, ...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t* pkt, int destination, int tag);
void changeState(state_t);
void changeTallow(int);
#endif
