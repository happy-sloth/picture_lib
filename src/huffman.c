#include "huffman.h"


huffman_tree_t *huffman_tree_create(uint8_t class, uint8_t id, uint8_t *codes_cnt_array, uint8_t *values, uint8_t values_cnt)
{
    huffman_tree_t *tree = calloc(1, sizeof(huffman_tree_t));

    uint8_t b_codes_cnt[16];
    memcpy(b_codes_cnt, codes_cnt_array, 16*sizeof(uint8_t));

    int codes_cnt_index = 0, value_cnt = 0;
    tree_node_t *current_node = tree->tree;
    for(int i = 0; i < values_cnt; i++){
        if (!current_node->left){
            current_node->left = calloc(1, sizeof(tree_node_t));

        } else {
            current_node = current_node->left;
        }
    }


    return tree;
}

