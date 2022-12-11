// Grupo 4
// Renato Custódio nº56320
// Bruno Soares nº57100
// Guilherme Marques nº55472

#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <zookeeper/zookeeper.h>
#include "tree_skel-private.h"

struct rtree_t {
    struct sockaddr_in socket; // endereço do outro servidor
    int socket_used;
    
    zhandle_t *zh;
    int is_connected;

    char *identifier ; // identificador do servidor
    char *identifier_other; //identificador do outro servidor
};

int connectServer(struct rtree_t *server, char *serverInfo);
#endif