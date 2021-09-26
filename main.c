//#include "main.h"

#include <pthread.h>

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

void inicjuj(int* argc, char*** argv)
{
    int provided;
    MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);


    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    const int nitems = 3; /* bo packet_t ma trzy pola */
    int       blocklengths[3] = { 1,1,1 };
    MPI_Datatype typy[3] = { MPI_INT, MPI_INT, MPI_INT };

    MPI_Aint     offsets[3];
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, data);

    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PAKIET_T);
    MPI_Type_commit(&MPI_PAKIET_T);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(rank);

    pthread_create(&threadMon, NULL, startKomWatek, 0);
    if (rank < B)
        typ_watku = "B";
    else {
        typ_watku = "C";
        pthread_create(&threadMon, NULL, startKomWatek, 0);
    }
    
    debug("jestem");
}

/* usunięcie zamkków, czeka, aż zakończy się drugi wątek, zwalnia przydzielony typ MPI_PAKIET_T
   wywoływane w funkcji main przed końcem
*/
void finalizuj()
{
    pthread_mutex_destroy(&stateMut);
    /* Czekamy, aż wątek potomny się zakończy */
    println("czekam na wątek \"komunikacyjny\"\n");
    pthread_join(threadKom, NULL);
    if (rank == 0) pthread_join(threadMon, NULL);
    MPI_Type_free(&MPI_PAKIET_T);
    MPI_Finalize();
}


/* opis patrz main.h */
void sendPacket(packet_t* pkt, int destination, int tag)
{
    int freepkt = 0;
    if (pkt == 0) { pkt = malloc(sizeof(packet_t)); freepkt = 1; }
    pkt->src = rank;
    MPI_Send(pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    if (freepkt) free(pkt);
}

void changeTallow(int newTallow)
{
    pthread_mutex_lock(&tallowMut);
    if (stan == InFinish) {
        pthread_mutex_unlock(&tallowMut);
        return;
    }
    tallow += newTallow;
    pthread_mutex_unlock(&tallowMut);
}

void zmienStan(int newState)
{
    pthread_mutex_lock(&stateMut);
    if (stan == InFinish) {
        pthread_mutex_unlock(&stateMut);
        return;
    }
    stan = newState;
    pthread_mutex_unlock(&stateMut);
}

int main(int argc, char** argv)
{
    /* Tworzenie wątków, inicjalizacja itp */
    inicjuj(&argc, &argv); // tworzy wątek komunikacyjny w "watek_komunikacyjny.c"
    tallow = 1000; // by było wiadomo ile jest łoju
    mainLoop();          // w pliku "watek_glowny.c"

    finalizuj();
    return 0;
}
