#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "entry.h"
#include "tree.h"

struct tree_t {
    /** a preencher pelo grupo */
    struct entry_t *root;
    struct tree_t *left;
    struct tree_t *right;
};

#endif
