#include "main.h"
#include "watek_komunikacyjny.h"
#include "watek_glowny.h"
#include "queue.h"
#include <pthread.h>


pthread_t threadKom;
pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lamportMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t timesMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t zidMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t groupMut = PTHREAD_MUTEX_INITIALIZER;

// inicjacja zmiennych z main.h
int B, C;
int Snum, Pnum;
int lamportValue;
int pairId;
int zId;
int zMessagesCount;
char typWatku;
state_t stan;
int size, rank;
MPI_Datatype MPI_PAKIET_T;
struct Queue* WaitQueueZ;
struct Queue* WaitQueueS;
struct Queue* WaitQueueP;
int* otherTimes;
int* pralniaTimes;
int* chosenConans;
int* zIds;


void check_thread_support(int provided)
{
    printf("THREAD SUPPORT: chcemy %d. Co otrzymamy?\n", provided);
    switch (provided) {
    case MPI_THREAD_SINGLE:
        printf("Brak wsparcia dla wątków, kończę\n");
        /* Nie ma co, trzeba wychodzić */
        fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
        MPI_Finalize();
        exit(-1);
        break;
    case MPI_THREAD_FUNNELED:
        printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
        break;
    case MPI_THREAD_SERIALIZED:
        /* Potrzebne zamki wokół wywołań biblioteki MPI */
        printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
        break;
    case MPI_THREAD_MULTIPLE: printf("Pełne wsparcie dla wątków\n"); /* tego chcemy. Wszystkie inne powodują problemy */
        break;
    default: printf("Nikt nic nie wie\n");
    }
}

// wariacja "inicjuj()" dla naszego problemu
void naszInit(int* argc, char*** argv)
{
    int provided;
    MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);

    /* Stworzenie typu */
    const int nitems = 5; /* bo packet_t ma pięć* pól */
    int       blocklengths[5] = { 1,1,1,1,1 };
    MPI_Datatype typy[5] = { MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT };

    MPI_Aint     offsets[5];
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, typ);
    offsets[3] = offsetof(packet_t, zid);
    offsets[4] = offsetof(packet_t, bibid);

    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PAKIET_T);
    MPI_Type_commit(&MPI_PAKIET_T);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(rank);

    WaitQueueZ = createQueue();
    WaitQueueS = createQueue();
    WaitQueueP = createQueue();

    otherTimes = malloc(sizeof(int) * C);
    memset(otherTimes, 0, sizeof(int) * C);
    pralniaTimes = malloc(sizeof(int) * Pnum);
    memset(pralniaTimes, 0, sizeof(int) * Pnum);
    for (int i = 0; i < Pnum; i++) {
        pralniaTimes[i] = -1;
    }
    zIds = malloc(sizeof(int) * B);
    memset(zIds, 0, sizeof(int) * B);
    chosenConans = malloc(sizeof(int) * CONAN_GROUP_SIZE);
    memset(chosenConans, 0, sizeof(int) * CONAN_GROUP_SIZE);

    if (rank < B) {
        typWatku = 'B';
        stan = bOdpoczywa;
    }
    else {
        typWatku = 'C';
        stan = cOdpoczywa;
    }
    pairId = -1;
    zId = -1;
    zMessagesCount = 0;
    
    pthread_create(&threadKom, NULL, startKomWatek, 0);
}

/* usunięcie zamków, czeka, aż zakończy się drugi wątek, zwalnia przydzielony typ MPI_PAKIET_T
   wywoływane w funkcji main przed końcem
*/
void finalizuj()
{
    pthread_mutex_destroy(&stateMut);
    pthread_mutex_destroy(&lamportMut);
    pthread_mutex_destroy(&timesMut);
    pthread_mutex_destroy(&zidMut);
    pthread_mutex_destroy(&groupMut);
    
    /* Czekamy, aż wątek potomny się zakończy */
    printf("[%d] - Czekam na watek komunikacyjny (konczenie).\n", rank );
    pthread_join(threadKom, NULL);
    MPI_Type_free(&MPI_PAKIET_T);
    MPI_Finalize();
}


/* opis patrz main.h */
void sendPacket(packet_t *pkt, int destination, int tag)
{
    int freepkt=0;
    if (pkt==0) { pkt = malloc(sizeof(packet_t)); freepkt=1;}
    pkt->src = rank;
    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    if (freepkt) free(pkt);
}

void sendPacketToAllConans(packet_t *pkt, int tag)
{
    int freepkt=0;
    if (pkt==0) { pkt = malloc(sizeof(packet_t)); freepkt=1;}
    pkt->src = rank;
    for (int i=B; i<B+C; i++) {
        MPI_Send( pkt, 1, MPI_PAKIET_T, i, tag, MPI_COMM_WORLD);
    }
    if (freepkt) free(pkt);
}

void sendPacketToOtherConans(packet_t *pkt, int tag)
{
    int freepkt=0;
    if (pkt==0) { pkt = malloc(sizeof(packet_t)); freepkt=1;}
    pkt->src = rank;
    for (int i=B; i<B+C; i++) {
        if (i != rank) {
            MPI_Send( pkt, 1, MPI_PAKIET_T, i, tag, MPI_COMM_WORLD);
        }
    }
    if (freepkt) free(pkt);
}

int zwiekszLamporta()
{
    pthread_mutex_lock( &lamportMut );
    lamportValue++;
    int tmp = lamportValue;
    pthread_mutex_unlock( &lamportMut );
    return tmp;
}

int zmianaLamporta(int value)
{
    pthread_mutex_lock( &lamportMut );
    lamportValue = value;
    int tmp = lamportValue;
    pthread_mutex_unlock( &lamportMut );
    return tmp;
}

void zmienStan(state_t newState)
{
    pthread_mutex_lock(&stateMut);
    stan = newState;
    pthread_mutex_unlock(&stateMut);
}

void updateTimes(int src, int ts) {
    pthread_mutex_lock( &timesMut );
    otherTimes[src - B] = ts;
    pthread_mutex_unlock( &timesMut );
}

int readTime(int idx) {
    pthread_mutex_lock( &timesMut );
    int time = otherTimes[idx - B];
    pthread_mutex_unlock( &timesMut );
    return time;
}

void updateZIds(int i, int id) {
    if (i >= B) {
        printf("Blad podczas aktualizowania Zids - niepoprawny indeks!\n");
        return; 
    }
    pthread_mutex_lock( &zidMut );
    zIds[i] = id;
    pthread_mutex_unlock( &zidMut );
}

int readZid(int i) {
    pthread_mutex_lock( &zidMut );
    int zid = zIds[i];
    pthread_mutex_unlock( &zidMut );
    return zid;
}

void updateChosenConans(int i, int cid) {
    if (i >= CONAN_GROUP_SIZE) {
        printf("Blad podczas aktualizowania Cid - niepoprawny indeks!\n");
        return; 
    }
    pthread_mutex_lock( &groupMut );
    chosenConans[i] = cid;
    pthread_mutex_unlock( &groupMut );
}

int readChosenConan(int i) {
    pthread_mutex_lock( &groupMut );
    int Cid = chosenConans[i];
    pthread_mutex_unlock( &groupMut );
    return Cid;
}


int main(int argc, char** argv)
{
    if (argc != 3) {
        printf("Uruchomienie zakonczone niepowodzeniem - niepoprawna ilosc parametrow\n");
        return 0;
    }

    B = atoi( argv[1] );
    C = atoi( argv[2] );
    
    Snum = 1;  // definiowanie liczby strojow
    Pnum = 10;  // definiowanie liczby pralek
    lamportValue = 0;  // inicjacja zegaru Lamporta

    naszInit(&argc, &argv);  // tworzy wątek komunikacyjny, robi dodatkowe fajne rzeczy

    if (typWatku == 'B') {
        bibliLoop();
    } else {
        conanLoop();
    }

    finalizuj();
    return 0;
}
