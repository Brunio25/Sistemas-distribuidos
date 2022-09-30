#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../include/data.h"
#include "../include/entry.h"

/* Função que cria uma entry, reservando a memória necessária para a
 * estrutura e inicializando os campos key e value, respetivamente, com a
 * string e o bloco de dados passados como parâmetros, sem reservar
 * memória para estes campos.
 */
struct entry_t *entry_create(char *key, struct data_t *data) {
    struct entry_t *new_entry = (struct entry_t*) malloc (sizeof(struct entry_t));
    new_entry -> key = key;
    new_entry -> value = data;
    return new_entry;
}

/* Função que elimina uma entry, libertando a memória por ela ocupada
 */
void entry_destroy(struct entry_t *entry) {
    if(entry == NULL)
        return;
        
    data_destroy (entry -> value);
    free (entry);
}

/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 */
struct entry_t *entry_dup(struct entry_t *entry) {
    char* new_key = strdup (entry -> key);
    struct data_t *new_data = data_dup (entry -> value);
    struct entry_t *new_entry = entry_create (new_key, new_data);
    return new_entry;
}

/* Função que substitui o conteúdo de uma entrada entry_t.
*  Deve assegurar que destroi o conteúdo antigo da mesma.
*/
void entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value) {
    entry -> key = new_key;
    entry -> value = new_value;
    //NAO PASSA NOS TESTES DA STORA
    //data_replace (entry -> value, new_value -> datasize, new_value -> data);
}

/* Função que compara duas entradas e retorna a ordem das mesmas.
*  Ordem das entradas é definida pela ordem das suas chaves.
*  A função devolve 0 se forem iguais, -1 se entry1<entry2, e 1 caso contrário.
*/
int entry_compare(struct entry_t *entry1, struct entry_t *entry2) { 
    return strcmp (entry1 -> key, entry2 -> key); 
}