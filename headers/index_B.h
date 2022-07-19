#ifndef INDEX_B 
#define INDEX_B

#include<stdio.h>
#include"record.h"
#include"index.h"

#define TREE_ORDER 4
#define NoPROMOTED 1 
#define PROMOTED 2

// Cabecalho do arquivo para arvore B
typedef struct b_tree B_TREE;
// Nó da arvore B
typedef struct node_t NODE_T;

/**
 * @brief Cria um arquivo de indice de arvore B
 * 
 * @param bin_file    Arquivo de dados de onde serao retirados os registros 
 * @param index_file  Arquivo de indice onde sera escrito a arvore
 * @param type_file   Tipo de registro presente no arquivo de dados
 * @return int        -2 arquivos corrompidos/1 execucao correta 
 */
int create_b_tree_index(FILE* bin_file, FILE* index_file, int type_file);

/**
 * @brief Busca recursivamente determinado ID no arquivo de indice no formato de arvore B
 * 
 * @param index_file    Arquivo de indice
 * @param type_file     Tipo do registro contido no arquivo de dados
 * @param id            ID que sera buscado
 * @param b             Struct da arvore B
 * @return int          -1 caso nao encontre / RRN/BOS caso encontre
 */
int search_b(FILE* index_file, int type_file, int id, int rrn);

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
 *                      1 Nao ha promocao (split) - 
 *                      2 Ha promocao (split)
 */
int insert_b(FILE* ind, B_TREE* b, INDEX* key, int curr_rrn, 
            int* promo_child, INDEX* promo_key, int type_file);

// Le o cabeçalho de arquivo de indice arvore B
B_TREE* read_header_b_tree(FILE* index_file, int type_file);

// Atualiza no arquivo, o cabelho do arquivo, baseado nos dados em $(b)
void update_header_b(FILE* index_file, B_TREE* b);

// Cria uma nova raiz para a avore B, com a chave $(key) e filhos $(left) e $(right)
void create_root(FILE* index_file, B_TREE* b, INDEX* key, int left, int right, int type_file);

int get_root_node(B_TREE* b_tree);

#endif