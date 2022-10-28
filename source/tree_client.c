#// Grupo 4
#// Renato Custódio nº56320
#// Bruno Soares nº57100
#// Guilherme Marques nº55472

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/client_stub-private.h"
#include "../include/client_stub.h"
#include "../include/data.h"
#include "../include/sdmessage.pb-c.h"

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
        printf(" %s", (char *)((MessageT__Data *)values[i])->data);
        i++;
    }
    printf("\n");
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
    signal(SIGPIPE, SIG_IGN);
    char input[19];
    const char tre[100] = "127.0.0.1:1025";

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
            char *key = strtok(NULL, " ");
            char *data = strtok(NULL, " ");

            int bool = rtree_put(rtree, entry_create(key, data_create2(strlen(data), data)));  // memory leak?
            if (bool == -1) {
                printf("Insertion on Remote Tree Failed.\n");
            } else {
                printf("Insertion on Remote Tree Successful!\n");
            }
        } else if (strcmp(command, "get") == 0) {
            char *key = strtok(NULL, "\n");
            struct data_t *data = rtree_get(rtree, key);
            if (data != NULL) {
                printf("Fetched Data: %s\n", (char *)data->data);
            } else {
                printf("There isn't a entry of key: %s on the Remote Tree.\n", key);
            }

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
            printKeys(rtree_get_keys(rtree));

        } else if (strcmp(command, "getvalues") == 0) {
            printValues(rtree_get_values(rtree));  // mem leak

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
