#include "main.h"
#include "watek_komunikacyjny.h"

// queue.h nie jest używane
// struct QNode {
//     int key;
//     int active;
//     int ts;
//     struct QNode* next;
// };

// struct Queue {
//     struct QNode* front, * rear;
// };
// struct QNode* newNode(int k, int active, int ts);
// struct Queue* createQueue();
// void enQueue(struct Queue* q, int k, int active, int ts);

// struct QNode* newNode(int k, int a, int ts)
// {
//     struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
//     temp->key = k;
//     temp->next = NULL;
//     temp->active = a;
//     temp->ts = ts;
//     return temp;
// }
// vector<QNode> sQueue;


// void deQueue(struct Queue* q)
// {
//     // If queue is empty, return NULL.
//     if (q->front == NULL)
//         return;

//     // Store previous front and move front one node ahead
//     struct QNode* temp = q->front;

//     q->front = q->front->next;

//     // If front becomes NULL, then change rear also as NULL
//     if (q->front == NULL)
//         q->rear = NULL;

//     free(temp);
// }

// to juz jest w pliku main
// void zmianaLamporta(int value)
// {
//     pthread_mutex_lock(&lamportMut);
//     if (value == -1)
//         lamportValue++;
//     else
//         lamportValue = value;
//     pthread_mutex_unlock(&lamportMut);
//     //nie do końca czaje po co ten mutex
// }

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    packet_t pakiet;
    packet_t* ans = malloc(sizeof(packet_t));
    int maxLamportSeen = 0;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    if (typWatku == 'C') {
        while (1) {
            MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MSG_TAG, MPI_COMM_WORLD, &status);

            switch (pakiet.typ) {
                case ZLECENIE:
                    if (stan == cChceZlecenie) {
                        // sprawdz kolejke
                        ans->data = 1;  // akceptuj
                        ans->typ = ZADANIE_PRZYJETE;
                        pairId = pakiet.src;
                        sendPacket(ans, pakiet.src, MSG_TAG);
                        zmienStan(cWaitStroj);
                    }
                    else {
                        // jestem zajety czyms innym                    
                        ans->data = 0;  // odrzuc
                        ans->typ = ZADANIE_ODRZUCONE;
                        sendPacket(ans, pakiet.src, MSG_TAG);
                    }
                    break;

                case REQzlecenie:
                    //dodaj nadawce tego komunikatu do kolejki zlecen
                    break;

                case ACKzlecenie:
                    //usuń nadawce tego komunikatu z kolejki zlecen
                    break;

                case REQslipki:
                    //dodaj do lokalnej kolejki sQueue // .push_back(status.MPI_SOURCE);
                    //ans->ts = lamportValue; <- robione w funckji sendPacket
                    ans->typ = ACKslipki;
                    sendPacket(ans, pakiet.src, MSG_TAG);
                    break;

                case ACKslipki:
                    if (stan == cWaitStroj)
                    {
                        zwiekszLamporta();
                        //TODO: aktualizuje lamporta pod identyfikatorem nadawcy tej wiadomosci
                        //sTimes[pakiet.src] = pakiet.ts;
                    }
                    else {/*ignore*/ }
                    break;

                case REQpralnia:
                    //dodaj do lokalnej kolejki pQueue // .push_back(status.MPI_SOURCE);
                    //ans->ts = lamportValue; <- robione w funckji sendPacket
                    ans->typ = ACKpralnia;
                    sendPacket(ans, pakiet.src, MSG_TAG);
                    break;

                case ACKpralnia:
                    if (stan == cWaitPranie || stan == cInSecPranie)
                    {
                        zwiekszLamporta();
                        //TODO: aktualizuje lamporta pod identyfikatorem nadawcy tej wiadomosci
                        //pTimes[pakiet.src] = pakiet.ts;
                    }
                    else {/*ignore*/ }
                    break;

                case RELEASE:

                    break;
            }
        }
    }
    else if (typWatku == 'B')
    {
        while(1) {
            MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MSG_TAG, MPI_COMM_WORLD, &status);

            switch (pakiet.typ) {
            case ZADANIE_PRZYJETE:
                if (stan == bTworzyZlecenie) {
                    pairId = pakiet.src;
                    zmienStan(bCzeka);
                } else {
                    // ignoruje
                }
                break;

            case ZADANIE_ODRZUCONE:
                if (stan == bTworzyZlecenie) {
                    // odrzuconeNum++
                    // jesli odrzuconeNum == ilosc odbiorcow, to powroc do odpoczywania
                    zmienStan(bOdpoczywa);
                } else {
                    // ignoruje
                }
                break;

            case ZADANIE_ZAKONCZONE:
                if (stan == bCzeka) {
                    if (pakiet.src == pairId) {
                        pairId = -1;
                        zmienStan(bOdpoczywa);
                    }
                } else {
                    // ignoruje
                }
                break;
            }
        }
    }      
}
