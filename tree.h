#pragma once
#include <stdlib.h>

typedef struct {
    const char* tag;
} Tree;

extern Tree* const NIL;

typedef struct {
    Tree super;
    void* token;
} Leaf;

Leaf* leaf_of(void* token, const char* tag);

typedef struct {
    Tree super;
    Tree* children;
    int nb_children;
} Node;

Node* node_of(Tree* children, int nb_children, const char* tag);

typedef struct {
    Tree super;
    Tree* child;
} Branch;

Branch* branch_of(Tree* child, const char* tag);