#// Grupo 4
#// Renato Custódio nº56320
#// Bruno Soares nº57100
#// Guilherme Marques nº55472

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/client_stub-private.h"
#include "../include/client_stub.h"

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

    
    while (1) {
        struct rtree_t *rtree = rtree_connect(tre);
        fgets(input, 19, stdin);

        if (rtree == NULL) {
            printf("erro na ligacao ao servidor\n");
            return -1;
        }

        char *command;
        if (strchr(input, ' ') != NULL) {
            command = strtok(input, " ");
        } else {
            command = strdup(input);
            command[strlen(command) - 1] = '\0';
        }

        if (strcmp(command, "put") == 0) {
            int bool;
            char *key = strtok(NULL, " ");
            char *data = strtok(NULL, "\n");  // replicar por todos
            bool = rtree_put(rtree, entry_create(key, data_create2(strlen(data), data)));
            if(bool == -1){
                printf("Problemas com o put\n");
            }else{
                printf("O put foi bem sucedido\n");
            }
        } else if (strcmp(command, "get") == 0) {

            char *key = strtok(NULL, "\n");
            struct data_t *data = rtree_get(rtree, key);
            if(data != NULL){
                printf("data: %s\n", (char *)data->data);
            }else{
                printf("Não ha nenhum valor armazenado na arvore com a chave dada\n");
            }
            
        } else if (strcmp(command, "del") == 0) {

            char *key = strtok(NULL, " ");
            rtree_del(rtree, key);

        } else if (strcmp(command, "size") == 0) {

            printf("size: %d\n", rtree_size(rtree));

        } else if (strcmp(command, "height") == 0) {

            printf("height: %d\n", rtree_height(rtree));

        } else if (strcmp(command, "getkeys") == 0) {

           printf("mem: %p\n",rtree_get_keys(rtree));

        } else if (strcmp(command, "getvalues") == 0) {

            rtree_get_values(rtree);

        } else if (strcmp(command, "quit") == 0) {

            rtree_disconnect(rtree);
            break;

        } else {

            printf("comando invalido\n");

        }

        rtree_disconnect(rtree);
    }
    

    return 0;
}
