#include "../include/network_server.h"


int main(int argc, char const *argv[])
{
    int sockfd = network_server_init(1025);
    network_main_loop(sockfd);

    return 0;
}
