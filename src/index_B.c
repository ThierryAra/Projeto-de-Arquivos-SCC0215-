#include<stdio.h>
#include<stdlib.h>
#include"../headers/index_B.h"

#define ROOT_NODE '0'
#define INTER_NODE '1' 
#define LEAF_NODE '2'

#define NoPROMOTED 1 
#define PROMOTED 2

#define MAX_KEYS TREE_ORDER-1
#define MIN_KEYS TREE_ORDER/2

#define STATIC_INDEX_B 45
#define VARIABLE_INDEX_B 56

typedef struct b_tree{
    int order;
    int status;
    int root_node;
    int nextRRN;
    int qtt_nodes;
}B_TREE;

typedef struct node_t{
    char type;
    int qtt_keys;
    int c[TREE_ORDER - 1];
    long int pr[TREE_ORDER - 1];
    int p[TREE_ORDER];
}NODE_T;                                                                 

int insert(FILE* ind, INDEX* key, int curr_rrn, int* 
          promo_child, INDEX* promo_key, int type_file);
          
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
//---------------------------------------------------------
B_TREE* initialize_b_tree(int order){
    B_TREE* b = malloc(sizeof(B_TREE));

    b->order = TREE_ORDER;
    b->nextRRN = 0;
    b->qtt_nodes = 0;
    b->status = 0;
    b->root_node = -1;

    return b;
}

NODE_T initialize_node(char node_type){
    NODE_T node_t;
    node_t.type = node_type;
    node_t.qtt_keys = 0;

    int i;
    for (i = 0; i < TREE_ORDER - 1; i++){
        node_t.c[i]  = -1;
        node_t.p[i]  = -1;
        node_t.pr[i] = -1;   
    }

    node_t.p[i] = -1;
}

void create_b_tree_file(B_TREE* b, FILE* index_file, int type_file){
    //Escrevendo o cabecalho
    fwrite(&b->status, 1, sizeof(int),index_file);
    fwrite(&b->root_node, 1, sizeof(int),index_file);
    fwrite(&b->nextRRN, 1, sizeof(int),index_file);
    fwrite(&b->qtt_nodes, 1, sizeof(int),index_file);

    // preenchendo o cabecalho com lixo para obter o mesmo tamanho dos nos
    int qtt = 0;
    if(type_file == 1) qtt = 32;
    else               qtt = 44;

    for (int i = 0; i < qtt; i++)
        fwrite("$", 1, sizeof(char), index_file); 

    INDEX* root = initialize_index();
    create_root(index_file, b, root, -1, -1, type_file);
    free_index(root);
}

int create_b_tree_index(FILE* bin_file, FILE* index_file, int type_file){
    if(index_file == NULL || bin_file == NULL)
        return -2;

    B_TREE* b = initialize_b_tree(TREE_ORDER);
    create_b_tree_file(b, index_file, type_file);

    RECORD* r = create_record();
    HEADER* h = create_header();
    
    ignore_header(bin_file, type_file);
    
    int promo_child = -1;
    INDEX* promo_key = initialize_index();
    int res = 0;
    int rrn = 0;
    long int BOS;
    int p = -1;

    // ler arquivo de dados e obter as chaves
    while((res = get_record(bin_file, r, h, type_file)) != -2){
        if(res != -1){
            BOS = ftell(bin_file);  

            INDEX index;
            index.id = get_id(r);
            if(type_file == 1) index.rrn = rrn;
            else               index.BOS = BOS;
            p = insert(index_file, &index, b->root_node, &promo_child, promo_key, type_file); 
            if(p == PROMOTED)
                create_root(index_file, b, promo_key, b->root_node, promo_child, type_file);
                

            promo_child = -1;
        }

        rrn++;
    }
}

/**
 * @brief 
 * 
 * @param ind 
 * @param key 
 * @param curr_rrn 
 * @param promo_child 
 * @param promo_key 
 * @param type_file 
 * @return int 
 */
int insert(FILE* ind, INDEX* key, int curr_rrn, int* promo_child, INDEX* promo_key, int type_file){
    //Armazena onde sera inserida a nova chave e o rrn do nos recursivos na busca da posicao
    int next_node = -1;
    //chave promovida
    INDEX* p_b_key = initialize_index();
    //novo RRN criado caso haja promocao de chave (split)
    int p_b_rrn = 0;
    NODE_T node_t;


    if(curr_rrn == -1){
        *promo_key   = *key;
        *promo_child = -1;

        free_index(p_b_key);
        return PROMOTED;
    }
    
    node_t = read_node(ind, type_file);
    next_node = find_key_in_node(&node_t, key->id, type_file);

    // duplicated key
    if(next_node != -2){
        free_index(p_b_key);
        return -1;
    }

    //rrn do proximo nó da arvore
    p_b_rrn = next_node;
    int return_value = insert(ind, key, curr_rrn, &next_node, p_b_key, type_file);

    // Caso a recursão encontre a chave na arvore ou nao haja promocao
    if(return_value != PROMOTED){
        free_index(p_b_key);
        return return_value;
    
    // Há espaco para a insercao da chave no nó 
    }else if(node_t.qtt_keys < TREE_ORDER - 1){
        insert_in_node(key, 0, &node_t, type_file);
        
        free_index(p_b_key);
        return NoPROMOTED;

    // Nó cheio, deve haver split
    }else{
        NODE_T new_node_t;
        split(ind, p_b_key, promo_key, p_b_rrn, promo_child, &node_t, &new_node_t, type_file);
        
        free_index(p_b_key);
        return PROMOTED;
    }
}

NODE_T read_node(FILE* index_file, int type_file){
    NODE_T node_t;
    
    fread(&node_t.type, sizeof(int), 1, index_file);
    fread(&node_t.qtt_keys, sizeof(int), 1, index_file);
    
    for (int i = 0; i < TREE_ORDER - 1; i++){
        fread(&node_t.c[i], sizeof(int), 1, index_file); //key[i]
        //reference to key[i] in data_file
        if(type_file == 1) fread(&node_t.pr[i], sizeof(int), 1, index_file);  
        else               fread(&node_t.pr[i], sizeof(long int), 1, index_file); 
    }
    
    for (int i = 0; i < TREE_ORDER; i++)
        fread(&node_t.p[i], sizeof(int), 1, index_file); //reference sub-tree

    return node_t;
}

int find_key_in_node(NODE_T* node_t, int key, int type_file){
    int next_node = -1;
    //buscando pela chave no nó atual
    for (int i = 0; i < TREE_ORDER - 1; i++){
        if(node_t->c[i] == key)
            return -2;

        //caso nao encontre a chave, next_node contera o RRN do proximo nó na arvore
        else if(next_node == -1 && (node_t->c[i] == -1 || key < node_t->c[i]))
            if(node_t->c[i] == -1)
                next_node = node_t->p[i - 1];
            else if(key < node_t->c[i])
                next_node = node_t->p[i];
    }
    
    if(next_node == -1)
        next_node = node_t->p[TREE_ORDER - 1];

    return next_node;
} 

/* INDEX* find_key_in_node(NODE_T node_t, int key, int type_file){
    INDEX* index = initialize_index();

    //buscando pela chave no nó atual
    for (int i = 0; i < TREE_ORDER - 1; i++){
        if(node_t.c[i] == key){
            index->id = node_t.c[i];
            if(type_file == 1) index->rrn = node_t.pr[i];
            else               index->BOS = node_t.pr[i];
        
            return index;
        }
        //caso nao encontre a chave, index->rrn contera o RRN do proximo nó na arvore
        else if(index->rrn == -1 && (node_t.c[i] == -1 || key < node_t.c[i])){
            index->rrn = node_t.p[i - 1];
        }
    }
    
    if(index->rrn == -1)
        index->rrn = node_t.p[TREE_ORDER - 1];

    return index;
} */

void split(
    FILE* index_file,
    INDEX* key,         /*Chave que sera adicionada    */
    INDEX* promo_key,   /*Chave que sera promovida     */
    int p_b_rrn,        /*RRN do novo nó               */
    int* promo_child,   /*rrn de referencia da direita */
    NODE_T* node_t,     /*RRN que sera promovido       */
    NODE_T* new_node_t, /*no que sera criado           */
    int type_file       /*tipo de arquivo              */
){
    int i;
    int mid;
    INDEX keys[MAX_KEYS+1];
    int childs[MAX_KEYS+2];

    //Adicionando todas as chaves em um vetor para fazer a distribuicao homoegena
    for (i = 0; i < MAX_KEYS; i++){
        keys[i].id = node_t->c[i];
        keys[i].BOS = node_t->pr[i];

        childs[i] = node_t->p[i];
    }
    childs[i] = node_t->p[i];

    
    i = MAX_KEYS;
    //movendo as chaves para manter ordenados apos inserir a nova chave
    while(keys[i].id < node_t->c[i-1] && i > 0){
        keys[i].id  = keys[i - 1].id;
        if(type_file == 1) keys[i].rrn = keys[i - 1].rrn;
        else               keys[i].BOS = keys[i - 1].BOS;
        
        childs[i + 1] = childs[i];
        i--;
    }
    
    //Adicionando a nova chave no vetor ordenado
    keys[i].id  = key->id;
    if(type_file == 1) keys[i].rrn = keys[i - 1].rrn;
    else               keys[i].BOS = keys[i - 1].BOS;
    childs[i+1] = p_b_rrn;

    //criando um novo no
    *promo_child = get_next_rrn(index_file);
    *new_node_t = initialize_node(node_t->type);

    //Realizando a distribuicao entre os dois nos
    for (i = 0; i < MIN_KEYS; i++){
        node_t->c[i] = keys[i].id;
        node_t->p[i] = childs[i];

        new_node_t->c[i] = keys[i+1+MIN_KEYS].id;
        new_node_t->p[i] = childs[i];
        
        if(type_file == 1){
            node_t->pr[i]     = keys[i - 1].rrn;
            new_node_t->pr[i] = keys[i - 1].rrn;
        }
        else{
            node_t->pr[i]     = keys[i - 1].BOS;
            new_node_t->pr[i] = keys[i - 1].BOS;
        }   

        node_t->c[i + MIN_KEYS]  = -1;
        node_t->pr[i + MIN_KEYS] = -1;          
    }

    node_t->p[MIN_KEYS] = childs[MIN_KEYS];
    node_t->qtt_keys = MIN_KEYS;

    new_node_t->p[MIN_KEYS] = childs[i+1+MIN_KEYS];
    new_node_t->qtt_keys = MAX_KEYS - MIN_KEYS;

    *promo_key = keys[MIN_KEYS];
}

void insert_in_node(INDEX* key, int r_child, NODE_T* node, int type_file){
    int i = node->qtt_keys;
    
    //movendo as chaves para manter ordenados
    while(key->id < node->c[i-1] && i > 0){
        node->c[i] = node->c[i - 1];
        node->p[i+1] = node->p[i];
        i--;
    }

    node->qtt_keys++;

    //Adicionando a nova chave
    node->c[i] = key->id;
    if(type_file == 1) node->pr[i] = key->rrn;
    else               node->pr[i] = key->BOS;

    node->p[i + 1] = r_child;
}

int get_next_rrn(FILE* index_file){
    int next = -1;
    long int BOS = ftell(index_file);

    if(index_file != NULL){
        fseek(index_file, 5, SEEK_SET);
        fread(&next, sizeof(int), 1, index_file);    
        
        fseek(index_file, BOS, SEEK_SET);
    }

    return next;
}

void jump_to_root(FILE* index_file, B_TREE* b, int type_file){
    if(b->root_node != -1){
        if(type_file == 1) fseek(index_file, 45 + b->root_node*45, SEEK_SET);
        else               fseek(index_file, 57 + b->root_node*57, SEEK_SET);
    }else{
        NODE_T node = initialize_node(ROOT_NODE);
        //insert();
    }
}

void jump_to_node_b(FILE* index_file, int rrn, int type_file){
    if(type_file == 1)
        fseek(index_file, rrn*STATIC_INDEX_B + STATIC_INDEX_B, SEEK_SET);
    else
        fseek(index_file, rrn*VARIABLE_INDEX_B + VARIABLE_INDEX_B, SEEK_SET);
}

void write_node(FILE* index_file, int rrn, int type_file, NODE_T node){
    jump_to_node_b(index_file, rrn, type_file);

    fwrite(&node.type, 1, sizeof(char), index_file);
    fwrite(&node.qtt_keys, 1, sizeof(int), index_file);

    //key + reference
    for (int i = 0; i < TREE_ORDER - 1; i++){
        fwrite(&node.c[i], 1, sizeof(int), index_file);
        if(type_file == 1) fwrite(&node.pr[i], 1, sizeof(int), index_file);
        else               fwrite(&node.pr[i], 1, sizeof(long int), index_file);
    }

    //childs
    for (int i = 0; i < TREE_ORDER; i++)
        fwrite(&node.p[i], 1, sizeof(int), index_file);
}

void create_root(FILE* index_file, B_TREE* b, INDEX* key, int left, int right, int type_file){
    NODE_T node = initialize_node(ROOT_NODE);

    //Alterando a antiga raiz caso haja a insercao de uma nova
    if(b->root_node != -1){
        jump_to_root(index_file, b, type_file);
        fputc(INTER_NODE, index_file);
    }

    node.c[0]  = key->id;
    if(type_file == 2) node.pr[0] = key->BOS;
    else               node.pr[0] = key->rrn;
    node.p[0] = left;
    node.p[1] = right;

    node.qtt_keys = 1;

    write_node(index_file, b->nextRRN, type_file, node);
    b->root_node = b->nextRRN++;
}

