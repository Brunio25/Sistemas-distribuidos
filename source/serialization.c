// Grupo 4
// Renato Custódio nº56320
// Bruno Soares nº57100
// Guilherme Marques nº55472

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/serialization.h"

/* Serializa todas as keys presentes no array de strings keys
 * para o buffer keys_buf que será alocado dentro da função.
 * O array de keys a passar em argumento pode ser obtido através
 * da função tree_get_keys. Para além disso, retorna o tamanho do
 * buffer alocado ou -1 em caso de erro.
 */
int keyArray_to_buffer(char **keys, char **keys_buf) {
    if (keys == NULL || keys_buf == NULL) {
        return -1;
    }

    //obter o numero de chaves
    int size = 0 ;
    for(int i = 0;keys[i] != NULL; i++){
        size += sizeof(keys[i]);
    }
  
    *keys_buf  = (char *) malloc (size + sizeof (keys));
    //serializar as chaves

    int offset = 0;

    for(int i = 0; keys[i] != NULL; i++){
        char *temp = malloc(sizeof(keys[i]));
        sprintf(temp, "%ld", sizeof(keys[i]));

        strcat(*keys_buf,temp);

        strcat(*keys_buf,keys[i]); 

        free(temp);     
    }

    return sizeof(*keys_buf);
}

int readNumber(char *str, int *size);

/* De-serializa a mensagem contida em keys_buf, com tamanho
 * keys_buf_size, colocando-a e retornando-a num array char**,
 * cujo espaco em memória deve ser reservado. Devolve NULL
 * em caso de erro.
 */
char **buffer_to_keyArray(char *keys_buf, int keys_buf_size) {
   
    if (keys_buf == NULL || keys_buf_size < 0) {
        return NULL;
    }

    char **returnarray = (char **) malloc (keys_buf_size + 1);
    if (returnarray == NULL) {
        return NULL;
    }

    int offset = 0, pos = 0;
    char *temp;
    while (keys_buf[offset] != '\0') {
        int *length = 0;
        int size = readNumber(keys_buf + offset, length);
        offset += *length;
        returnarray[pos] = malloc(size);
        memcpy(returnarray[pos], &keys_buf[offset],size);
        pos++;
        offset += size;
    }

    return returnarray;
}


int readNumber(char *str, int *length) {
    char *nums = malloc(10);
    int pos = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (isdigit(str[i])) {
            nums[pos] = str[i];
            pos++;
            (*length) += 1;
        }
        else {
            break;
        }
    }
    
    int result = atoi(nums);
    free(nums);
    return result;
}

