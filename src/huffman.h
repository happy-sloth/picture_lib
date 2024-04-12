
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include <stdio.h>
#include <stdint.h>

typedef struct tree_node{
    int value;
    bool leaf_node;
    struct tree_node *left;
    struct tree_node *right;
} tree_node_t;

typedef struct huffman_tree {
    uint8_t id; // tree id
    uint8_t tree_class; // AC if 1 or DC if 0
    tree_node_t *tree;
} huffman_tree_t;


huffman_tree_t *huffman_tree_create(uint8_t tree_class, uint8_t id, uint8_t *codes_cnt_array, uint8_t *values, uint8_t values_cnt);
