#include "../include/client_stub.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
    char input[19];
    printf("Introduza o comando: \n");
    fgets(input, 19, stdin);

    char *command = strtok(input, " ");

    if(strcmp(command, "put") == 0) {
        char *arg1 = strtok(NULL, " ");

    } else if(strcmp(command, "get") == 0) {

    } else if(strcmp(command, "del") == 0) {

    } else if(strcmp(command,"size") == 0) {

    } else if(strcmp(command, "height") == 0) {

    } else if(strcmp(command, "getkeys") == 0) {

    } else if(strcmp(command,"getvalues") == 0) {
        
    } else if(strcmp(command, "quit") == 0) {
        
    } else{
        printf("comando invalido\n");
    }



    const char tre[100] = "127.0.0.1:1025";
    struct rtree_t *rtree = rtree_connect(tre);
    rtree_disconnect(rtree);
    return 0;
}
