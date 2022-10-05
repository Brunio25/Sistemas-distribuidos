#include "../include/tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>

#include "../include/tree-private.h"

/* Função para criar uma nova árvore tree vazia.
 * Em caso de erro retorna NULL.
 */
struct tree_t *tree_create() {
    struct tree_t *new_tree = (struct tree_t *)malloc(sizeof(struct tree_t));

    if (new_tree != NULL) {
        new_tree->root = NULL;
        new_tree->left = NULL;
        new_tree->right = NULL;
    }
    
    return new_tree;
}

/* Função para libertar toda a memória ocupada por uma árvore.
 */
void tree_destroy(struct tree_t *tree) {
    if (tree == NULL) {
        return;
    }

    tree_destroy(tree->left);
    tree_destroy(tree->right);

    entry_destroy(tree->root);
    free(tree);
}

/* Função para adicionar um par chave-valor à árvore.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na árvore,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int tree_put(struct tree_t *tree, char *key, struct data_t *value) {
    struct entry_t * temp;
    if (tree == NULL) {
        return -1;
    }

    temp = entry_create(key, value);
    if (tree->root == NULL) {
        // tree->root = entry_create(key,value);
        tree->root = entry_dup(temp);
        free(temp);
        return 0;
    }

    //printf("key: %s, treeKey: %s\n", key, tree->root->key);
    int cmp = entry_compare(temp, tree->root);
    free(temp);
    
    if (cmp == 0) {
        data_destroy(tree->root->value);
        tree->root->value = data_dup(value);
        return 0;
    }

    if (cmp < 0) {
        if (tree->left == NULL) {
            tree->left = tree_create();
        }

        tree_put(tree->left, key, value);
    } else {
        if (tree->right == NULL) {
            tree->right = tree_create();
        }

        tree_put(tree->right, key, value);
    }
}

/* Função para obter da árvore o valor associado à chave key.
 * A função deve devolver uma cópia dos dados que terão de ser
 * libertados no contexto da função que chamou tree_get, ou seja, a
 * função aloca memória para armazenar uma *CÓPIA* dos dados da árvore,
 * retorna o endereço desta memória com a cópia dos dados, assumindo-se
 * que esta memória será depois libertada pelo programa que chamou
 * a função. Devolve NULL em caso de erro.
 */
struct data_t *tree_get(struct tree_t *tree, char *key) {
    if (tree == NULL) {
        return NULL;
    }

    if (tree->root == NULL) {
        return NULL;
    }

    int cmp = strcmp(key, tree->root->key);

    if (cmp == 0) {
        return data_dup(tree->root->value);
    }

    if (cmp < 0) {
        tree_get(tree->left, key);
    } else {
        tree_get(tree->right, key);
    }
}

/* Função para remover um elemento da árvore, indicado pela chave key,
 * libertando toda a memória alocada na respetiva operação tree_put.
 * Retorna 0 (ok) ou -1 (key not found).
 */
int tree_del(struct tree_t *tree, char *key) {
    if (tree == NULL) {
        return -1;
    }

    int cmp = strcmp(key, tree->root->key);

    if (cmp < 0) {
        tree_del(tree->left, key);
    }
    else if (cmp > 0) {
        tree_del(tree->left, key);
    }
    else {
        //No Children
        if (tree->left == NULL && tree->right == NULL) {
            tree_destroy(tree);
        }
        else if(tree->left == NULL || tree->right == NULL) {
            //One Child
            struct tree * temp;
            if (tree->left == NULL) {
                temp = tree->right;
                tree->right = NULL;
            }
            else {
                temp = tree->left;
                tree->left = NULL;
            }
            entry_destroy(tree->root);
            tree->root = temp;
        }
        else {
            //Two Children
            struct tree_t *temp = minValue(tree->right);
            tree->root = temp->root;
            tree->right = tree_del(tree->right, temp->root->key);
        }

        return 0;
    }
  
} 

struct tree_t* minValue(struct tree_t *tree) {
    if(tree == NULL)
        return NULL;
    else if(tree->left != NULL) 
        return minValue(tree->left);
    return tree;
}

/* Função que devolve o número de elementos contidos na árvore.
 */
int tree_size(struct tree_t *tree) {
    if (tree == NULL || tree->root == NULL) {
        return 0;
    }

    return tree_size(tree->left) + 1 + tree_size(tree->right);
}

/* Função que devolve a altura da árvore.
 */
/*int tree_height(struct tree_t *tree) {
    if (tree == NULL || tree->root == NULL) {
        return 0;
    }

    return 1 + max(tree_height(tree->left), tree_height(tree->right));
}*/

/* Função que devolve um array de char* com a cópia de todas as keys da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária. As keys devem vir ordenadas segundo a ordenação
 * lexicográfica das mesmas.
 */
char **tree_get_keys(struct tree_t *tree) {
    if (tree == NULL) {
        return NULL;
    }

    int size = tree_size(tree);
    char **keys = (char **)malloc((size + 1) * sizeof(char *));

    if (keys == NULL) {
        return NULL;
    }

    int i = 0;
    tree_get_keys_aux(tree, keys, &i);

    keys[size] = NULL;

    return keys;
    
}

void tree_get_keys_aux(struct tree_t *tree, char **keys, int *i) {
    if (tree == NULL) {
        return;
    }

    tree_get_keys_aux(tree->left, keys, i);

    keys[*i] = strdup(tree->root->key);
    (*i)++;

    tree_get_keys_aux(tree->right, keys, i);
}

/* Função que devolve um array de void* com a cópia de todas os values da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
void **tree_get_values(struct tree_t *tree) {
    return;
}

/* Função que liberta toda a memória alocada por tree_get_keys().
 */
void tree_free_keys(char **keys) {
    return;
}

/* Função que liberta toda a memória alocada por tree_get_values().
 */
void tree_free_values(void **values) {
    return;
}