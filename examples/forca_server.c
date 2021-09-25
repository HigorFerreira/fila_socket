#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// Struct Elementos do Jogo
struct elementos_jogo{
    int jogadores_sala[4];
    char PalavraCorrente[47];
    char PalavraOculta[47];
    char dica[100];
    int num_processo;
    int ganhador;
    int cont_conexoes;
    int pode_jogar;
    int fim_jogo;
};


// Vetor de Palavras cadastradas
char palavras[5][47] = { "babaca", "gelo", "farinha", "vegano", "remanescente"};

// Vetor de Dicas
char dicas[5][100] = { "Adjetivo perjorativo", "Estado da agua", "Baiano", "Fotosintese", "O que falta"};

// ID do processo filho criado
pid_t id_processo;


// Porta do servidor
#define PORT 4242

// Tamanho do Buffer
#define BUFFER_LENGTH 7168

// Funcoes de manipulacao do jogo
void InicioJogo(int socket, struct elementos_jogo *jogo_corrente, int num_processo_localp);
void GerarPalavra(struct elementos_jogo *jogo_corrente);
void TestarPalpite(char buffer[BUFFER_LENGTH], struct elementos_jogo *jogo_corrente);


int
main(void) {

    // Area de Memoria Compartilhada entre processo servidor pai e seus filhos
    int shmid;
    key_t key;
    struct elementos_jogo *jogo_corrente = 0;

    key = 5678;

    // Criando segmento de memoria compartilhada
    shmid = shmget(key, sizeof(struct elementos_jogo), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("Erro ao criar o segmento de memoria compartilhada no processo pai com shmget");
        return EXIT_FAILURE;
    }

    // Associando segmento de memoria compatilhada ao processo servidor pai
    jogo_corrente = (struct elementos_jogo *) shmat(shmid, NULL, 0);
    if ((int) jogo_corrente == -1) {
        perror("Erro ao associar o segmento de memoria compartilhada ao endereco do processo pai com shmat");
        return EXIT_FAILURE;
    }

    // Inicializando jogo corrente
    (*jogo_corrente).jogadores_sala[0]=0;
    (*jogo_corrente).jogadores_sala[0]=0;
    (*jogo_corrente).jogadores_sala[0]=0;
    (*jogo_corrente).jogadores_sala[0]=0;

    strcpy((*jogo_corrente).PalavraCorrente, "");
    strcpy((*jogo_corrente).PalavraOculta, "");
    strcpy((*jogo_corrente).dica, "");

    (*jogo_corrente).num_processo = 0;
    (*jogo_corrente).ganhador = 0;
    (*jogo_corrente).cont_conexoes = 0;
    (*jogo_corrente).pode_jogar = 1;
    (*jogo_corrente).fim_jogo = 0;

    // Estruturas do Socket Cliente e Servidor
    struct sockaddr_in client, server;

    // Descritores de arquivo do Servidor e do Cliente 
    int serverfd, clientfd;

    char buffer[BUFFER_LENGTH];

    fprintf(stdout, "Iniciando Servidor\n\n\n");

    // Criando Socket IPv4
    serverfd = socket(AF_INET, SOCK_STREAM, 0);

    if(serverfd == -1) {
        perror("Erro ao criar Socket do Servidor:");
        return EXIT_FAILURE;
    }

    fprintf(stdout, "Socket do servidor criado com descritor de arquivo: %d\n", serverfd);


    // Define as propriedades do Socket do Servidor
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    memset(server.sin_zero, 0x0, 8);


    // Trata o erro quando a porta em uso está ocupada
    int yes = 1;
    if(setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR,
                  &yes, sizeof(int)) == -1) {
        perror("Erro do Socket:");
        return EXIT_FAILURE;
    }


    // Funcao bind que vincula o socket a uma porta
    if(bind(serverfd, (struct sockaddr*)&server, sizeof(server)) == -1 ) {
        perror("Erro ao vincular socket a porta:");
        return EXIT_FAILURE;
    }


    // Funcao Listen que permite que o servidor espere por conexoes de clientes na porta definida 
    if(listen (serverfd, 4) == -1) {
        perror("Erro na função Listen:");
        return EXIT_FAILURE;
    }

    fprintf(stdout, " Servidor escutando na porta %d\n", PORT);

    
    do { 
            // Aceitando conexao com cliente
            socklen_t client_len = sizeof(client);
            if ((clientfd=accept(serverfd,(struct sockaddr *) &client, &client_len )) == -1) {
                perror("Erro ao aceitar conexão do cliente:");
                return EXIT_FAILURE;
            }

            // Incrementando numero de conexoes
            (*jogo_corrente).cont_conexoes++;

            // Incrementando o numero de procesos
            (*jogo_corrente).num_processo++;

            // Se for a primeira conexao é preparado o jogo corrente
            if((*jogo_corrente).cont_conexoes == 1){
                GerarPalavra(jogo_corrente);
            }

            // Criando processo filho para delegar tarefa de iniciar jogo
            id_processo = fork();

            if(id_processo < 0){
                perror("Fork falhou!!!!!!!!!");
                return EXIT_FAILURE;
            }else if(id_processo == 0){

                int cont;
                for(cont = 0; cont < 4; cont++){
                    if((*jogo_corrente).jogadores_sala[cont] == 0){
                        (*jogo_corrente).jogadores_sala[cont] == id_processo;
                    }
                }

                // Associando memoria compartilhada com o processo filho...
                int shmid;
                key_t key;
                struct elementos_jogo *jogo_corrente;

                key = 5678;

                shmid = shmget(key, sizeof(struct elementos_jogo), 0666);
                if (shmid < 0) {
                    perror("Erro ao criar o segmento de memoria compartilhada do processo filho com shmget");
                    return EXIT_FAILURE;
                }

                jogo_corrente = (struct elementos_jogo *) shmat(shmid, NULL, 0);
                if ((int) jogo_corrente == -1) {
                    perror("Erro ao associar o segmento de memoria compartilhada ao endereco do processo filho com shmat");
                    return EXIT_FAILURE;
                }

                // Atribuindo ao processo seu numero
                int num_processo_local = (*jogo_corrente).num_processo;

                // Fechando conexao com socket servidor
                close(serverfd);

                // Processo filho vai requisitar jogada do cliente até que seja fim de jogo
                do{
                    if((*jogo_corrente).pode_jogar == num_processo_local){
                        printf("Movimento do jogador: %d\n", num_processo_local);
                        InicioJogo(clientfd, jogo_corrente, num_processo_local);

                        // Passo a vez para proximo jogador se houver mais conexoes
                        if((*jogo_corrente).cont_conexoes > (*jogo_corrente).pode_jogar){
                            (*jogo_corrente).pode_jogar++;
                        }else{
                            (*jogo_corrente).pode_jogar = 1;
                        }

                    }

                    if ((*jogo_corrente).fim_jogo != 1){
                        while((*jogo_corrente).pode_jogar != num_processo_local)
                            sleep(1);
                    }

                }while((*jogo_corrente).fim_jogo != 1);

                // Guardando numero do processo do cliente ganhador
                if((*jogo_corrente).ganhador == 0){
                    (*jogo_corrente).ganhador = num_processo_local;
                }
                
                // Se o cliente não for o ganhador enviar para o cliente que o jogo acabou e o numero do jogador
                if((*jogo_corrente).ganhador != num_processo_local){
                    char buffer[BUFFER_LENGTH];
                    
                    sprintf(buffer, "%d", (*jogo_corrente).ganhador);

                    send(clientfd, buffer, strlen(buffer), 0);


                    // Passo a vez para proximo jogador se houver mais conexoes
                    if ((*jogo_corrente).num_processo != 1){
                        if((*jogo_corrente).cont_conexoes > (*jogo_corrente).pode_jogar){
                            (*jogo_corrente).pode_jogar++;
                        }else{
                            (*jogo_corrente).pode_jogar = 1;
                        }
                    }
                }
                    
                // Fechando conexao com cliente
                close(clientfd);

                // Desassociando memoria compartilhada do espaço de endereçamento do processo
                shmdt((void *) jogo_corrente);

                // Decrementando contador de processos e contador de conexões
                (*jogo_corrente).num_processo--;
                (*jogo_corrente).cont_conexoes--;

                // Fechando processo
                exit(0);
            }

            printf("Contador Processos: %d\n", (*jogo_corrente).num_processo);
            printf("Contador Conexoes: %d\n", (*jogo_corrente).cont_conexoes);

        } while(1);

    // Desassociando memoria compartilhada do espaço de endereçamento do processo
    shmdt((void *) jogo_corrente);

    // Remove o segmento de memoria partilhada
    shmctl(shmid, IPC_RMID, NULL);


    return EXIT_SUCCESS; 
}

void Inicializar_JogoCorrente(struct elementos_jogo *jogo_corrente){

    // Inicializando jogo corrente
    (*jogo_corrente).jogadores_sala[0]=0;
    (*jogo_corrente).jogadores_sala[0]=1;
    (*jogo_corrente).jogadores_sala[0]=2;
    (*jogo_corrente).jogadores_sala[0]=3;

    strcpy((*jogo_corrente).PalavraCorrente, "");
    strcpy((*jogo_corrente).PalavraOculta, "");
    strcpy((*jogo_corrente).dica, "");

    (*jogo_corrente).num_processo = 0;
    (*jogo_corrente).ganhador = 0;
    (*jogo_corrente).cont_conexoes = 0;
    (*jogo_corrente).pode_jogar = 1;
    (*jogo_corrente).fim_jogo = 0;
}


void InicioJogo(int socket, struct elementos_jogo *jogo_corrente, int num_processo_localp){
    
    char buffer[BUFFER_LENGTH];

    // Copia o numero do jogador para o buffer
    strcpy(buffer, "\n\nJogador     ");
    char processo[2];
    sprintf(processo, "%d", num_processo_localp);
    strcat(buffer, processo);
    strcat(buffer, "\n\n");

    // Copia a Dica da palavra para Buffer
    strcat(buffer, "Dica da Palavra: \n");
    strcat(buffer, (*jogo_corrente).dica);
    strcat(buffer, "\n\n");

    // Copia Palavra Oculta para buffer
    strcat(buffer, "Palavra: \n");
    strcat(buffer, (*jogo_corrente).PalavraOculta);
    strcat(buffer, "\0");

    // Envia a palavraOculta e dica para o cliente
    send(socket, buffer, strlen(buffer), 0);

    // Recebe mensagem do cliente
    int message_len;
    if((message_len = recv(socket, buffer, BUFFER_LENGTH, 0)) > 0) {
        buffer[message_len - 1] = '\0';
    }

    // Testa se a letra enviada pelo cliente existe na palavra
    TestarPalpite(buffer, jogo_corrente);

    if(strcmp((*jogo_corrente).PalavraOculta, (*jogo_corrente).PalavraCorrente) == 0) {
        (*jogo_corrente).fim_jogo = 1;
        send(socket, "Ganhou", 6, 0);
    } else {
        // Limpando buffer
        memset(buffer, 0x0, BUFFER_LENGTH);
        strcpy(buffer, (*jogo_corrente).PalavraOculta);
        send(socket, buffer, strlen(buffer), 0);
    }

}



void GerarPalavra(struct elementos_jogo *jogo_corrente){

    // Funcao para gerar valor aleatorio da posicao da palavra
    srand(time(NULL));
    int posicao = rand() % 4;

    strcpy((*jogo_corrente).PalavraCorrente, palavras[posicao]);
    strcpy((*jogo_corrente).dica, dicas[posicao]);
    int tamanho = strlen((*jogo_corrente).PalavraCorrente);
    int c;
    for(c=0;c<tamanho;c++){
        strcat((*jogo_corrente).PalavraOculta, "-");
    }

}

void TestarPalpite(char buffer[BUFFER_LENGTH], struct elementos_jogo *jogo_corrente){

    int contador;
    int tamanho = strlen((*jogo_corrente).PalavraCorrente);
    for(contador=0; contador<tamanho; contador++){
        if((*jogo_corrente).PalavraCorrente[contador] == buffer[0]){
           (*jogo_corrente).PalavraOculta[contador] = buffer[0];
        }
    }
}