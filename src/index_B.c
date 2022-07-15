#include<stdio.h>
#include<stdlib.h>
#include"../headers/index_B.h"

#define ROOT_NODE '0'
#define INTER_NODE '1' 
#define LEAF_NODE '2'

#define MAX_KEYS (TREE_ORDER-1)
#define MIN_KEYS (TREE_ORDER/2)

#define STATIC_INDEX_B 45
#define VARIABLE_INDEX_B 57

struct b_tree{
    char status;
    int order;
    int root_node;
    int nextRRN;
    int qtt_nodes;
};

struct node_t{
    char type;
    int qtt_keys;
    int c[TREE_ORDER - 1];
    long int pr[TREE_ORDER - 1];
    int p[TREE_ORDER];
};                                                                 

char select_type_node(B_TREE* b, NODE_T* node);
void write_header_b_tree(B_TREE* b, FILE* index_file, int type_file);

B_TREE* initialize_b_tree(int order){
    B_TREE* b = malloc(sizeof(B_TREE));

    b->status = '0';
    b->nextRRN = 0;
    b->qtt_nodes = 0;
    b->root_node = -1;
    b->order = TREE_ORDER;

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

    return node_t;
}

int create_b_tree_index(FILE* bin_file, FILE* index_file, int type_file){
    if(index_file == NULL || bin_file == NULL)
        return -2;

    B_TREE* b = initialize_b_tree(TREE_ORDER);
    write_header_b_tree(b, index_file, type_file);

    RECORD* r = create_record();
    HEADER* h = create_header();
    
    ignore_header(bin_file, type_file);
    
    int promo_child = -1;
    INDEX* promo_key = initialize_index();
    int res = 0;
    int rrn = 0;
    long int BOS = ftell(bin_file);
    int pos = -1;

    // ler arquivo de dados e obter as chaves
    while((res = get_record(bin_file, r, h, type_file)) != -2){
        if(res != -1){

            INDEX index;
            index.id = get_id(r);
            if(type_file == 1) 
                index.rrn = rrn;
            else{
                index.BOS = BOS;
                BOS = ftell(bin_file);  
            }
            
            pos = insert_b(index_file, b, &index, b->root_node, &promo_child, promo_key, type_file); 
            
            if(pos == PROMOTED)
                create_root(index_file, b, promo_key, b->root_node, promo_child, type_file);
        }

        rrn++;
    }

    update_header_b(index_file, b);

    free(b);
    free_rec(r);
    free_header(h);
    free_index(promo_key);
    return 1;
}

/**
 * @brief Busca recursivamente determinado ID no arquivo de indice no formato de arvore B
 * 
 * @param index_file    Arquivo de indice
 * @param type_file     Tipo do registro contido no arquivo de dados
 * @param id            ID que sera buscado
 * @param b             Struct da arvore B
 * @return int          -1 caso nao encontre / -2 caso encontre
 */
int search_b(FILE* index_file, int type_file, int id, int rrn){
    if(rrn == -1)
        return -1;

    jump_to_node_b(index_file, rrn, type_file);
    
    NODE_T node_t = read_node(index_file, type_file);
    
    int found = 0;
    long int reference = find_key_in_node(&node_t, id, type_file, &found);
    if(found == 1)
        return reference;

    return search_b(index_file, type_file, id, reference);
}

int insert_b(
    FILE* index_file, 
    B_TREE* b, 
    INDEX* key, 
    int curr_rrn, 
    int* promo_child, 
    INDEX* promo_key, 
    int type_file
){
    //armazena o rrn do proximo no na recursao de busca
    int next_node = -1;
    NODE_T node_t;


    //fim da arvore alcancada e nao foi encontrada, entao deve ser inserida
    if(curr_rrn == -1){
        *promo_key = *key;
        *promo_child = -1;

        return PROMOTED;
    }
    
    //chave promovida
    INDEX p_b_key;
    p_b_key.id  = -1;
    p_b_key.BOS = -1;
    p_b_key.rrn = -1;
    int p_b_rrn = 0;
    
    //leitura do nó e procura da chave
    jump_to_node_b(index_file, curr_rrn, type_file);
    node_t = read_node(index_file, type_file);
    
    int found = 0;
    next_node = find_key_in_node(&node_t, key->id, type_file, &found);
    if(found == 1)
        return -1;
    
    p_b_rrn = -1;
    int return_value = insert_b(index_file, b, key, next_node, 
                                &p_b_rrn, &p_b_key, type_file);
    //printf("PBKEY -> %d\n", p_b_key.id);

    if(return_value == PROMOTED){
        // Caso a recursão encontre a chave na arvore e haja espaco para insercao
        if(node_t.qtt_keys < MAX_KEYS){
            insert_in_node(&p_b_key, p_b_rrn, &node_t, type_file);
            write_node(index_file, curr_rrn, type_file, node_t);
            return_value = NoPROMOTED;
        // Nó cheio, deve haver split
        }else{
            char type = select_type_node(b, &node_t);
            NODE_T new_node_t = initialize_node(type);
            
            *promo_child = b->nextRRN++;
            split(index_file, &p_b_key, promo_key, p_b_rrn, 
                    promo_child, &node_t, &new_node_t, type_file);
            
            write_node(index_file, curr_rrn, type_file, node_t);
            write_node(index_file, *promo_child, type_file, new_node_t);
            
            b->qtt_nodes++;
            return_value = PROMOTED;
        }
    }

    //free_index(p_b_key);
    return return_value;
}

void split(
    FILE* index_file,
    INDEX* key,         /*Chave que sera adicionada    */
    INDEX* promo_key,   /*Chave que sera promovida     */
    int p_b_rrn,        /*RRN do novo nó               */
    int* promo_child,   /*rrn de referencia da direita */
    NODE_T* node_t,     /*no que sera splitado         */
    NODE_T* new_node_t, /*no que sera criado           */
    int type_file       /*tipo de arquivo              */
){
    int i;
    int childs[MAX_KEYS+2];
    INDEX keys[MAX_KEYS+1];

    //Adicionando todas as chaves em um vetor para fazer a distribuicao homoegena
    for (i = 0; i < MAX_KEYS; i++){
        keys[i].id = node_t->c[i];
        if(type_file == 1) keys[i].rrn = node_t->pr[i];
        else               keys[i].BOS = node_t->pr[i];

        childs[i] = node_t->p[i];
    }
    childs[i] = node_t->p[i];
    
    i = MAX_KEYS;
    //movendo as chaves para manter ordenados apos inserir a nova chave
    while(key->id < keys[i-1].id && i > 0){
        keys[i] = keys[i - 1];
        
        childs[i + 1] = childs[i];
        i--;
    }

    //Adicionando a nova chave no vetor ordenado
    keys[i] = *key;
    childs[i+1] = p_b_rrn;

    //Realizando a distribuicao entre os dois nos
    for (i = 0; i < MIN_KEYS; i++){
        node_t->c[i] = keys[i].id;
        node_t->p[i] = childs[i];

        if(type_file == 1)  node_t->pr[i] = keys[i].rrn;
        else                node_t->pr[i] = keys[i].BOS;

        //IF's para evitar erros quando se tem numero de chaves impar
        if(i + MIN_KEYS < MAX_KEYS){
            node_t->c[i + MIN_KEYS]  = -1;
            node_t->pr[i + MIN_KEYS] = -1;
           
            new_node_t->c[i] = keys[i+1+MIN_KEYS].id;
        
            if(type_file == 1) new_node_t->pr[i] = keys[i+1+MIN_KEYS].rrn;
            else               new_node_t->pr[i] = keys[i+1+MIN_KEYS].BOS;
        }

        if(i+1+MIN_KEYS <= MAX_KEYS){
            new_node_t->p[i] = childs[i+1+MIN_KEYS];
            node_t->p[i + 1 + MIN_KEYS] = -1;
        }
    }

    node_t->p[MIN_KEYS] = childs[MIN_KEYS];
    node_t->qtt_keys = MIN_KEYS;

    new_node_t->qtt_keys = MAX_KEYS - MIN_KEYS;

    if(MAX_KEYS%2 != 0)  new_node_t->p[MIN_KEYS-1] = childs[i+MIN_KEYS];
    else                 new_node_t->p[MIN_KEYS]   = childs[1+i+MIN_KEYS];
    *promo_key = keys[MIN_KEYS];
}

NODE_T read_node(FILE* index_file, int type_file){
    NODE_T node_t;
    
    fread(&node_t.type, sizeof(char), 1, index_file);
    fread(&node_t.qtt_keys, sizeof(int), 1, index_file);
    
    //printf("TIPO: %c [%d]\n", node_t.type, node_t.type);    
    //printf("QTT KEYS: %d\n", node_t.qtt_keys);       
    for (int i = 0; i < TREE_ORDER - 1; i++){
        fread(&node_t.c[i], sizeof(int), 1, index_file); //key[i]
        //reference to key[i] in data_file
        if(type_file == 1) fread(&node_t.pr[i], sizeof(int), 1, index_file);  
        else               fread(&node_t.pr[i], sizeof(long int), 1, index_file); 
        //printf("KEY i: %d -> %d : ", i, node_t.c[i]);    
        //printf("%ld\n", node_t.pr[i]);    
    }
    for (int i = 0; i < TREE_ORDER; i++){
        fread(&node_t.p[i], sizeof(int), 1, index_file); //reference sub-tree
        //printf("p: %d\n", node_t.p[i]);
    }

    return node_t;
}

int find_key_in_node(NODE_T* node_t, int key, int type_file, int* found){
    int next_node = node_t->p[0];

    int i = 0;
    while(i < node_t->qtt_keys && key > node_t->c[i]){
        i++;
        next_node = node_t->p[i];
    }

    if(i < MAX_KEYS && key == node_t->c[i]){
        *found = 1;
        return node_t->pr[i];
    }

    return next_node;
} 

void insert_in_node(INDEX* key, int r_child, NODE_T* node, int type_file){
    int i = node->qtt_keys;
    
    //movendo as chaves para manter ordenados
    while(key->id < node->c[i-1] && i > 0){
        node->c[i]  = node->c[i - 1];
        node->pr[i] = node->pr[i - 1]; 
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

void jump_to_root(FILE* index_file, B_TREE* b, int type_file){
    if(b->root_node != -1){
        if(type_file == 1) fseek(index_file, 45 + b->root_node*45, SEEK_SET);
        else               fseek(index_file, 57 + b->root_node*57, SEEK_SET);
    }
}

void jump_to_node_b(FILE* index_file, int rrn, int type_file){
    if(type_file == 1)
        fseek(index_file, (rrn*STATIC_INDEX_B) + STATIC_INDEX_B, SEEK_SET);
    else if(type_file == 2)
        fseek(index_file, (rrn*VARIABLE_INDEX_B) + VARIABLE_INDEX_B, SEEK_SET);
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
    for (int i = 0; i < TREE_ORDER; i++){
        fwrite(&node.p[i], 1, sizeof(int), index_file);
    }
}

void create_root(FILE* index_file, B_TREE* b, INDEX* key, int left, int right, int type_file){
    NODE_T node = initialize_node(ROOT_NODE);
    
    //Alterando a antiga raiz caso haja a insercao de uma nova
    if(b->root_node != -1){
        jump_to_root(index_file, b, type_file);
        char type = select_type_node(b, &node); 
        fputc(type, index_file);
    }

    node.c[0]  = key->id;
    if(type_file == 2) node.pr[0] = key->BOS;
    else               node.pr[0] = key->rrn;
    node.p[0] = left;
    node.p[1] = right;

    node.qtt_keys = 1;

    write_node(index_file, b->nextRRN, type_file, node);
    b->root_node = b->nextRRN++;
    b->qtt_nodes++;
}

void print_node(NODE_T* node, int type_file){
    printf("TYPE: %c\n", node->type);
    printf("QTT: %d\n", node->qtt_keys);
    
    for(int i = 0; i < TREE_ORDER - 1; i++){
        printf("KEY: %d ", node->c[i]);
        if(type_file == 1) printf(" [%d]\n", (int)node->pr[i]);
        else               printf(" [%ld]\n", node->pr[i]);
    }
    
    printf("P  : ");
    for(int i = 0; i < TREE_ORDER; i++)
        printf("[%i]: %d -> ", i, node->p[i]);
    printf("\n\n\n");
}

char select_type_node(B_TREE* b, NODE_T* node){
    if(node->type == ROOT_NODE){
        if(b->qtt_nodes < 4)
            return LEAF_NODE;
        else
            return INTER_NODE;
    }
    
    return node->type;
}

void update_header_b(FILE* index_file, B_TREE* b){
    b->status = '1';

    fseek(index_file, 0, SEEK_SET);
    fwrite(&b->status, 1, sizeof(char), index_file);
    fwrite(&b->root_node, 1, sizeof(int), index_file);
    fwrite(&b->nextRRN, 1, sizeof(int), index_file);
    fwrite(&b->qtt_nodes, 1, sizeof(int), index_file);
}

void show_nodes(FILE* index_file, int type_file){
    int stack[100];
    for (int i = 0; i < 100; i++)
        stack[i] = -1;
 
    int root = -1;
    int qtt_nodes = -1;

    fseek(index_file, 1, SEEK_SET);
    fread(&root, 1, sizeof(int), index_file);
    fread(&qtt_nodes, 1, sizeof(int), index_file);
    jump_to_node_b(index_file, root, type_file);

    int i = 0;
    int index = 1, p = 0;

    NODE_T node;
    do{
        node = read_node(index_file, type_file);
        int j = 0;
        
        while(node.p[j] != -1)
            stack[p++] = node.p[j++];
    
        jump_to_node_b(index_file, stack[index++], type_file);
    }while(++i < qtt_nodes);
    
    for (int i = 0; i < qtt_nodes; i++){
        jump_to_node_b(index_file, stack[i], type_file);
        node = read_node(index_file, type_file);
        printf("RRN %d: \n", i);
        print_node(&node, type_file);
    }
}

void write_header_b_tree(B_TREE* b, FILE* index_file, int type_file){
    //Escrevendo o cabecalho
    fwrite(&b->status, 1, sizeof(char),index_file);
    fwrite(&b->root_node, 1, sizeof(int),index_file);
    fwrite(&b->nextRRN, 1, sizeof(int),index_file);
    fwrite(&b->qtt_nodes, 1, sizeof(int),index_file);

    // preenchendo o cabecalho com lixo para obter o mesmo tamanho dos nos
    int qtt = 0;
    if(type_file == 1) qtt = STATIC_INDEX_B - 13;
    else               qtt = VARIABLE_INDEX_B - 13;

    for (int i = 0; i < qtt; i++)
        fwrite("$", 1, sizeof(char), index_file); 
}

B_TREE* read_header_b_tree(FILE* index_file, int type_file){
    B_TREE* b = malloc(sizeof(B_TREE));

    //Escrevendo o cabecalho
    fread(&b->status, 1, sizeof(char),index_file);
    fread(&b->root_node, 1, sizeof(int),index_file);
    fread(&b->nextRRN, 1, sizeof(int),index_file);
    fread(&b->qtt_nodes, 1, sizeof(int),index_file);

    return b;
}

int get_root_node(B_TREE* b_tree){
    return b_tree->root_node;
}