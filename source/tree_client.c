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
#include "../include/data.h"

void printKeys(char **strs) {
    int i = 0;

    printf("Keys:");
    while (strs[i] != NULL) {
        printf(" %s", (char *)strs[i]);
        i++;
    }
    printf("\n");
}

void printValues(void **values) {
    int i = 0;

    printf("Values:");
    while (values[i] != NULL) {
        struct data_t *data = (struct data_t *)values[i];
        printf(" %s", (char *)data->data);
        i++;
    }
    printf("\n");
}

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

    if (rtree == NULL) {
        printf("erro na ligacao ao servidor\n");
        return -1;
    }

    while (1) {
        fgets(input, 19, stdin);
        input[strcspn(input, "\n")] = '\0';

        char *command;
        if (strchr(input, ' ') != NULL) {
            command = strtok(input, " ");
        } else {
            command = input;
        }

        if (strcmp(command, "put") == 0) {
            int bool;
            char *key = strtok(NULL, " ");
            char *data = strtok(NULL, " ");  // replicar por todos
            bool = rtree_put(rtree, entry_create(key, data_create2(strlen(data), data)));
            if (bool == -1) {
                printf("Problemas com o put\n");
            } else {
                printf("O put foi bem sucedido\n");
            }
        } else if (strcmp(command, "get") == 0) {
            char *key = strtok(NULL, "\n");
            struct data_t *data = rtree_get(rtree, key);
            if (data != NULL) {
                printf("data: %s\n", (char *)data->data);
            } else {
                printf("Não ha nenhum valor armazenado na arvore com a chave dada\n");
            }

        } else if (strcmp(command, "del") == 0) {
            char *key = strtok(NULL, "\n");
            if (rtree_del(rtree, key) == -1) {
                printf("Problemas com o delete\n");
            } else {
                printf("O delete foi bem sucedido\n");
            }

        } else if (strcmp(command, "size") == 0) {
            printf("size: %d\n", rtree_size(rtree));

        } else if (strcmp(command, "height") == 0) {
            printf("height: %d\n", rtree_height(rtree));

        } else if (strcmp(command, "getkeys") == 0) {
            printKeys(rtree_get_keys(rtree));

        } else if (strcmp(command, "getvalues") == 0) {
            printValues(rtree_get_values(rtree));

        } else if (strcmp(command, "quit") == 0) {
            break;
        } else {
            printf("comando invalido\n");
        }
    }

    rtree_disconnect(rtree);
    return 0;
}
