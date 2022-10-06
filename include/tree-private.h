#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "entry.h"
#include "tree.h"

struct tree_t {
    struct entry_t *root;
    struct tree_t *left;
    struct tree_t *right;
};

struct tree_t *minValue(struct tree_t *tree);

struct tree_t *tree_del_aux(struct tree_t *tree, char *key);

int max(int a, int b);

void tree_get_keys_aux(struct tree_t *tree, char **keys, int *i);

void tree_get_values_aux(struct tree_t *tree, char **values, int *i);

#endif
