#// Grupo 4
#// Renato Custódio nº56320
#// Bruno Soares nº57100
#// Guilherme Marques nº55472

#include "tree_skel.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sdmessage.pb-c.h"
#include "tree.h"
#include "tree_skel-private.h"

struct tree_t *tree;
struct op_proc operations;
int last_assigned;
int maxNumOps = 40;
pthread_t *threadids;
pthread_mutex_t queue_lock, tree_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
int op_count;
int NumThreads;
struct request_t *queue_head;

/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke().
 * A função deve lançar N threads secundárias responsáveis por atender
 * pedidos de escrita na árvore.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int tree_skel_init(int N) {
    NumThreads = N;
    tree = tree_create();

    if (tree == NULL) {
        return -1;
    }

    operations.in_progress = calloc(maxNumOps, sizeof(int));
    queue_head = NULL;

    int i;
    threadids = malloc(sizeof(pthread_t) * NumThreads);
    for (i = 0; i < NumThreads; i++) {
        pthread_create(&threadids[i], NULL, &process_request, (void *)&queue_head);
    }

    last_assigned = 1;

    return 0;
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy() {
    tree_destroy(tree);
    int i = 0;
    for (i = 0; i < NumThreads; i++) {
        pthread_exit(&threadids[i]);
    }
    free(threadids);
    return;
}

/* Verifica se a operação identificada por op_n foi executada.
 */
int verify(int op_n) {
    return operations.max_proc >= op_n;
}

/* Função da thread secundária que vai processar pedidos de escrita.
 */
void *process_request(void *params) {
    while (1) {
        pthread_mutex_lock(&queue_lock);

        while (queue_head == NULL) {  // TODO if em vez de while?
            pthread_cond_wait(&queue_not_empty, &queue_lock);
        }

        struct request_t *request = queue_head;

        exec_write_operation(request);
        operations.max_proc = request->op_n;  // max proc não estava a ser atualizado

        if (request->next_request == NULL) {
            queue_head = NULL;
        } else {
            queue_head = queue_head->next_request;
        }
        pthread_mutex_unlock(&queue_lock);
    }
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
        tree_del(tree, msg->key);
        msg->result = last_assigned;
        int i;
        for (i = 0; i < maxNumOps; i++) {
            if (operations.in_progress[i] == 0) {
                operations.in_progress[i] = last_assigned;
            }
        }
        last_assigned++;
        request = (struct request_t *)malloc(sizeof(struct request_t));
        request->op_n = op_count;
        request->op = 0;
        request->key = msg->key;
        request->data = NULL;
        request->next_request = NULL;
        request->message = msg;

        fill_buffer(request);
        free(request);

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
            msg->value->data = temp->data;
        }

    } else if (op == MESSAGE_T__OPCODE__OP_PUT && msg->c_type == MESSAGE_T__C_TYPE__CT_ENTRY) {
        msg->opcode = op + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        struct data_t *data = data_create2(msg->entry->value->datasize, msg->entry->value->data);
        if (data == NULL) {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }

        if ((tree_put(tree, msg->entry->key, data)) == -1) {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }
        msg->result = last_assigned;
        int i;
        for (i = 0; i < maxNumOps; i++) {
            if (operations.in_progress[i] == 0) {
                operations.in_progress[i] = last_assigned;
            }
        }
        last_assigned++;
        request = (struct request_t *)malloc(sizeof(struct request_t));
        request->op_n = op_count;
        request->op = 1;
        request->key = msg->key;
        request->data = data_create2(msg->entry->value->datasize, msg->entry->value->data);
        request->next_request = NULL;
        request->message = msg;

        fill_buffer(request);
        data_destroy(request->data);  // destruição de data temp
        free(request);
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

            msg->values[i]->data = values[i]->data;  // bug ? put a b + put a b = merda
            // printf("msg: %s, func: %s\n",msg->values[i]->data,(char *)values[i]->data);
            msg->values[i]->datasize = values[i]->datasize;
            i++;
        }

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
 * Do genero produtor de tasks para o conceito de Produtor/Consumidor
 */
void fill_buffer(struct request_t *request) {
    pthread_mutex_lock(&queue_lock);

    if (queue_head == NULL) {
        queue_head = request;
    } else {
        struct request_t *aux = queue_head;

        while (aux->next_request != NULL) {
            aux = aux->next_request;
        }

        aux->next_request = request;
    }

    pthread_cond_signal(&queue_not_empty);
    pthread_mutex_unlock(&queue_lock);
}

int exec_write_operation(struct request_t *request) {
    pthread_mutex_lock(&tree_lock);

    if (request->op == 1) {
        int value = tree_put(tree, request->key, request->data);
        if (value == -1) {
            request->message->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            request->message->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            op_count++;

            pthread_mutex_unlock(&tree_lock);
            return value;
        }

        request->message->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
        request->message->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        request->message->result = last_assigned;
        op_count++;

        pthread_mutex_unlock(&tree_lock);
        return value;

    } else if (request->op == 0) {  // substituir por else?
        int value = tree_del(tree, request->key);

        if (value == -1) {
            request->message->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            request->message->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            op_count++;
            pthread_mutex_unlock(&tree_lock);
            return value;
        }
        request->message->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
        request->message->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        request->message->result = last_assigned;
        op_count++;

        pthread_mutex_unlock(&tree_lock);
        return value;
    }

    return -1;  // Nao eh esperado que entre aqui, nunca.
}