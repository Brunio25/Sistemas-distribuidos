# // Grupo 4
# // Renato Custódio nº56320
# // Bruno Soares nº57100
# // Guilherme Marques nº55472


#include "../include/network_server.h"


int main(int argc, char const *argv[])
{
    int sockfd = network_server_init(1025);
    //tree_skel_init();
    network_main_loop(sockfd);

    return 0;
}