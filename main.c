
#include<stdio.h>
#include<stdlib.h>

#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/socket.h>
#include<limits.h>
#include<netinet/in.h>
#include<string.h>

#define STOP 1
#define GO 0
#define TRUE 1
#define FALSE 0
#define PORT 4001

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
    
    // Socket structures
    struct sockaddr_in client, server;
    // Socket file descriptors
    int serverfd, clientfd;
    
    // Creating socket
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverfd == -1){
        perror("Error while creating server socket\n");
        exit(1);
    }
    printf("Server socket created with file descriptor %d\n", serverfd);
    // Defining server socket properties
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    memset(server.sin_zero, 0x0, 8);
    // Port busy error handler
    int yes = 1;
    if(setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR,
                  &yes, sizeof(int)) == -1) {
        perror("PORT is busy");
        exit(1);
    }
    // Binding soxket
    if(bind(serverfd, (struct sockaddr *)&server, sizeof(server)) == -1){
        perror("Error while attachind socket to port\n");
        exit(1);
    }
    // Listening for connections on port
    if(listen(serverfd, 4) == -1){
        perror("Error while listening\n");
        exit(1);
    }

    printf("Server listening on port %d\n", PORT);
}