#ifndef INDEX_B 
#define INDEX_B

#include<stdio.h>
#include"record.h"
#include"index.h"

#define TREE_ORDER 4

typedef struct b_tree B_TREE;
typedef struct node_t NODE_T;

int create_b_tree_index(FILE* bin_file, FILE* index_file, int type_file);

/**
 * @brief  Insere uma chave (id + referencia do registro no arquivo de dados) na arvore B
 * 
 * @param ind           Arquivo do indice arvore B 
 * @param key           Chave que sera adicionada
 * @param curr_rrn      RRN do nó que esta sendo acessado na recursao
 * @param promo_key     Chave que sera promovida caso haja split
 * @param promo_child   RRN do filho da direita ao no promovido
 * @param type_file     Tipo de arquivo, 1->Registo estático, 2->Registro de tam variavel
 * @return int ->       -1 Chave duplicada - 
 *                      1 Ha promocao (split) - 
 *                      2 Nao ha promocao (split)
 */
int insert(FILE* ind, INDEX* key, int curr_rrn, int* promo_child, INDEX* promo_key, int type_file);
      
void split(FILE* index_file, INDEX* key, INDEX* promo_key, int p_b_rrn, 
          int* promo_child, NODE_T* node_t, NODE_T* new_node_t,  int type_file); 

int find_key_in_node(NODE_T* node_t, int key, int type_file);

NODE_T read_node(FILE* index_file, int type_file);

void insert_in_node(INDEX* key, int r_child, NODE_T* node, int type_file);

int get_next_rrn(FILE* index_file);

void jump_to_root(FILE* index_file, B_TREE* b, int type_file);

void create_root(FILE* index_file, B_TREE* b, INDEX* key, int left, int right, int type_file);

void jump_to_node_b(FILE* index_file, int rrn, int type_file);

void write_node(FILE* index_file, int rrn, int type_file, NODE_T node);

void print_node(NODE_T* node, int type_file);

#endif