// Grupo 4
// Renato Custódio nº56320
// Bruno Soares nº57100
// Guilherme Marques nº55472

#include "client_stub.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <zookeeper/zookeeper.h>

#include "client_stub-private.h"
#include "entry.h"
#include "network_client.h"
#include "tree_skel-private.h"

/* Função para estabelecer uma associação entre o cliente e o servidor,
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */

#define ZDATALEN 1024 
struct rtree_t *head;
struct rtree_t *tail;
static char *root_path = "/chain";
typedef struct String_vector zoo_string;
zoo_string *children_list;
static char *watcher_ctx = "ZooKeeper Data Watcher";

struct rtree_t *conn;

void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void *context){
    if (type == ZOO_SESSION_EVENT){
        if (state == ZOO_CONNECTED_STATE){
            conn->is_connected = 1;
        }
        else{
            conn->is_connected = 0;
        }
    }
}

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx){
    //filhos tiveram update entao precisamos ver novos head/tail e ligar a esses
    int data_len = 60;
    children_list = (zoo_string *)malloc(sizeof(zoo_string));
    char headPath[125];
    char tailPath[125];
    char *headInfo = malloc(data_len);
    char *tailInfo = malloc(data_len);
    if (state == ZOO_CONNECTED_STATE){
        if (type == ZOO_CHILD_EVENT){
            /* Get the updated children and reset the watch */
            if (ZOK != zoo_wget_children(conn->zh, root_path, child_watcher, watcher_ctx, children_list)){
                fprintf(stderr, "Error setting watch at %s!\n", root_path);
            }
            //conseguir nome do menor node e maior node
            printf("Ao reconetar aos servidores...\n");
            for (int i = 0; i < children_list->count; i++){
                head->identifier = children_list->data[0];
                tail->identifier = children_list->data[0];
                for (int i = 0; i < children_list->count; i++){
                    if (strcmp(children_list->data[i], head->identifier) < 0)
                        head->identifier = children_list->data[i];
                    if (strcmp(children_list->data[i], tail->identifier) > 0)
                        tail->identifier = children_list->data[i];
                }
            }
           
            sleep(7);//tempo aos nodes reorganizarem

            //conseguir path do maior e menor node
            strcpy(headPath, root_path);
            strcat(headPath, "/");
            strcat(headPath, head->identifier);
            strcpy(tailPath, root_path);
            strcat(tailPath, "/");
            strcat(tailPath, tail->identifier);
            //conseguir data do maior e menor node e guardar em head e tail
            zoo_get(conn->zh, headPath, 0, headInfo, &data_len, NULL);
            zoo_get(conn->zh, tailPath, 0, tailInfo, &data_len, NULL);
            //conetar a esses servidores para enviar/receber pedidos
            if(connectServer(head, headInfo) == -1){
                printf("Erro ao conetar a Head");
                exit(1);
            }
            if(connectServer(tail, tailInfo) == -1){
                printf("Erro ao conetar a Tail");
                exit(1);
            }
        }
    }
    free(children_list);
    free(headInfo);
    free(tailInfo);
}

int connectServer(struct rtree_t *server, char *serverInfo){
    char *host = strtok((char *)serverInfo, ":");
    int port = atoi(strtok(NULL, ":")); 

    server->socket.sin_family = AF_INET;
    server->socket.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &server->socket.sin_addr) < 1) {
        printf("Erro ao converter IP\n");
        return -1;
    }
    if (network_connect(server) == -1) {
        return -1;
    }

    return 0;
}


struct rtree_t *rtree_connect(const char *address_port) {
    conn = (struct rtree_t *)malloc(sizeof(struct rtree_t));
    head = (struct rtree_t *)malloc(sizeof(struct rtree_t));
    tail = (struct rtree_t *)malloc(sizeof(struct rtree_t));
    children_list = (zoo_string *)malloc(sizeof(zoo_string));

    conn->zh = zookeeper_init(address_port, connection_watcher, 20000, 0, NULL, 0);
    if (conn->zh == NULL){
        fprintf(stderr, "Error connecting to ZooKeeper server[%d]!\n", errno);
        exit(EXIT_FAILURE);
    }
    sleep(4); //dorme para conectar

    int data_len = 50;
    char headPath[120];
    char tailPath[120];
    char *headInfo = malloc(data_len);
    char *tailInfo = malloc(data_len);

    if (conn->is_connected){
        // Possibilidade de usar um watch do /chain se nao existir (no valor 0)
        if (ZNONODE == zoo_exists(conn->zh, root_path, 0, NULL)){
            printf("Error: %s doesnt exist!!", root_path);
        }
        
        if (ZOK != zoo_wget_children(conn->zh, root_path, &child_watcher, watcher_ctx, children_list)){
            fprintf(stderr, "Error setting watch at %s!\n", root_path);
        }

        //conseguir nome do menor node e maior node(ex: node0000001)
        //fprintf(stderr, "\n=== znode Head and Tail set === [ %s ]", root_path);
        head->identifier = children_list->data[0];
        tail->identifier = children_list->data[0];
        for (int i = 0; i < children_list->count; i++){
            if (strcmp(children_list->data[i], head->identifier) < 0)
                head->identifier = children_list->data[i];
            if (strcmp(children_list->data[i], tail->identifier) > 0)
                tail->identifier = children_list->data[i];
        }
        //fprintf(stderr, "\n=== done ===\n");
        //conseguir path do maior e menor node (ex: /chain/node000001)
        strcpy(headPath, root_path);
        strcat(headPath, "/");
        strcat(headPath, head->identifier);

        strcpy(tailPath, root_path);
        strcat(tailPath, "/");
        strcat(tailPath, tail->identifier);
        //conseguir data do maior e menor node("IP:Port") e guardar em headInfo e tailInfo
        zoo_get(conn->zh, headPath, 0, headInfo, &data_len, NULL);
        zoo_get(conn->zh, tailPath, 0, tailInfo, &data_len, NULL);
    }
    //conetar a esses servidores para enviar/receber pedidos
    if(connectServer(head, headInfo) == -1){
        printf("Erro ao conetar a Head");
        return NULL;
    }
    if(connectServer(tail, tailInfo) == -1){
        printf("Erro ao conetar a Tail");
        return NULL;
    }

    free(children_list);
    free(headInfo);
    free(tailInfo);

    return NULL;
}

/* Termina a associação entre o cliente e o servidor, fechando a
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtree_disconnect(struct rtree_t *rtree) {
   if (network_close(head) != 0)
        return -1;
    if (network_close(tail) != 0)
        return -1;
        
    free(head);
    free(tail);
    zookeeper_close(conn->zh);
    free(conn);
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

    struct _MessageT *msgRec = network_send_receive(head, msg);

    if (msgRec->opcode == MESSAGE_T__OPCODE__OP_PUT + 1 && msgRec->c_type == MESSAGE_T__C_TYPE__CT_RESULT) {
        int waitNum = msgRec->result;
        message_t__free_unpacked(msg, NULL);
        message_t__free_unpacked(msgRec, NULL);
        return waitNum;
    }
    message_t__free_unpacked(msg, NULL);
    message_t__free_unpacked(msgRec, NULL);
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

    struct _MessageT *msgRec = network_send_receive(tail, &msg);

    if (msgRec->opcode != MESSAGE_T__OPCODE__OP_ERROR) {
        struct data_t *data = data_create(msgRec->value->datasize + 1);

        if (strlen(msgRec->value->data) <= 0) {
            free(data->data);
            data->data = NULL;
        } else {
            memcpy(data->data, msgRec->value->data, data->datasize);
        }
        message_t__free_unpacked(msgRec, NULL);
        return data;
    }
    message_t__free_unpacked(msgRec, NULL);
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

    struct _MessageT *msgRec = network_send_receive(head, &msg);

    if (msgRec->opcode == MESSAGE_T__OPCODE__OP_DEL + 1 && msgRec->c_type == MESSAGE_T__C_TYPE__CT_RESULT) {
        int waitNum = msgRec->result;
        message_t__free_unpacked(msgRec, NULL);
        return waitNum;
    }
    message_t__free_unpacked(msgRec, NULL);
    return -1;
}

/* Devolve o número de elementos contidos na árvore.
 */
int rtree_size(struct rtree_t *rtree) {
    struct _MessageT msg;
    message_t__init(&msg);

    msg.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    struct _MessageT *msgRec = network_send_receive(tail, &msg);

    if (msgRec->opcode != MESSAGE_T__OPCODE__OP_ERROR) {
        int val = msgRec->result;
        message_t__free_unpacked(msgRec, NULL);
        return val;
    }
    message_t__free_unpacked(msgRec, NULL);
    return -1;
}

/* Função que devolve a altura da árvore.
 */
int rtree_height(struct rtree_t *rtree) {
    struct _MessageT msg;
    message_t__init(&msg);

    msg.opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    struct _MessageT *msgRec = network_send_receive(tail, &msg);

    if (msgRec->opcode != MESSAGE_T__OPCODE__OP_ERROR) {
        int val = msgRec->result;
        message_t__free_unpacked(msgRec, NULL);
        return val;
    }
    message_t__free_unpacked(msgRec, NULL);
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

    struct _MessageT *msgRec = network_send_receive(tail, &msg);

    if (msgRec->opcode != MESSAGE_T__OPCODE__OP_ERROR && msgRec->keys != NULL) {
        char **keys = malloc(sizeof(msgRec->keys) + 1);

        int i = 0;
        while (i < msgRec->n_keys) {
            keys[i] = malloc(sizeof(msgRec->keys[i]));
            strcpy(keys[i], msgRec->keys[i]);
            i++;
        }
        keys[msgRec->n_keys] = NULL;
        message_t__free_unpacked(msgRec, NULL);
        return keys;
    }
    message_t__free_unpacked(msgRec, NULL);
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

    struct _MessageT *msgRec = network_send_receive(tail, &msg);
    void **values = malloc(sizeof(msgRec->values) + 1);
    if (msgRec->opcode != MESSAGE_T__OPCODE__OP_ERROR && msgRec->values != NULL) {
        int i = 0;
        while (i < msgRec->n_values) {
            values[i] = malloc(msgRec->values[i]->datasize + 1);
            strcpy(values[i], msgRec->values[i]->data);
            i++;
        }
        values[msgRec->n_values] = NULL;
        message_t__free_unpacked(msgRec, NULL);
        return values;
    }
    message_t__free_unpacked(msgRec, NULL);
    return NULL;
}

/* Verifica se a operação identificada por op_n foi executada.
 */
int rtree_verify(struct rtree_t *rtree, int op_n) {
    struct _MessageT msg;
    message_t__init(&msg);

    msg.opcode = MESSAGE_T__OPCODE__OP_VERIFY;
    msg.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
    msg.result = op_n;

    struct _MessageT *msgRec = network_send_receive(tail, &msg);

    if (msgRec->opcode == MESSAGE_T__OPCODE__OP_VERIFY + 1 && msgRec->c_type == MESSAGE_T__C_TYPE__CT_RESULT) {
        int result = msgRec->result;
        message_t__free_unpacked(msgRec, NULL);
        return result;
    }
    message_t__free_unpacked(msgRec, NULL);
    return -1;
}
