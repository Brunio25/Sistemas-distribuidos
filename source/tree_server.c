#// Grupo 4
#// Renato Custódio nº56320
#// Bruno Soares nº57100
#// Guilherme Marques nº55472

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "network_server.h"

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("Usage: ./binary/tree-server <serverPort> <IP>:<port>\n");
        return -1;
    }
    
    int sockfd = network_server_init(atoi(argv[1]));
    if (tree_skel_init(strdup(argv[1])) == -1) {
        return -1;
    } 
    network_zookeeper_init(strdup(argv[2]),strdup(argv[1]));
    network_main_loop(sockfd);
    network_server_close();

    return 0;
}
