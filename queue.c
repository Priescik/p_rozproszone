#include "queue.h"

int getQueueTs(struct Queue* q, int rank) {
	int num = 0;
	struct QNode* tmp = q->front;
	while (tmp != NULL) {
		if (tmp->Cid == rank && tmp->active == 1) { return tmp->ts; }
		tmp = tmp->next;
		num += 1;
	}
	return -1;
}

int getQueueNum(struct Queue* q, int rank) {
    // znajdź pozycję mojego pierwszego aktywnego Req w kolejce
	int num = 0;
	struct QNode* tmp = q->front;
	while (tmp != NULL) {
		if (tmp->Cid == rank && tmp->active == 1) { return num; }
		tmp = tmp->next;
		num += 1;
	}
	return -1;
}

void setInactive(struct Queue* q, int rank) {
    // ustaw moje Req jako nieaktywne
    // udało mi się wejść do sekcji krytycznej, ale Req musi nadal pozostać w kolejce
	struct QNode* tmp = q->front;
    while (tmp != NULL) {
        if(tmp->Cid == rank && tmp->active == 1) {
            tmp->active = 0;
            return;
        }
        tmp = tmp->next;
    }
    return;
}

int getQueueLastOccuranceNum(struct Queue* q, int rank) {
    // znajdź pozycję mojego najstarszego aktywnego Req w kolejce
    int num = -1, i = 0;
    struct QNode* tmp = q->front;
    while (tmp != NULL) {
        if (tmp->Cid == rank && tmp->active == 1) { num = i; }
        tmp = tmp->next;
        i += 1;
    }
    return num;
}

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

int checkActive(struct Queue* q, int key) {
    // zwróć '1' jeśli aktywny Req sprawdzanego procesu jest w kolejce wcześniej niż mój
    // zwróć '-1' w przeciwnym wypadku
    struct QNode* tmp = q->front;
    while (tmp != NULL) {
        if (tmp->Cid == rank && tmp->active == 1) { return -1; }
        else if (tmp->Cid == key && tmp->active == 1) { return 1; }
        tmp = tmp->next;
    }
    return -1;
}

struct QNode* newNode(int k, int a, int ts)
{
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->Cid = k;
    temp->next = NULL;
    temp->active = a;
    temp->ts = ts;
    return temp;
}

struct Queue* createQueue()
{
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}

void delFromQueue(struct Queue* q, int element) {
    // usuń wybrane żądanie/element z kolejki
    struct QNode* tmp = q->front;
    struct QNode* prev = NULL;
    while (tmp != NULL) {
        if (tmp->Cid == element) {
            if (prev == NULL) { deQueue(q); return; }
            else if (tmp->next == NULL) {
                q->rear = prev;
                prev->next = NULL;
                free(tmp);
                return;
            }
            else {
                prev->next = tmp->next;
                free(tmp);
                return;
            }

        }
        prev = tmp;
        tmp = tmp->next;
    }
}

void insertToQ(struct Queue* q, struct QNode* in) {
    // wstaw nowe żądanie do kolejki na podstawie jego Ts i Cid
    struct QNode* tmp = q->front;
    struct QNode* prev = NULL;

    if (q->rear == NULL && q->front == NULL) {
        q->rear = in;
        q->front = in;
        return;
    }
    while (tmp != NULL) {
        if (tmp->ts > in->ts || (tmp->ts == in->ts && tmp->Cid > in->Cid)) {
            if (prev == NULL)
            {
                q->front = in;  
                in->next = tmp;
                return;
            }
            else {
                prev->next = in;
                in->next = tmp;
                return;
            }
        }
        prev = tmp;
        tmp = tmp->next;
    }
    tmp = q->rear;
    q->rear = in;
    tmp->next = in;
    in->next = NULL;
    return;
}

int canGetToSlipkiSec() {
    // jeżeli istnieje proces, którego nie ma jeszcze w kolejce sQueue,
    // ale jego ostatnia widziana etykieta czasowa była mniejsza od mojej - zwiększ licznik "unknown"
    // na koniec zwróć moją pozycję w kolejce sQueue powiększoną o licznik "unknown"
    int unknown = 0;
    for (int i = B; i < C+B; i++) {
        int active  = checkActive(WaitQueueS, i);
        printf("\033[0;%dm rank- %d, i- %d, act- %d\n",rank,rank, i, active);
        if(i != rank && active == -1) {
            if((readTime(i) < getQueueTs(WaitQueueS, rank)) || (readTime(i) == getQueueTs(WaitQueueS, rank) && i < rank)) {
                unknown++;
		        printf("\033[0;%dm C- %d, other[i]- %d, ts- %d\n",rank,rank, readTime(i), getQueueTs(WaitQueueS, rank));
            }
        }
    }
    int num = getQueueNum(WaitQueueS, rank);
    printf("\033[0;%dm C- %d, num- %d, other- %d, ts- %d\n",rank,rank, num, unknown, getQueueTs(WaitQueueS, rank));
    num += unknown;
    return num;
}

int canGetToPralkiSec() {
    // jeżeli istnieje proces, którego nie ma jeszcze w kolejce pQueue,
    // ale jego ostatnia widziana etykieta czasowa była mniejsza od mojej - zwiększ licznik "unknown"
    // na koniec zwróć moją pozycję w kolejce pQueue powiększoną o licznik "unknown"
    int unknown = 0;
    for (int i = B; i < C+B; i++) {
        int active  = checkActive(WaitQueueP, i);
        printf("\033[0;%dm rank- %d, i- %d, act- %d\n",rank,rank, i, active);
        if(i != rank && active == -1) {
            if((readTime(i) < getQueueTs(WaitQueueP, rank)) || (readTime(i) == getQueueTs(WaitQueueP, rank) && i < rank)) {
                unknown++;
            }
        }
    }
    int num = getQueueNum(WaitQueueP, rank);
    num += unknown;
    return num;
}

