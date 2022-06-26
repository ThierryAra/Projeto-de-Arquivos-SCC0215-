#include<stdio.h>
#include<stdlib.h>
#include"../headers/record.h"
#include"../headers/index.h"
#include"../headers/header.h"
#include"../headers/list_stack.h"

/*
    Atuliza o campo numRecRem do cabecalho do arquivo, tanto para
    remocoes como para adicoes de registros
        mode
             1 -> Remocao
             2 -> Adicao                                       */
void att_numRecRem(FILE* bin_file, int mode, int type_file, int quantity){
    int numRecRem = 0;
    if(type_file == 1){
        fseek(bin_file, 178, SEEK_SET);
        fread(&numRecRem, 1, sizeof(int), bin_file);
    }else{
        fseek(bin_file, 186, SEEK_SET);
        fread(&numRecRem, 1, sizeof(int), bin_file);
    }

    //Removendo um registro
    if(mode == 1)
        if(numRecRem <= 0)
            numRecRem = quantity;
        else
            numRecRem += quantity;
    else //Adicionando um registro
        numRecRem -= quantity;
        
    fseek(bin_file, -sizeof(int), SEEK_CUR);
    fwrite(&numRecRem, 1, sizeof(int), bin_file);
}

//-------------------------------STACK
struct node_stack{
    int rrn;
    int next;
};

struct stack{
    int rec_amount;
    int begin;
    NODE_stack* r_stack;
};

STACK* create_stack(int stack_size){
    STACK* stack = malloc(sizeof(STACK));

    stack->rec_amount = 0;
    stack->r_stack    = malloc(stack_size*sizeof(NODE_stack));
    stack->begin      = -1;
    return stack;
}

void free_stack(STACK* stack){
    if(stack != NULL){
        if(stack->r_stack != NULL)
            free(stack->r_stack);
        
        free(stack);
    }
}

void read_stack_top(FILE* bin_file, STACK* stack){
    fseek(bin_file, 1, SEEK_SET);
    int top_stack = -1;

    fread(&top_stack, 1, sizeof(int), bin_file);
    NODE_stack node_stack;

    node_stack.rrn  = top_stack;
    node_stack.next = -1;

    if(top_stack != -1){
        stack->r_stack[0] = node_stack;
        stack->begin = 0;
        stack->rec_amount++;
    }
}

void read_stack(FILE* bin_file, STACK* stack){
    int rrn = -1;
    int next_rrn = -1;

    //Lendo o topo da pilha
    fseek(bin_file, 1, SEEK_SET);
    fread(&rrn, 1, sizeof(int), bin_file);
    
    char c = 0;
    NODE_stack node;
    node.rrn = rrn;
    
    while(node.rrn != -1){
        jump_to_record(bin_file, node.rrn, 0);
        fseek(bin_file, 1, SEEK_CUR);
        fread(&next_rrn, 1, sizeof(int), bin_file);

        //adicionando o no na pilha
        node.next = next_rrn;
        stack->r_stack[stack->rec_amount++] = node;

        //buscando o proximo no
        node.rrn = next_rrn;
    }
}

void add_stack(STACK* stack, int rrn){
    if(stack == NULL || rrn == -1)
        return;

    //printf("ADICIONADO NA STACK %d [%d]\n", rrn, stack->rec_amount);
    NODE_stack node_stack;
    node_stack.rrn = rrn;
    stack->r_stack[stack->rec_amount].next = stack->begin;

    //Adicionando o novo rrn e atualizando o topo
    stack->r_stack[stack->rec_amount++] = node_stack;
    stack->begin = stack->rec_amount - 1;
}

int write_stack(FILE* bin_file, STACK* stack){
    if(stack->rec_amount == 0)
        return -1;

    NODE_stack node = stack->r_stack[stack->begin];
    //atualiza o topo da stack
    fseek(bin_file, 1, SEEK_SET);
    fwrite(&node.rrn, 1, sizeof(int), bin_file);

    NODE_stack old_node;
    old_node.next = 0;
    
    //Adiciona a pilha a estrutura dos registros excluidos no arquivo
    while(stack->rec_amount != 1){
        old_node = node;
        if(node.next != -1)
            node = stack->r_stack[node.next];

        jump_to_record(bin_file, old_node.rrn, 0);
        fwrite("1", 1, sizeof(char), bin_file);
        fwrite(&node.rrn, 1, sizeof(int), bin_file);

        stack->rec_amount--;
    }

    jump_to_record(bin_file, node.rrn, 0);
    fwrite("1", 1, sizeof(char), bin_file);
}

void print_stack(STACK* stack){
    int i = 0;

    for (i = 0; i < stack->rec_amount; i++)
        printf("%d | ", stack->r_stack[i].rrn);
}

int remove_from_stack(STACK* stack){
    if(stack->rec_amount == 0){
        return -1;
    }

    stack->begin = stack->r_stack[stack->begin].next;
    return 1;
}

int return_stack_top(STACK* stack){
    return stack->r_stack[stack->begin].rrn;
}


//-------------------------------LIST
struct node_list{
    long int BOS;
    int rec_size;
    int next;
};

struct list{
    int rec_amount;
    int begin;
    NODE_list* r_list;
};

LIST* create_list(int list_size){
    LIST* list = malloc(sizeof(LIST));

    list->rec_amount = 0;
    list->r_list     = malloc(list_size*sizeof(NODE_list));

    return list; 
}

void free_list(LIST* list){
    if(list != NULL){
        if(list->r_list != NULL)
            free(list->r_list);
        
        free(list);
    }
}

/*  Adiciona um elemento na lista, de forma sequencial */
void add_list(LIST* list, long int BOS, int rec_size, long int next_BOS){
    list->r_list[list->rec_amount].BOS = BOS;
    list->r_list[list->rec_amount].rec_size = rec_size;
    
    // Como eles ja estao no arquivo ordenadamente, caso exista um proxBOS,
    //ele estara na proxima posicao do vetor
    if(next_BOS != -1)
        list->r_list[list->rec_amount].next = list->rec_amount + 1;
    else
        list->r_list[list->rec_amount].next = -1;

    list->rec_amount++;
}

void read_list(FILE* bin_file, LIST* list){
    if(bin_file == NULL)
        return;

    fseek(bin_file, 1, SEEK_SET);

    long int BOS = 0;
    fread(&BOS, 1, sizeof(long int), bin_file);

    int rec_size = 0;
    long int next_BOS = 0;

    if(BOS != -1){
        // Enquanto houver registros removidos ja salvos no arquivo, o loop se mantem
        while(BOS != -1){
            //Busca os dados do registro
            fseek(bin_file, BOS+1, SEEK_SET);
            fread(&rec_size, 1, sizeof(int), bin_file);
            fread(&next_BOS, 1, sizeof(long int), bin_file);

            //adiciona na lista
            add_list(list, BOS, rec_size, next_BOS);

            BOS = next_BOS;
        }

        list->begin = 0;
    }
}

void add_sorted_to_list(LIST* list, long int BOS, int rec_size){
    NODE_list node_list;
    node_list.rec_size = rec_size;
    node_list.BOS      = BOS;
    node_list.next     = -1;

    if(list->rec_amount == 0){
        list->r_list[0] = node_list;
        list->rec_amount++;
        list->begin = 0;
        return;
    }

    // loop control
    int i = 0;

    int actual_node = list->begin;
    int last_node   = -1;
    
    // Loop para buscar a posicao ideal para adicionar o novo registro 
    //na lista ordenada a partir do rec_size
    while(actual_node != -1 && list->r_list[actual_node].rec_size > rec_size){
        last_node = actual_node;
        
        //proximo no da lista
        actual_node = list->r_list[actual_node].next;
        i++;
    }
    
    
    if(i == list->rec_amount){          //insercao no fim
        list->r_list[last_node].next = list->rec_amount;
    }else if(i == 0){                   //insercao no inicio
        node_list.next = list->begin;
    }else{                              //insercao no meio
        node_list.next = actual_node;
        list->r_list[last_node].next = list->rec_amount;
    }

    list->r_list[list->rec_amount] = node_list;
    list->rec_amount++;
}

void print_list(LIST* list){
    if(list == NULL)
        return;
    

    int actual = list->begin;

    while (actual != -1){
        printf("%d [%ld] | ", list->r_list[actual].rec_size, list->r_list[actual].BOS);
        actual = list->r_list[actual].next;
    }
}

int write_list(FILE* bin_file, LIST* list){
    if(bin_file == NULL && list == NULL)
        return -1;

    fseek(bin_file, 1, SEEK_SET);
    
    //se ha apenas um elemento, basta atualizar o topo da lista
    if(list->rec_amount == 1){
        fwrite(&list->r_list[0].BOS, 1, sizeof(long int), bin_file);
        return 1;
    }

    // loop control
    int i = 0;
    
    NODE_list node_list = list->r_list[list->begin];
    //Adiciona todos os registros removidos da lista no arquivo de dados
    while(i < list->rec_amount){
        fseek(bin_file, node_list.BOS, SEEK_SET);
        //marca como removido
        fwrite("1", 1, sizeof(char), bin_file);
        //adiciona o nextBOS
        fseek(bin_file, sizeof(int), SEEK_CUR);
        
        if(node_list.next != -1){
            fwrite(&list->r_list[node_list.next].BOS, 1, sizeof(long int), bin_file);
            node_list = list->r_list[node_list.next];
        }
        i++;
    }

    return 1;
}

int return_list_top(LIST* list, int* size){
    *size = list->r_list[list->begin].rec_size;
    return list->r_list[list->begin].BOS;
}