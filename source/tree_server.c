#// Grupo 4
#// Renato Custódio nº56320
#// Bruno Soares nº57100
#// Guilherme Marques nº55472

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "network_server.h"

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("Usage: ./server <server_port> <N>\n");
        return -1;
    }

    for (int i = 0; argv[1][i] != '\0'; i++) {
        if (!isdigit(argv[1][i])) {
            printf("Invalid Usage.");
            printf("Usage:\n \t./tree-server <server_port>\n");
            return -1;
        }
    }
    
    int sockfd = network_server_init(atoi(argv[1]));
    if (tree_skel_init(atoi(argv[2])) == -1) {                  // numero de threads secundarias
        return -1;
    } 
    network_main_loop(sockfd);
    network_server_close();

    return 0;
}
