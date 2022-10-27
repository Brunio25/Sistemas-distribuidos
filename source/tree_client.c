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
    printf("\tput <key> <data>\n");
    printf("\tget <key>\n");
    printf("\tdel <key>\n");
    printf("\tsize\n");
    printf("\theight\n");
    printf("\tgetkeys\n");
    printf("\tgetvalues\n");
    printf("\tquit\n");
    printf("\n");

    struct rtree_t *rtree = rtree_connect(tre);
    while(1) {
        fgets(input, 19, stdin);
        
        if(rtree == NULL) {
            printf("erro na ligacao ao servidor\n");
            return -1;
        }
       
        char *command;
        if (strchr(input, ' ') != NULL) {
            command = strtok(input, " ");
        }
        else {
            strcpy(command,input);
            command[strlen(command)-1] = '\0';
        }
        
        if(strcmp(command, "put") == 0) {
            char *key = strtok(NULL, " ");
            char *data = strtok(NULL, " ");
            rtree_put(rtree, entry_create(key,data_create2(strlen(data), data)));
        } else if(strcmp(command, "get") == 0) {
            char *key = strtok(NULL, " ");
            struct data_t *data = rtree_get(rtree,key);
            printf("data: %s\n",(char *)data->data);
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

    return 0;
}
