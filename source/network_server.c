# // Grupo 4
# // Renato Custódio nº56320
# // Bruno Soares nº57100
# // Guilherme Marques nº55472

#include "../include/network_server.h"
#include "../include/message-private.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int sockfd;
/* Função para preparar uma socket de receção de pedidos de ligação
 * num determinado porto.
 * Retornar descritor do socket (OK) ou -1 (erro).
 */
int network_server_init(short port) {
    struct sockaddr_in server;
    
    // Cria socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("Erro ao criar socket");
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port); // Porta TCP
    server.sin_addr.s_addr = htonl(INADDR_ANY); // Todos os endereços na máquina

    if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("Erro ao fazer bind");
        close(sockfd);
        return -1;
    }
    
    // Faz listen
    if (listen(sockfd, 0) < 0){
        perror("Erro ao executar listen");
        close(sockfd);
        return -1;
    };
    
    return sockfd;
}

/* Esta função deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 */
int network_main_loop(int listening_socket) {
    struct sockaddr client;
    socklen_t size_client;
    int connsockfd;
    while ((connsockfd = accept(listening_socket,(struct sockaddr *) &client, &size_client)) != -1) {
        printf("server recebeu conexao\n");
        struct message_t *message = network_receive(connsockfd);
        //espera resposta
        network_send(connsockfd,message);
        close(connsockfd);
    }
}

/* Esta função deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura message_t.
 */
struct message_t *network_receive(int client_socket) {
    char buf[MAX_MSG+1];
    read_all(client_socket, buf, MAX_MSG);

}

/* Esta função deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Libertar a memória ocupada por esta mensagem;
 * - Enviar a mensagem serializada, através do client_socket.
 */
int network_send(int client_socket, struct message_t *msg){
    
}

/* A função network_server_close() liberta os recursos alocados por
 * network_server_init().
 */
int network_server_close(){
    close(sockfd);
}