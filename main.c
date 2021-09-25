
#include<stdio.h>
#include<stdlib.h>

#include<sys/ipc.h>
#include<sys/shm.h>
#include<limits.h>

#define STOP 1
#define GO 0
#define TRUE 1
#define FALSE 0

struct  Node{
    int dado;
    struct Node *next;
};


struct Queue{
    struct Node *start;
    struct Node *end;
    int semaforo;
};

typedef struct Node Node;
typedef struct Queue Queue;

Node *createNode(int data){
    Node *node;
    node = (Node *)malloc(sizeof(Node));
    node->dado = data;
    node->next = (Node *)NULL;

    return node;
}

void createQueue(Queue *queue){
    queue->start = (Node*)NULL;
    queue->end = queue->start;
    queue->semaforo = GO;
}

int emptyQueue(Queue *q){
    if(q->start == q->end && q->start == NULL && q->end == NULL)
        return TRUE;
    return FALSE;
}

void enQueue(Node *node, Queue *q){
    if(emptyQueue(q)){
        q->start = node;
        q->end = node;
    }
    else{
        q->end->next = node;
        q->end = node;
    }
}

int unQueue(Queue *q){
    if(emptyQueue(q)) return INT_MIN;
    if(q->start == q->end){
        int value = q->start->dado;
        free(q->start);
        q->start = (Node *)NULL;
        q->end = q->start;
        return value;
    }
    else{
        Node *oldStartNode = q->start;
        int value = oldStartNode->dado;
        q->start = oldStartNode->next;
        free(oldStartNode);
        return value;
    }
}

int main(int argc, char *argv[]){

    Queue *queue;

    // Creating shared memory segment
    key_t key = 5429;
    int shm_id = shmget(key, sizeof(Queue), IPC_CREAT | 0666);
    if(shm_id < 0){
        perror("Error while creating shared memory segment");
        exit(1);
    }

    // Assigning queue to the memory area
    queue = (Queue *)shmat(shm_id, NULL, 0);
    // Initializing queue
    createQueue(queue);
    
    printf("Empty queue %i\n", emptyQueue(queue));
    enQueue(createNode(2), queue);
    printf("Queued node value: %i\n", queue->start->dado);
    printf("Empty queue %i\n", emptyQueue(queue));
    printf("Unqueued value: %i\n", unQueue(queue));
    printf("Empty queue %i\n", emptyQueue(queue));
    enQueue(createNode(1), queue);
    printf("Queued node value: %i\n", queue->start->dado);
    enQueue(createNode(2), queue);
    printf("Queued node value: %i\n", queue->start->next->dado);
    printf("Empty queue %i\n", emptyQueue(queue));
    printf("Unqueued value: %i\n", unQueue(queue));
    printf("Unqueued value: %i\n", unQueue(queue));
    printf("Empty queue %i\n", emptyQueue(queue));

}