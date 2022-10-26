# // Grupo 4
# // Renato Custódio nº56320
# // Bruno Soares nº57100
# // Guilherme Marques nº55472

#include "../include/client_stub.h"
#include "../include/client_stub-private.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
    char input[19];
    const char tre[100] = "127.0.0.1:1025";
    printf("Introduza um comando da Lista: \n");
    printf("put <key> <data>\n");
    printf("get <key>\n");
    printf("del <key>\n");
    printf("size\n");
    printf("height\n");
    printf("getkeys\n");
    printf("getvalues\n");
    printf("quit\n");
    printf("\n");

    struct rtree_t *rtree = rtree_connect(tre);
    while(1) {
        fgets(input, 19, stdin);
        
        if(rtree == NULL){
            printf("erro na ligacao ao servidor\n");
            return -1;
        }
        
        char *command;
        if (strchr(input, ' ') != NULL){
            command = strtok(input, " ");
        }else{
            strcpy(command,input);
            command[strlen(command)-1] = '\0';
        }
        
        if(strcmp(command, "put") == 0) {
            char *key = strtok(NULL, " ");
            char *data = strtok(NULL, " ");
            rtree_put(rtree, entry_create(key,data));
        } else if(strcmp(command, "get") == 0) {
            char *key = strtok(NULL, " ");
            rtree_get(rtree,key);
        } else if(strcmp(command, "del") == 0) {
            char *key = strtok(NULL, " ");
            rtree_del(rtree,key);
        } else if(strcmp(command,"size") == 0) {
            printf("size: %d\n",rtree_size(rtree));
        } else if(strcmp(command, "height") == 0) {
            printf("height: %d\n",rtree_height(rtree));
        } else if(strcmp(command, "getkeys") == 0) {           
            rtree_get_keys(rtree);
        } else if(strcmp(command,"getvalues") == 0) {
            rtree_get_values(rtree);
        } else if(strcmp(command, "quit") == 0) {
            break;
        } else{
            printf("comando invalido\n");
        }
        
    }
    rtree_disconnect(rtree);

    /* const char tre[100] = "127.0.0.1:1025";
    struct rtree_t *rtree = rtree_connect(tre);
    rtree_disconnect(rtree); */
    return 0;
}
