#include <assert.h>
#include <stdlib.h>
#include "tree.h"

Tree _NIL = {NULL};
Tree* const NIL = &_NIL;

Leaf* leaf_of(void* token, const char* tag) {
    Leaf* leaf = malloc(sizeof(Leaf)); assert(leaf);
    *leaf = (Leaf){(Tree){tag}, token};
    return leaf;
}

Node* node_of(Tree* children, int nb_children, const char* tag) {
    Node* node = malloc(sizeof(Node)); assert(node);
    *node = (Node){(Tree){tag}, children, nb_children};
    return node;
}

Branch* branch_of(Tree* child, const char* tag) {
    Branch* branch = malloc(sizeof(Branch)); assert(branch);
    *branch = (Branch){(Tree){tag}, child};
    return branch;
}
