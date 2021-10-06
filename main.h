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
#include <math.h>
#include <time.h> 
/* odkomentować, jeżeli się chce DEBUGI */
//#define DEBUG 
/* boolean */
#define TRUE 1
#define FALSE 0

#define STATE_CHANGE_PROB 75  // procentowa szansa na zmianę stanu przez proces
#define SEC_IN_STATE 1  // czas przez ktory proces przebywa w jednym stanie
#define MSG_TAG 2
#define SEC_IN_PRALKA 5  // czas potrzebny na wypranie stroju
#define CONAN_GROUP_SIZE 3  // liczba conanów, do których bibliotekarz wysyła to samo zlecenie

#define ROOT 0

/* stany procesów */
typedef enum { bOdpoczywa, bTworzyZlecenie, bCzeka, cOdpoczywa, cChceZlecenie, cWaitStroj, cInSecStroj, cPraca, cWaitPranie, cInSecPranie } state_t;
extern state_t stan;
extern int rank;
extern int size;

typedef struct {
    int ts;       /* timestamp (zegar lamporta) */
    int src;      /* id nadawcy wiadomosci */
    int typ;      /* typ nadawanej wiadomosci */
    int zid;      /* identyfikator zlecenia */
    int bibid;    /* identyfikator nadawcy zlecenia */
} packet_t;
extern MPI_Datatype MPI_PAKIET_T;


extern int B, C;  // liczba procesów bibliotekarzy, conanów
extern int Snum, Pnum;  // liczba strojów i pralek
extern int lamportValue;  // wartosc zegara Lamporta

extern int pairId;  // identyfikator drugiej strony zlecenia
extern int zId;  // identyfikator zlecenia o które się ubiegam / które przyjąłem
extern int zMessagesCount;  // licznik wiadomosci zwiazanych ze zleceniem

extern char typWatku;  // 'B' lub 'C'; określa czy wątek jest bibliotekarzem, czy Conanem
extern state_t stan;

extern struct Queue* WaitQueueZ;  // kolejka na żądania związane z przyjmowaniem zleceń
extern struct Queue* WaitQueueS;  // kolejka na żądania dostępu do sekcji krytycznej S
extern struct Queue* WaitQueueP;  // kolejka na żądania dostępu do sekcji krytycznej P
extern int* otherTimes;  // tablica znanych wartości zegarów innych procesów
extern int* pralniaTimes;  // tablica z czasami zakończenia prania strojów danego Conana
extern int* chosenConans;  // tablica z indeksami conanów przypisanych do jednego zlecenia
extern int* zIds;  // tablica z najwyższymi zaobserwowanymi id zleceń wysłanymi przez poszczególnych bibliotekarzy

/*nTypy wiadomości */
#define REQzlecenie 1
#define REQslipki 2
#define REQpralnia 3
#define ACK 4
#define RELEASE 5
#define ZLECENIE 10
#define CHCE_ZLECENIE 11
#define MOZESZ_ZLECENIE 12
#define ZADANIE_PRZYJETE 13
#define ZADANIE_ODRZUCONE 14
#define ZADANIE_ZAKONCZONE 15

/* Funkcje */
/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t* pkt, int destination, int tag);
void sendPacketToAllConans(packet_t *pkt, int tag);
void sendPacketToOtherConans(packet_t *pkt, int tag);
void zmienStan(state_t);
int zwiekszLamporta();
int zmianaLamporta(int value);
void updateTimes(int, int);  // ustaw i-tą pozycję w tablicy zegarów na wartość j
int readTime(int);  // odczytaj wartość i-tej pozycji w tablicy zegarów 
void updateZIds(int i, int id);  // ustaw i-tą pozycję w tablicy indeksów na 'id'
int readZid(int i);  // odczytaj wartość i-tej pozycji w tablicy największych indeksów zleceń
void updateChosenConans(int i, int cid);  // ustaw i-tą pozycję w tablicy conanów na 'cid'
int readChosenConan(int i);  // odczytaj indeks i-tej pozycji w tablicy wybranych conanow

#endif
