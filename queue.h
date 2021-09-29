#include <stdio.h>
#include <stdlib.h>
#include "main.h"

struct QNode {
    int Cid;
    int active;
    int ts;
    struct QNode* next;
};

struct Queue {
    struct QNode* front, * rear;
};


int getQueueTs(struct Queue*, int);
int getQueueNum(struct Queue*, int);
void setInactive(struct Queue*, int);
int getQueueLastOccuranceNum(struct Queue*, int);

void deQueue(struct Queue*);

int checkActive(struct Queue*, int);

struct QNode* newNode(int, int, int);

struct Queue* createQueue();

void delFromQueue(struct Queue*, int);
void insertToQ(struct Queue*, struct QNode*);

int canGetToSlipkiSec();
int canGetToPralkiSec();
