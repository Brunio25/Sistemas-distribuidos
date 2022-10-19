#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

struct rtree_t {
    int sockfd;
    struct sockaddr_in *server;
    struct sockaddr_in *client;
};

#endif