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
struct op_proc *operations;
int last_assigned;
int maxNumOps = 40;
pthread_t *threadids;
pthread_mutex_t tree_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
int NumThreads;
struct request_t *queue_head;
int close = 0;

/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke().
 * A função deve lançar N threads secundárias responsáveis por atender
 * pedidos de escrita na árvore.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int tree_skel_init(int N) {
    operations = malloc(sizeof(struct op_proc));
    tree = tree_create();

    if (tree == NULL) {
        return -1;
    }

    operations->in_progress = calloc(maxNumOps, sizeof(int));
    operations->max_proc = 0;
    queue_head = NULL;

    int i;
    threadids = malloc(sizeof(pthread_t) * N);
    for (i = 0; i < N; i++) {
        pthread_create(&threadids[i], NULL, &process_request, NULL);
    }

    last_assigned = 1;

    return 0;
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy() {
    close = 1;
    pthread_cond_broadcast(&queue_not_empty);
    free(operations->in_progress);
    free(operations);
    tree_destroy(tree);
    free(threadids);
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
    while (close == 0) {
        pthread_mutex_lock(&queue_lock);
        if (queue_head == NULL) {
            pthread_cond_wait(&queue_not_empty, &queue_lock);
        }
        if (close != 0) {
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
        free(request->key);
        data_destroy(request->data);
        pthread_mutex_unlock(&tree_lock);
        return value;

    } else if (request->op == 0) {  // Se a operacao = 0 eh delete
        int value = tree_del(tree, request->key);

        if (value == -1) {  // Erro no tree_del
            pthread_mutex_unlock(&tree_lock);
            return value;
        }
        free(request->key);
        pthread_mutex_unlock(&tree_lock);
        return value;
    }

    return -1;  // Retorna este valor caso a operacao nao seja valida (nem eh put, nem eh delete)
}