#// Grupo 4
#// Renato Custódio nº56320
#// Bruno Soares nº57100
#// Guilherme Marques nº55472

#include "tree_skel.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <netdb.h>
#include <zookeeper/zookeeper.h>

#include "sdmessage.pb-c.h"
#include "tree.h"
#include "tree_skel-private.h"
#include "client_stub-private.h"
#include "network_server.h"

#define ZDATALEN 1024 * 1024

typedef struct String_vector zoo_string;
zoo_string *children_list; //lista de filhos
static char *watcher_ctx = "ZooKeeper Data Watcher";
struct rtree_t *info_system; //struct com o handler
struct info_server *server_info;
const char *zoo_root = "/chain"; // path do node normal
char node_path[120] = ""; // path deste node

char hostbuffer[256];
char *IPbuffer;
struct hostent *host_entry;
struct tree_t *tree;
struct op_proc *operations;
int last_assigned;
int maxNumOps = 40;
pthread_t threadid;
pthread_mutex_t tree_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
int NumThreads;
struct request_t *queue_head;
int close1 = 0;

void *process_request(void *params);

int tree_skel_init(char* host_port) {
    operations = malloc(sizeof(struct op_proc));
    tree = tree_create();

    if (tree == NULL) {
        return -1;
    }

    children_list = (zoo_string *) malloc(sizeof(zoo_string));
    info_system = (struct rtree_t *) malloc (sizeof(struct rtree_t *));

    operations->in_progress = calloc(maxNumOps, sizeof(int));
    operations->max_proc = 0;
    queue_head = NULL;

    pthread_create(&threadid, NULL, &process_request, NULL);
    
    last_assigned = 1;

    return 0;
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy() {
    close1 = 1;
    zookeeper_close(info_system->zh);
    pthread_cond_broadcast(&queue_not_empty);
    free(operations->in_progress);
    free(operations);
    tree_destroy(tree);
    return;
}

/* Verifica se a operação identificada por op_n foi executada.
 */
int verify(int op_n) {
    return operations->max_proc >= op_n;
}

/* Função da thread secundária que vai processar pedidos de escrita.
 */
void *process_request(void *params) {
    while (close1 == 0) {
        pthread_mutex_lock(&queue_lock);
        if (queue_head == NULL) {
            pthread_cond_wait(&queue_not_empty, &queue_lock);
        }
        if (close1 != 0) {
            break;
        }
        struct request_t *request = queue_head;

        exec_write_operation(request);
        int i;
        for (i = 0; i < maxNumOps; i++) {
            if (operations->in_progress[i] == request->op_n) {
                operations->max_proc = request->op_n;
                operations->in_progress[i] = 0;
            }
        }

        if (request->next_request == NULL) {
            queue_head = NULL;
        } else {
            queue_head = queue_head->next_request;
        }

        pthread_mutex_unlock(&queue_lock);
    }
    pthread_join(pthread_self(), NULL);
    return 0;
}

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao incializada)
 */
int invoke(struct _MessageT *msg) {
    struct request_t *request;
    MessageT__Opcode op = msg->opcode;

    if (tree == NULL || op == MESSAGE_T__OPCODE__OP_BAD) {
        msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        return -1;
    }

    if (op == MESSAGE_T__OPCODE__OP_SIZE && msg->c_type == MESSAGE_T__C_TYPE__CT_NONE) {
        msg->opcode = op + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->result = tree_size(tree);

    } else if (op == MESSAGE_T__OPCODE__OP_HEIGHT && msg->c_type == MESSAGE_T__C_TYPE__CT_NONE) {
        msg->opcode = op + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->result = tree_height(tree);

    } else if (op == MESSAGE_T__OPCODE__OP_DEL && msg->c_type == MESSAGE_T__C_TYPE__CT_KEY) {
        msg->opcode = op + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->result = last_assigned;
        int i;
        for (i = 0; i < maxNumOps; i++) {
            if (operations->in_progress[i] == 0) {
                operations->in_progress[i] = last_assigned;
                break;
            }
        }

        request = (struct request_t *)malloc(sizeof(struct request_t));
        request->op_n = last_assigned;
        request->op = 0;
        request->key = msg->key;
        request->data = NULL;
        request->next_request = NULL;
        request->message = msg;
        last_assigned++;
        fill_buffer(request);

    } else if (op == MESSAGE_T__OPCODE__OP_GET && msg->c_type == MESSAGE_T__C_TYPE__CT_KEY) {
        struct data_t *temp = tree_get(tree, msg->key);
        msg->opcode = op + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
        msg->value = malloc(sizeof(MessageT__Data));
        message_t__data__init(msg->value);
        if (temp == NULL) {
            msg->value->datasize = 0;
            msg->value->data = NULL;
        } else {
            msg->value->datasize = temp->datasize;
            msg->value->data = malloc(msg->value->datasize);
            strcpy(msg->value->data, (char *)temp->data);
        }
        data_destroy(temp);

    } else if (op == MESSAGE_T__OPCODE__OP_PUT && msg->c_type == MESSAGE_T__C_TYPE__CT_ENTRY) {
        msg->opcode = op + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->result = last_assigned;
        int i;
        for (i = 0; i < maxNumOps; i++) {
            if (operations->in_progress[i] == 0) {
                operations->in_progress[i] = last_assigned;
            }
        }

        request = (struct request_t *)malloc(sizeof(struct request_t));
        request->op_n = last_assigned;
        request->op = 1;
        request->key = strdup(msg->entry->key);
        request->data = data_create2(msg->entry->value->datasize, strdup(msg->entry->value->data));
        request->next_request = NULL;
        request->message = msg;
        last_assigned++;
        fill_buffer(request);

    } else if (op == MESSAGE_T__OPCODE__OP_GETKEYS && msg->c_type == MESSAGE_T__C_TYPE__CT_NONE) {
        msg->opcode = op + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
        msg->n_keys = tree_size(tree);
        msg->keys = tree_get_keys(tree);

    } else if (op == MESSAGE_T__OPCODE__OP_GETVALUES && msg->c_type == MESSAGE_T__C_TYPE__CT_NONE) {
        msg->opcode = op + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_VALUES;
        msg->n_values = tree_size(tree);
        msg->values = (MessageT__Data **)malloc((msg->n_values) * sizeof(MessageT__Data *));

        struct data_t **values = (struct data_t **)tree_get_values(tree);

        int i = 0;
        while (i < msg->n_values) {
            msg->values[i] = (MessageT__Data *)malloc(sizeof(MessageT__Data));
            message_t__data__init(msg->values[i]);

            msg->values[i]->data = values[i]->data;
            msg->values[i]->datasize = values[i]->datasize;
            i++;
        }
        for (i = 0; i < msg->n_values; i++) {
            free(values[i]);
        }
        free(values);
    } else if (op == MESSAGE_T__OPCODE__OP_VERIFY && msg->c_type == MESSAGE_T__C_TYPE__CT_RESULT) {
        if (msg->result <= 0) {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }
        msg->opcode = op + 1;
        msg->result = verify(msg->result);

    } else {
        msg->opcode = MESSAGE_T__OPCODE__OP_BAD;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        return -1;
    }

    return 0;
}

/* Função que adiciona ao buffer uma task
 * Do genero produtor de requests para o conceito de Produtor/Consumidor
 */
void fill_buffer(struct request_t *request) {
    pthread_mutex_lock(&queue_lock);

    if (queue_head == NULL) {  // Adiciona na cabeca da fila
        queue_head = request;
    } else {  // Adiciona no fim da fila
        struct request_t *aux = queue_head;

        while (aux->next_request != NULL) {
            aux = aux->next_request;
        }

        aux->next_request = request;
    }

    pthread_cond_signal(&queue_not_empty);  // Avisa um bloqueado nessa condicao
    pthread_mutex_unlock(&queue_lock);
}

int exec_write_operation(struct request_t *request) {
    pthread_mutex_lock(&tree_lock);

    if (request->op == 1) {  // Se a operacao = 1 eh put
        int value = tree_put(tree, request->key, request->data);
        if (value == -1) {  // Erro no tree_put
            pthread_mutex_unlock(&tree_lock);
            perror("Tree Put Error.\n");
            return value;
        }
        pthread_mutex_unlock(&tree_lock);
        if(strcmp(info_system->identifier_other, "none") != 0){
            struct _MessageT *msg = malloc(sizeof(struct _MessageT));
            if (msg == NULL) return -1;

            message_t__init(msg);

            msg->entry = (MessageT__Entry *)malloc(sizeof(MessageT__Entry));
            if (msg->entry == NULL) return -1;

            message_t__entry__init(msg->entry);
            msg->entry->key = strdup(request->key);

            msg->entry->value = (MessageT__Data *)malloc(sizeof(MessageT__Data));
            if (msg->entry->value == NULL) return -1;

            message_t__data__init(msg->entry->value);

            msg->entry->value->datasize = request->data->datasize;
            msg->entry->value->data = strdup(request->data->data);
            msg->opcode = MESSAGE_T__OPCODE__OP_PUT;
            msg->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
            network_send(info_system->socket_used, msg);
        }
        return value;

    } else if (request->op == 0) {  // Se a operacao = 0 eh delete
        int value = tree_del(tree, request->key);

        if (value == -1) {  // Erro no tree_del
            pthread_mutex_unlock(&tree_lock);
            return value;
        }
        pthread_mutex_unlock(&tree_lock);
        if(strcmp(info_system->identifier_other, "none") != 0){
            struct _MessageT msg;
            message_t__init(&msg);

            msg.opcode = MESSAGE_T__OPCODE__OP_DEL;
            msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;
            msg.key = request->key;

            network_send(info_system->socket_used, &msg);
        }
        return value;
    }

    return -1;  // Retorna este valor caso a operacao nao seja valida (nem eh put, nem eh delete)
}

/**
* Data Watcher function for /chain node
*/
static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {

    if (state == ZOO_CONNECTED_STATE) {
        if (type == ZOO_CHILD_EVENT) {

            /* Get the updated children and reset the watch */

            if (ZOK != zoo_wget_children(info_system->zh, zoo_root, child_watcher, watcher_ctx, children_list)) {
                printf( "Error setting watch at %s!\n", zoo_root);
            }

            char *next = malloc(ZDATALEN);
            strcpy(next, "0");
            printf("A reconetar ao próximo servidor...\n");
         
            for (int i = 0; i < children_list->count; i++)  {
                
                if(strcmp(next, "0")==0 && strcmp(children_list->data[i], info_system->identifier)>0){
                    strcpy(next, children_list->data[i]);
                }
                if(strcmp(children_list->data[i], info_system->identifier)>0 && strcmp(children_list->data[i], next)<0)
                    strcpy(next, children_list->data[i]);
            }

            sleep(3);
            
            if(strcmp(next, "0")!=0){
              
                info_system->identifier_other= strdup(next);
                
                int data_size = 50;
                char *data = malloc(data_size);
                char dataPath[120];
                strcpy(dataPath, zoo_root);
                strcat(dataPath, "/");
                strcat(dataPath, next);
                zoo_get(info_system->zh, dataPath, 0, data, &data_size, NULL);
                char *host = strtok((char *)IPbuffer, ":");
                int port = atoi(strtok(NULL, ":"));
                info_system->socket.sin_family = AF_INET;
                info_system->socket.sin_port = htons(port);
                if (inet_pton(AF_INET, host, &info_system->socket.sin_addr) < 1) {
                    printf("Erro ao converter IP\n");
                    return;
                }
                //Criar socket TCP
                if((info_system->socket_used = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                    perror("Erro ao criar socket TCP - Cliente");
                    exit(1);
                }
                //printf("SOCKET NUMBER: %d\n\n\n", server->socket_num);
                
                //Estabelece conexao com o servidor
                if(connect(info_system->socket_used,(struct sockaddr *)&info_system->socket, sizeof(info_system->socket)) < 0){
                    perror("Erro ao conetar ao servidor - Client");
                    close(info_system->socket_used);
                    exit(1);
                }

                // signal(SIGPIPE, conn_lost);

                printf("Conetado ao próximo servidor\n");

                free(data);  
                free(next);
            }
        free(children_list);
        }
    }
}
                        


void watcher_func(zhandle_t *zzh, int type, int state, const char *path, void* context){
  if (type == ZOO_SESSION_EVENT) {
    if (state == ZOO_CONNECTED_STATE) {
      info_system->is_connected = 1;
    } else {
      info_system->is_connected = 0;
    }
  }
}

int start_zookeeper(char *zookeeper_addr, char * serverPort){
    info_system->zh = zookeeper_init(zookeeper_addr, watcher_func, 2000, 0, 0, 0);
    if (info_system->zh == NULL) {
        fprintf(stderr, "Error connecting to ZooKeeper server\n");
        return -1;
    }
    sleep(3);
    if(info_system->is_connected){

        if(ZNONODE == zoo_exists(info_system->zh,zoo_root,0, NULL)){
            printf("The node %s does not exist\n", zoo_root );

            // se o chain nao existir ent vai criar-lo
            if(ZOK == zoo_create(info_system->zh, zoo_root, NULL, -1, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0) ){
                printf("The normal node %s created\n",zoo_root);
            } else {
                printf("Failed to create the normal node %s\n", zoo_root );
                free(info_system);
                return -1;
            }

        } // se a chain existir vamos ver os filhos

        gethostname(hostbuffer, sizeof(hostbuffer));
        host_entry = gethostbyname(hostbuffer);
        IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
        strcat(IPbuffer,":");
        strcat(IPbuffer,serverPort);

        
        const char *node = "/node";

        strcat(node_path, zoo_root);
        strcat(node_path, node);
        int new_path_len = 1024;
        char* new_path = malloc (new_path_len);

        if (ZOK != zoo_create(info_system->zh, node_path, IPbuffer, strlen(IPbuffer), &ZOO_OPEN_ACL_UNSAFE, ZOO_SEQUENCE | ZOO_EPHEMERAL , new_path, new_path_len)) {
            fprintf(stderr, "Error creating znode from path %s!\n", node_path);
            exit(EXIT_FAILURE);
        }
        printf("%s\n",new_path);
        info_system->identifier = strdup(new_path);
        free(new_path);


        if (ZOK != zoo_wget_children(info_system->zh, zoo_root, &child_watcher, watcher_ctx, children_list)) {
            fprintf(stderr, "Error setting watch at %s!\n", zoo_root);
        }
        return 0;
    }
    return -1;
}
