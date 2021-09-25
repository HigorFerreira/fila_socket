#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



// Define a porta do Servidor
#define PORT 4242


// Tamanho do Buffer do Socket
#define LEN 4096


// Endereco do Servidor
#define SERVER_ADDR "127.0.0.1"


int
main(int argc, char *argv[]) {

    // Socket do Servidor
    struct sockaddr_in server;

    // Descritor de arquivo do cliente para o Socket local
    int sockfd;

    int len = sizeof(server);
    int slen;

    // Buffer para receber mensagens
    char buffer_in[LEN];
    // Buffer para enviar mensagens
    char buffer_out[LEN];


    fprintf(stdout, "INICIANDO JOGO DA FORCA ...\n\n\n");


    // Criando Socket para o cliente
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Erro ao criar socket do cliente:");
        return EXIT_FAILURE;
    }
    

    // Define as propriedades da conexão
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    memset(server.sin_zero, 0x0, 8);


    // Conexão com o servidor
    if (connect(sockfd, (struct sockaddr*) &server, len) == -1) {
        perror("Conexão com servidor falhou!!!!!!!");
        return EXIT_FAILURE;
    }



    // Trocando mensagens com o servidor
    while (true) {

        // Limpando Buffers
        memset(buffer_in, 0x0, LEN);
        memset(buffer_out, 0x0, LEN);
        
        recv(sockfd, buffer_in, LEN, 0);
        
        // Mensagem'num_cliente' finaliza a conexão
        if((strcmp(buffer_in, "1") == 0) || (strcmp(buffer_in, "2") == 0) || (strcmp(buffer_in, "3") == 0) || (strcmp(buffer_in, "4") == 0)){
            printf("\n\nJogo finalizado. Ganhador foi o jogador: %s\n", buffer_in);
            break;
        }

        // Mensagem'Ganhou' finaliza a conexão
        if(strcmp(buffer_in, "Ganhou") == 0){
            fprintf(stdout, "\n\nVoce Ganhouuuuuuuuuuuuuu\n\n");
            break;
        }

        fprintf(stdout, "\n\nMensagem do Servidor %s\n\n", buffer_in);

        // Limpando Buffers
        memset(buffer_in, 0x0, LEN);
        memset(buffer_out, 0x0, LEN);

        fprintf(stdout, "Digite uma letra: ");
        fgets(buffer_out, LEN, stdin);

        // Enviando mensagem para o servidor pelo Socket
        send(sockfd, buffer_out, strlen(buffer_out), 0);

        // Recebendo mensagens do Servidor
        slen = recv(sockfd, buffer_in, LEN, 0);

        // Mensagem'num_cliente' finaliza a conexão
        if( (strcmp(buffer_in, "1") == 0) || (strcmp(buffer_in, "2") == 0) || (strcmp(buffer_in, "3") == 0) || (strcmp(buffer_in, "4") == 0)){
            printf("\n\nJogo finalizado. Ganhador foi o jogador: %s\n", buffer_in);
            break;
        }

        // Mensagem'Ganhou' finaliza a conexão
        if(strcmp(buffer_in, "Ganhou") == 0){
            fprintf(stdout, "\n\nVoce Ganhouuuuuuuuuuuuuu\n\n");
            break;
        }

        printf("\n\nResposta do servidor: %s\n", buffer_in);
        
    }


    // Fecha a conexão com o servidor
    close(sockfd);

    return EXIT_SUCCESS;
}