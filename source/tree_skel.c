#// Grupo 4
#// Renato Custódio nº56320
#// Bruno Soares nº57100
#// Guilherme Marques nº55472

#include "tree_skel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"

struct tree_t *tree;
int last_assigned;

/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke().
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int tree_skel_init() {
    tree = tree_create();

    if (tree == NULL) {
        return -1;
    }
    last_assigned = 1;
    return 0;
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy() {
    tree_destroy(tree);
    return;
}

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao incializada)
 */
int invoke(struct _MessageT *msg) {
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
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        tree_del(tree, msg->key);

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
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        struct data_t *data = data_create2(msg->entry->value->datasize, msg->entry->value->data);
        if (data == NULL) {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }

        if ((tree_put(tree, msg->entry->key, data)) == -1) {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }

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

            msg->values[i]->data = values[i]->data;  //bug ? put a b + put a b = merda
            //printf("msg: %s, func: %s\n",msg->values[i]->data,(char *)values[i]->data);
            msg->values[i]->datasize = values[i]->datasize;
            i++;
        }

    } else {
        msg->opcode = MESSAGE_T__OPCODE__OP_BAD;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        //last_assigned++;                                          em caso de erro tmb é suposto?
        return -1;
    }


    last_assigned++;
    return 0;
}
