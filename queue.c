#include "queue.h"

int getQueueTs(struct Queue* q, int rank) {
	int num = 0;
	struct QNode* tmp = q->front;
	while (tmp != NULL) {
		if (tmp->key == rank && tmp->active == 1) { return tmp->ts; }
		tmp = tmp->next;
		num += 1;
	}
	return -1;
}

int getQueueNum(struct Queue* q, int rank) {
	int num = 0;
	struct QNode* tmp = q->front;
	while (tmp != NULL) {
		if (tmp->key == rank && tmp->active == 1) { return num; }
		tmp = tmp->next;
		num += 1;
	}
	return -1;
}

int getQueueLastOccuranceNum(struct Queue* q, int rank) {
    int num = -1, i = 0;
    struct QNode* tmp = q->front;
    while (tmp != NULL) {
        if (tmp->key == rank && tmp->active == 1) { num = i; }
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

void delFromQueue(struct Queue* q, int element)
{
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
    //printf("debug-1 %d\n", rank);

    struct QNode* tmp = q->front;
    struct QNode* prev = NULL;
    //printf("debug0 %d\n", in->key);

    if (q->rear == NULL && q->front == NULL) {
        q->rear = in;
        q->front = in;
        return;
    }
    while (tmp != NULL) {
        //printf("debug %d\n", in->key);
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