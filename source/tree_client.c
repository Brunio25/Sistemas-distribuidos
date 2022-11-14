#// Grupo 4
#// Renato Custódio nº56320
#// Bruno Soares nº57100
#// Guilherme Marques nº55472

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client_stub-private.h"
#include "client_stub.h"
#include "data.h"
#include "sdmessage.pb-c.h"
#include "tree_client-private.h"
#include "tree.h"

void printKeys(char **strs) {
    int i = 0;

    printf("Keys:");
    while (strs[i] != NULL) {
        printf(" %s", strs[i]);
        i++;
    }
    printf("\n");
}

void printValues(void **values) {
    int i = 0;

    printf("Values:");
    while (values[i] != NULL) {
        printf(" %s", (char *)values[i]);
        i++;
    }
    printf("\n");
}

void free_keys(char **keys) {
    int i = 0;
    while (keys[i] != NULL) {
        free(keys[i]);
        i++;
    }

    free(keys);
}


void free_values(void **values) {
    int i = 0;
    while (values[i] != NULL) {
        printf("%p\n",values[i]);
        free(values[i]);
        i++;
    }

    free(values);
}

void usage() {
    printf("Insert one of the below commands: \n");
    printf("\tput <key> <data>\n");
    printf("\tget <key>\n");
    printf("\tdel <key>\n");
    printf("\tsize\n");
    printf("\theight\n");
    printf("\tgetkeys\n");
    printf("\tgetvalues\n");
    printf("\tquit\n");
    printf("\n");
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Usage: ./tree-client <server_IP:server_port>\n");
        return -1;
    }

    char *isValid = strchr(argv[1], ':');

    if (isValid == NULL) {
        printf("Usage: ./tree-client <server_IP:server_port>\n");
        return -1;
    }

    char input[100];
    signal(SIGPIPE, SIG_IGN);

    struct rtree_t *rtree = rtree_connect(argv[1]);

    if (rtree == NULL) {
        printf("erro na ligacao ao servidor\n");
        return -1;
    }

    usage();
    while (1) {
        fgets(input, 100, stdin);
        input[strcspn(input, "\n")] = '\0';

        char *command;
        if (strchr(input, ' ') != NULL) {
            command = strtok(input, " ");
        } else {
            command = input;
        }

        if (strcmp(command, "put") == 0) {
            char *key = strtok(NULL, " ");
            char *data = strtok(NULL, " ");

            int bool = rtree_put(rtree, entry_create(key, data_create2(strlen(data), data)));
            if (bool == -1) {
                printf("Insertion on Remote Tree Failed.\n");
            } else {
                printf("Insertion on Remote Tree Successful!\n");
            }
        } else if (strcmp(command, "get") == 0) {
            char *key = strtok(NULL, "\n");
            struct data_t *data = rtree_get(rtree, key);
            if (data->data != NULL) {
                printf("Fetched Data: %s\n", (char *)data->data);
            } else {
                printf("There isn't a entry of key: %s on the Remote Tree.\n", key);
            }
            data_destroy(data);
            
        } else if (strcmp(command, "del") == 0) {
            char *key = strtok(NULL, "\n");
            if (rtree_del(rtree, key) == -1) {
                printf("Deletion on Remote Tree Failed.\n");
            } else {
                printf("Deletion from Remote Tree Successful!\n");
            }

        } else if (strcmp(command, "size") == 0) {
            printf("Size: %d\n", rtree_size(rtree));

        } else if (strcmp(command, "height") == 0) {
            printf("Height: %d\n", rtree_height(rtree));

        } else if (strcmp(command, "getkeys") == 0) {
            char **keys = rtree_get_keys(rtree);
            if(keys == NULL){
                printf("Tree is empty\n");
            }else{
                printKeys(keys);
                free_keys(keys);
            }
            
        } else if (strcmp(command, "getvalues") == 0) {
            void **values = rtree_get_values(rtree);
            if(values == NULL){
                printf("Tree is empty\n");
            }else {
                printValues(values);
                //free_values(values);
            }
            
        } else if (strcmp(command, "quit") == 0) {
            printf("Connection Will Now Be Terminated!\n");
            break;
        } else {
            printf("Invalid Command\n");
            usage();
        }
    }

    rtree_disconnect(rtree);
    return 0;
}
