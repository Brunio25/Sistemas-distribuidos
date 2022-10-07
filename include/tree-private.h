// Grupo 4
// Renato Custódio nº56320
// Bruno Soares nº57100
// Guilherme Marques nº55472

#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "entry.h"
#include "tree.h"

struct tree_t {
    struct entry_t *root;
    struct tree_t *left;
    struct tree_t *right;
};

//Gets the smallest node from a tree
struct tree_t *minValue(struct tree_t *tree);

//helps the tree_del function with the deletion of a node
//by recursively deleting the node
struct tree_t *tree_del_aux(struct tree_t *tree, char *key);

//Given two ints returns the biggest one
int max(int a, int b);

//resursively gets all the keys inside the nodes from a tree 
//and puts them in an array
void tree_get_keys_aux(struct tree_t *tree, char **keys, int *i);

//resursively gets all the values inside the nodes from a tree 
//and puts them in an array
void tree_get_values_aux(struct tree_t *tree, char **values, int *i);

#endif
