
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 4001
#define SERVER_ADDR "127.0.0.1"

#define BUFFER_LENGTH 4096

int main(int argc, char *argv[]){
    // Socket of server
    struct sockaddr_in server;
    // Local client socket file descriptor
    int client_socketfd;

    // Buffer to receive data
    char buffer_in[BUFFER_LENGTH];
    // Buffer to send data
    char buffer_out[BUFFER_LENGTH];

    if( (client_socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
        perror("Error while creating client socket\n");
        exit(1);
    }

    // Connection properties
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    memset(server.sin_zero, 0x0, 8);

    //Connecting with server
    if( (connect(client_socketfd, (struct sockaddr *)&server, sizeof(server) )) == -1 ){
        perror("Failed to connect\n");
        exit(1);
    }

    while (1)
    {
        memset(buffer_out, 0, BUFFER_LENGTH);
        fgets(buffer_out, BUFFER_LENGTH, stdin);

        send(client_socketfd, buffer_out, strlen(buffer_out), 0);
    }
    
    exit(0);
}