#include "main.h"
#include "watek_komunikacyjny.h"

struct QNode {
    int key;
    int active;
    int ts;
    struct QNode* next;
};

// The queue, front stores the front node of LL and rear stores the
// last node of LL
struct Queue {
    struct QNode* front, * rear;
};
struct QNode* newNode(int k, int active, int ts);
struct Queue* createQueue();
void enQueue(struct Queue* q, int k, int active, int ts);

struct QNode* newNode(int k, int a, int ts)
{
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->key = k;
    temp->next = NULL;
    temp->active = a;
    temp->ts = ts;
    return temp;
}
vector<QNode> sQueue;


void deQueue(struct Queue* q)
{
    // If queue is empty, return NULL.
    if (q->front == NULL)
        return;

    // Store previous front and move front one node ahead
    struct QNode* temp = q->front;

    q->front = q->front->next;

    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;

    free(temp);
}

void zmianaLamporta(int value)
{
    pthread_mutex_lock(&lamportMut);
    if (value == -1)
        lamportValue++;
    else
        lamportValue = value;
    pthread_mutex_unlock(&lamportMut);
    //nie do końca czaje po co ten mutex
}

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    packet_t* ans = malloc(sizeof(packet_t));
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    if (watek_type == "C") {
        while (stan != InFinish) {
            debug("czekam na recv");
            MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MSG_TAG, MPI_COMM_WORLD, &status);

            switch (pakiet.typ) {
            case ZLECENIE:
                //sprawdza kolejkę
                ans->data = 1;
                sendPacket(ans, pakiet.src, MSG_TAG);

            case REQzlecenie:

                //dodaj na koniec kolejki
                break;

            case ACKzlecenie:
                //usuń C z kolejki
                break;

            case REQslipki:
                sQueue.push_back(status.MPI_SOURCE);
                //MPI_Send(Lamport, 1, MPI_INT, status.MPI_SOURCE, ACKslipki, MPI_COMM_WORLD);
                ans->ts = Lamport;
                ans->typ = ACKslipki;
                sendPacket(ans, pakiet.src, MSG_TAG);
                break;

            case ACKslipki:
                if (stan == CHCE_STROJ)
                {
                    //TODO: actaulize Lamport
                    zmianaLamporta(pakiet->ts); //o to chodzi, no nie?
                    sTimes[pakiet.src] = pakiet.data;
                }
                else {/*ignore*/ }
                break;

            case REQpralnia:
                pQueue.push_back(status.MPI_SOURCE);
                //MPI_Send(Lamport, 1, MPI_INT, status.MPI_SOURCE, ACKpralnia, MPI_COMM_WORLD);
                ans->ts = Lamport;
                ans->typ = ACKslipki;
                sendPacket(ans, pakiet.src, MSG_TAG);
                break;

            case ACKpralnia:
                if (stan == CHCE_PRANIE || stan == ROBI_PRANIE)
                {
                    //TODO aktualizuj zegar
                    zmianaLamporta(buffRecv); //o to chodzi, no nie?
                    pTimes[pakiet.src] = pakiet.ts;
                }//rzeczy
                else {/*ignore*/ }
                break;

                //case RELEASE:
                //	if (stan == CHCE_PRANIE) {/*ignore??????*/ }
                //	else {
                //		for (int i = 0; i < sQueue.size(); i++)
                //			if (sQueue[i] == status.MPI_SOURCE) {
                //				sQueue.remove(i);
                //				break;
                //			}
                //		for (int i = 0; i < pQueue.size(); i++)
                //			if (pQueue[i] == status.MPI_SOURCE) {
                //				pQueue.remove(i);
                //				break;
                //			}
                //	}
                //	break;
                //}
            }
        }
    }
    else
    {//potencjalnie bibliotekarz
    }
}

void 
