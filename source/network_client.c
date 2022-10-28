// Grupo 4
// Renato Custódio nº56320
// Bruno Soares nº57100
// Guilherme Marques nº55472

#include "../include/network_client.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/client_stub-private.h"
#include "../include/message-private.h"

/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) a base da
 *   informação guardada na estrutura rtree;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtree;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtree_t *rtree) {
    // Cria socket TCP
    if ((rtree->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket TCP");
        return -1;
    }

    // Preenche estrutura server com endereço do servidor para estabelecer
    // conexão
    rtree->server.sin_family = AF_INET;  // família de endereços

    // Estabelece conexão com o servidor definido na estrutura server
    if (connect(rtree->sockfd, (struct sockaddr *)&rtree->server, sizeof(rtree->server)) < 0) {
        perror("Erro ao conectar-se ao servidor");
        close(rtree->sockfd);
        return -1;
    }

    return 0;
}

/* Esta função deve:
 * - Obter o descritor da ligação (socket) da estrutura rtree_t;
 * - Serializar a mensagem contida em msg;
 * - Enviar a mensagem serializada para o servidor;
 * - Esperar a resposta do servidor;
 * - De-serializar a mensagem de resposta;
 * - Retornar a mensagem de-serializada ou NULL em caso de erro.
 */
struct _MessageT *network_send_receive(struct rtree_t *rtree, struct _MessageT *msg) {
    int sockfd = rtree->sockfd;

    int len = message_t__get_packed_size(msg);
    uint8_t *sendBuf = malloc(len);
    if (sendBuf == NULL) {
        perror("malloc error\n");
        return NULL;
    }
    message_t__pack(msg, sendBuf);
    int len_network = htonl(len);
    write(sockfd, &len_network, sizeof(int));
    write_all(sockfd, sendBuf, len);
    free(sendBuf);

    int lengthRec;
    if (read(sockfd, &lengthRec, sizeof(int)) < 0) {
        perror("Receaving error\n");
        return NULL;
    }
    lengthRec = ntohl(lengthRec);

    uint8_t recBuf[lengthRec];
    read_all(sockfd, recBuf, lengthRec);

    MessageT *recMsg = message_t__unpack(NULL, lengthRec, recBuf);

    return recMsg;
}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtree_t *rtree) {
    close(rtree->sockfd);
    return 0;
}