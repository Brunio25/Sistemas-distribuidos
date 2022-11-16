#// Grupo 4
#// Renato Custódio nº56320
#// Bruno Soares nº57100
#// Guilherme Marques nº55472

#define NFDESC 10

#include "network_server.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "message-private.h"
#include "sdmessage.pb-c.h"

int sockfd;

void close_server(int sig) {
    network_server_close();
    exit(0);
}

/* Função para preparar uma socket de receção de pedidos de ligação
 * num determinado porto.
 * Retornar descritor do socket (OK) ou -1 (erro).
 */
int network_server_init(short port) {
    struct sockaddr_in server;
    int opt = 1;

    if (tree_skel_init(5) == -1) {  // numero de threads secundarias
        return -1;
    }

    // Cria socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket");
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);               // Porta TCP
    server.sin_addr.s_addr = htonl(INADDR_ANY);  // Todos os endereços na máquina

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Erro ao fazer bind");
        close(sockfd);
        return -1;
    }

    // Faz listen
    if (listen(sockfd, 0) < 0) {  // TODO arg N
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
    struct sockaddr *client = malloc(sizeof(struct sockaddr));
    socklen_t *size_client = malloc(sizeof(socklen_t));
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, close_server);
    printf("Awaiting connection...\n");
    struct pollfd desc_set[NFDESC];
    int i, nfds, kfds;
    for (i = 0; i < NFDESC; i++) {
        desc_set[i].fd = -1;
    }
    desc_set[0].fd = sockfd;
    desc_set[0].events = POLLIN;

    nfds = 1;

    while ((kfds = poll(desc_set, nfds, 10)) >= 0) {
        if ((desc_set[0].revents & POLLIN) && (nfds < NFDESC)) {
            if ((desc_set[nfds].fd = accept(desc_set[0].fd, client, size_client)) > 0) {
                desc_set[nfds].events = POLLIN;
                nfds++;
            }
            printf("Connection accepted\n");
        }
        for (i = 1; i < nfds; i++) {
            if (desc_set[i].revents & POLLIN) {
                struct _MessageT *message = NULL;

                if ((message = network_receive(desc_set[i].fd)) == NULL) {
                    printf("Connection terminated\n");
                    close(desc_set[i].fd);
                    desc_set[i].fd = -1;
                    continue;
                }

                if (invoke(message) == -1) {
                    printf("There Has Been an Unexpected Error!\n");
                }

                // printf("Recebido do cliente %d:\n", i);

                if (network_send(desc_set[i].fd, message) < 0) {
                    printf("Connection terminated\n");
                    close(desc_set[i].fd);
                    desc_set[i].fd = -1;
                    continue;
                }
            }
        }
    }
    free(size_client);
    free(client);
    return 0;
}

/* Esta função deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura message_t.
 */
struct _MessageT *network_receive(int client_socket) {
    int lengthRec;
    if (read_all(client_socket, &lengthRec, sizeof(int)) < 0) {
        perror("read error\n");
        return NULL;
    }

    lengthRec = ntohl(lengthRec);

    if (lengthRec < 0) {
        return NULL;
    }

    if (lengthRec > 1000) {
        return NULL;
    }

    uint8_t buf[lengthRec];
    if (read_all(client_socket, buf, lengthRec) < 0) {
        perror("read failed\n");
        return NULL;
    }

    MessageT *recv_msg = message_t__unpack(NULL, lengthRec, buf);

    if (recv_msg == NULL) {
        perror("error unpacking message\n");
        return NULL;
    }

    return recv_msg;
}

/* Esta função deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Libertar a memória ocupada por esta mensagem;
 * - Enviar a mensagem serializada, através do client_socket.
 */
int network_send(int client_socket, struct _MessageT *msg) {
    int len = message_t__get_packed_size(msg);
    int len_network = htonl(len);

    uint8_t *buf = malloc(len);
    if (buf == NULL) {
        perror("malloc error\n");
        return -1;
    }

    message_t__pack(msg, buf);

    if ((write_all(client_socket, &len_network, sizeof(int))) < 0) {
        perror("write failed\n");
        return -1;
    }

    if ((write_all(client_socket, buf, len)) < 0) {
        perror("write failed\n");
        return -1;
    }
    free(buf);

    return 0;
}

/* A função network_server_close() liberta os recursos alocados por
 * network_server_init().
 */
int network_server_close() {
    tree_skel_destroy();
    close(sockfd);
    return 0;
}