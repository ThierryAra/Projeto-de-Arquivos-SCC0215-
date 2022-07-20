#ifndef INDEX_B 
#define INDEX_B

#include<stdio.h>
#include"record.h"
#include"index.h"

#define TREE_ORDER 4
#define NoPROMOTED 1 
#define PROMOTED 2

// File header for tree B
typedef struct b_tree B_TREE;
// Tree node B
typedef struct node_t NODE_T;

/**
 * @brief Create a B-tree index file
 * 
 * @param bin_file    Data file of a will be removed the records
 * @param index_file  Index file where the tree will be written
 * @param type_file   Record type present in the data file
 * @return int        -2 corrupt files / 1 correct execution
 */
int create_b_tree_index(FILE* bin_file, FILE* index_file, int type_file);

/**
 * @brief Recursively fetches given ID in index file in B-tree format
 * 
 * @param index_file    Index file
 * @param type_file     Record type contained in the data file
 * @param id            ID that will be searched
 * @param b             B tree struct
 * @return int          -1 if not found / RRN/BOS if found
 */
int search_b(FILE* index_file, int type_file, int id, int rrn);

/**
 * @brief  Insert a key (id + record reference in the data file) in the B tree
 * 
 * @param ind           B-tree index file
 * @param key           Key to be added
 * @param curr_rrn      RRN of the node being accessed in the recursion
 * @param promo_key     Key that will be promoted in case of split
 * @param promo_child   RRN from right child to promoted no
 * @param type_file     File type, 1->Static log, 2->Variable size log
 * @return int ->       -1 duplicate key - 
 *                      1 there is no promotion (split) - 
 *                      2 There is promotion (split)
 */
int insert_b(FILE* ind, B_TREE* b, INDEX* key, int curr_rrn, 
            int* promo_child, INDEX* promo_key, int type_file);

// Read B-tree index file header
B_TREE* read_header_b_tree(FILE* index_file, int type_file);

// Updates in the file, the header of the file, based on the data in $(b)
void update_header_b(FILE* index_file, B_TREE* b);

// Create a new root for tree B, with the key $(key) and children $(left) and $(right)
void create_root(FILE* index_file, B_TREE* b, INDEX* key, int left, int right, int type_file);

int get_root_node(B_TREE* b_tree);

#endif