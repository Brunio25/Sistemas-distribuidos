#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "entry.h"
#include "tree.h"

struct tree_t {
    struct entry_t *root;
    struct tree_t *left;
    struct tree_t *right;
};

struct tree_t* minValue(struct tree_t *tree);

struct tree_t *tree_del_aux(struct tree_t *tree, char *key);
#endif
