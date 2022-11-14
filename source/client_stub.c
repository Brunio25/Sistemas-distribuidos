// Grupo 4
// Renato Custódio nº56320
// Bruno Soares nº57100
// Guilherme Marques nº55472

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client_stub.h"
#include "client_stub-private.h"
#include "entry.h"
#include "network_client.h"

/* Função para estabelecer uma associação entre o cliente e o servidor,
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */
struct rtree_t *rtree_connect(const char *address_port) {
    if (address_port == NULL) {
        perror("Invalid Port\n");
        return NULL;
    }

    struct rtree_t *rtree = malloc(sizeof(struct rtree_t));
    char *localAddress_port = strdup(address_port);
    char *token1 = strtok(localAddress_port, ":");
    char *token2 = strtok(NULL, ":");

    for (int i = 0; token2[i] != '\0'; i++) {
        if (!isdigit(token2[i])) {
            printf("Usage: ./tree-client <server_IP:server_port>\n");
            free(rtree);
            return NULL;
        }
    }

    rtree->server.sin_port = htons(atoi(token2));  // Porta TCP

    if (inet_pton(AF_INET, token1, &rtree->server.sin_addr) < 1) {  // Endereço IP
        printf("Erro ao converter IP\n");
        free(rtree);
        return NULL;
    }

    if (network_connect(rtree) == -1) {
        free(rtree);
        return NULL;
    }
    free(localAddress_port);
    return rtree;
}

/* Termina a associação entre o cliente e o servidor, fechando a
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtree_disconnect(struct rtree_t *rtree) {

    network_close(rtree);
    free(rtree);
    return 0;
}

/* Função para adicionar um elemento na árvore.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Devolve 0 (ok, em adição/substituição) ou -1 (problemas).
 */
int rtree_put(struct rtree_t *rtree, struct entry_t *entry) {
    struct _MessageT *msg = malloc(sizeof(struct _MessageT));
    if (msg == NULL) return -1;

    message_t__init(msg);

    msg->entry = (MessageT__Entry *)malloc(sizeof(MessageT__Entry));
    if (msg->entry == NULL) return -1;

    message_t__entry__init(msg->entry);
    msg->entry->key = strdup(entry->key);

    msg->entry->value = (MessageT__Data *)malloc(sizeof(MessageT__Data));
    if (msg->entry->value == NULL) return -1;

    message_t__data__init(msg->entry->value);

    msg->entry->value->datasize = entry->value->datasize;
    msg->entry->value->data = strdup(entry->value->data);
    msg->opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;


    free(entry->value);
    free(entry);

    struct _MessageT *msgRec = network_send_receive(rtree, msg);

    if (msgRec->opcode != MESSAGE_T__OPCODE__OP_ERROR) {
        message_t__free_unpacked(msg,NULL);
        message_t__free_unpacked(msgRec,NULL);
        return 0;
    }
    message_t__free_unpacked(msg,NULL);
    message_t__free_unpacked(msgRec,NULL);
    return -1;
}

/* Função para obter um elemento da árvore.
 * Em caso de erro, devolve NULL.
 */
struct data_t *rtree_get(struct rtree_t *rtree, char *key) {
    struct _MessageT msg;
    message_t__init(&msg);

    msg.opcode = MESSAGE_T__OPCODE__OP_GET;
    msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg.key = key;

    struct _MessageT *msgRec = network_send_receive(rtree, &msg);

    if (msgRec->opcode != MESSAGE_T__OPCODE__OP_ERROR) {
        struct data_t *data = data_create(msgRec->value->datasize + 1);

        if(strlen(msgRec->value->data) <= 0){
            free(data->data);
            data->data = NULL;
        }else{
            memcpy(data->data, msgRec->value->data, data->datasize);
        }
        message_t__free_unpacked(msgRec,NULL);
        return data;
    }
    message_t__free_unpacked(msgRec,NULL);
    return NULL;
}

/* Função para remover um elemento da árvore. Vai libertar
 * toda a memoria alocada na respetiva operação rtree_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */
int rtree_del(struct rtree_t *rtree, char *key) {
    struct _MessageT msg;
    message_t__init(&msg);

    msg.opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg.key = key;

    struct _MessageT *msgRec = network_send_receive(rtree, &msg);

    if (msgRec->opcode != MESSAGE_T__OPCODE__OP_ERROR) {
        message_t__free_unpacked(msgRec,NULL);
        return 0;
    }
    message_t__free_unpacked(msgRec,NULL);
    return -1;
}

/* Devolve o número de elementos contidos na árvore.
 */
int rtree_size(struct rtree_t *rtree) {
    struct _MessageT msg;
    message_t__init(&msg);

    msg.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    struct _MessageT *msgRec = network_send_receive(rtree, &msg);

    if (msgRec->opcode != MESSAGE_T__OPCODE__OP_ERROR) {
        int val = msgRec->result;
        message_t__free_unpacked(msgRec,NULL);
        return val;
    }
    message_t__free_unpacked(msgRec,NULL);
    return -1;
}

/* Função que devolve a altura da árvore.
 */
int rtree_height(struct rtree_t *rtree) {
    struct _MessageT msg;
    message_t__init(&msg);

    msg.opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    struct _MessageT *msgRec = network_send_receive(rtree, &msg);

    if (msgRec->opcode != MESSAGE_T__OPCODE__OP_ERROR) {
        int val = msgRec->result;
        message_t__free_unpacked(msgRec,NULL);
        return val;
    }
    message_t__free_unpacked(msgRec,NULL);
    return -1;
}

/* Devolve um array de char* com a cópia de todas as keys da árvore,
 * colocando um último elemento a NULL.
 */
char **rtree_get_keys(struct rtree_t *rtree) {
    struct _MessageT msg;
    message_t__init(&msg);

    msg.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    struct _MessageT *msgRec = network_send_receive(rtree, &msg);
    
    
    if (msgRec->opcode != MESSAGE_T__OPCODE__OP_ERROR && msgRec->keys != NULL) {
        char **keys = malloc(sizeof(msgRec->keys));
        
        int i=0;
        while (i < msgRec->n_keys) {
            keys[i] = malloc(sizeof(msgRec->keys[i]));
            strcpy(keys[i], msgRec->keys[i]);
            i++;
        }
        keys[msgRec->n_keys] = NULL;
        message_t__free_unpacked(msgRec,NULL);
        return keys;
    }
    message_t__free_unpacked(msgRec,NULL);
    return NULL;
}

/* Devolve um array de void* com a cópia de todas os values da árvore,
 * colocando um último elemento a NULL.
 */
void **rtree_get_values(struct rtree_t *rtree) {
    struct _MessageT msg;
    message_t__init(&msg);

    msg.opcode = MESSAGE_T__OPCODE__OP_GETVALUES;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    struct _MessageT *msgRec = network_send_receive(rtree, &msg);
    void **values = malloc(sizeof(msgRec->values));
    if (msgRec->opcode != MESSAGE_T__OPCODE__OP_ERROR && msgRec->values != NULL ) {
        int i=0;
        while (i < msgRec->n_values) {
            values[i] = malloc(msgRec->values[i]->datasize + 1);
            strcpy(values[i], msgRec->values[i]->data);
            i++;
        }
        values[msgRec->n_values] = NULL;
        message_t__free_unpacked(msgRec,NULL);
        return values;
    }
    message_t__free_unpacked(msgRec,NULL);
    return NULL;
}
