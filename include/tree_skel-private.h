// Grupo 4
// Renato Custódio nº56320
// Bruno Soares nº57100
// Guilherme Marques nº55472

#ifndef _TREE_SKEL_PRIVATE_H
#define _TREE_SKEL_PRIVATE_H

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <zookeeper/zookeeper.h>

struct op_proc {
    int max_proc;
    int *in_progress;
};

struct request_t {
    int op_n; //o número da operação
    int op; //a operação a executar. op=0 se for um delete, op=1 se for um put
    char* key; //a chave a remover ou adicionar
    struct data_t *data; // os dados a adicionar em caso de put, ou NULL em caso de delete
    struct request_t *next_request;
    struct _MessageT *message;
};

int exec_write_operation(struct request_t *request);

void fill_buffer(struct request_t *request);

int start_zookeeper(char *zookeeper_addr, char * port);

void watcher_func(zhandle_t *zzh, int type, int state, const char *path, void* context);

#endif