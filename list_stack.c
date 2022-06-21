#include<stdio.h>
#include<stdlib.h>
#include"record.h"
#include"index.h"
#include"header.h"
#include"list_stack.h"

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
struct stack{
    int rec_amount;
    int* r_stack;
};

STACK* create_stack(int stack_size){
    STACK* stack = malloc(sizeof(STACK));

    stack->rec_amount = 0;
    stack->r_stack    = malloc(stack_size*sizeof(int));

    return stack;
}

void free_stack(STACK* stack){
    if(stack != NULL){
        if(stack->r_stack != NULL)
            free(stack->r_stack);
        
        free(stack);
    }
}

void add_stack(STACK* stack, int rrn){
    if(stack == NULL || rrn == -1)
        return;

    printf("ADICIONADO NA STACK %d [%d]\n", rrn, stack->rec_amount);
    stack->r_stack[stack->rec_amount++] = rrn;
}

int write_stack(FILE* bin_file, STACK* stack){
    if(stack->rec_amount == 0)
        return -1;

    int rrn = stack->r_stack[stack->rec_amount - 1];
    //atualiza o topo da stack
    fseek(bin_file, 1, SEEK_SET);
    fwrite(&rrn, 1, sizeof(int), bin_file);

    int old_rrn = 0;
    //Adiciona a pilha a estrutura dos registros excluidos no arquivo
    while(stack->rec_amount > 0){
        
        if(stack->rec_amount == 1)
            break;

        rrn     = stack->r_stack[stack->rec_amount - 1];
        old_rrn = stack->r_stack[stack->rec_amount - 2];

        jump_to_record(bin_file, rrn, 0);
        fseek(bin_file, 1, SEEK_CUR);
        fwrite(&old_rrn, 1, sizeof(int), bin_file);

        stack->rec_amount--;
    }
}

void print_stack(STACK* stack){
    int i = 0;

    for (i = 0; i < stack->rec_amount; i++)
    {
        printf("%d | ", stack->r_stack[i]);
    }
}

//-------------------------------LIST
void add_list(FILE* bin_file, long int BOS, int record_size){
    if(BOS == -1)
        return;

    long int begin_list = 0;
    fseek(bin_file, 1, SEEK_SET);
    fread(&begin_list, 1, sizeof(long int), bin_file);

    long int old_BOS = -1;

    long int new_BOS = begin_list;
    int new_size = -1;
    fseek(bin_file, BOS+1, SEEK_SET);
    fread(&new_size, 1, sizeof(int), bin_file);

    //Adicao sem elementos na lista
    if(begin_list == -1){
        fseek(bin_file, 1, SEEK_SET);
        fwrite(&BOS, 1, sizeof(long int), bin_file);
    }else{
        //Busca a posicao que o novo registro removido deve ser adicionado na lista
        while(new_BOS != -1 && record_size < new_size){
            old_BOS = new_BOS;
            //busca o proximo registro na fila
            fseek(bin_file, BOS+1, SEEK_SET);
            fread(&new_size, 1, sizeof(int), bin_file);
            fread(&new_BOS, 1, sizeof(long int), bin_file);
        }

        //Adicionar no inicio da lista
        if(old_BOS == -1){
            fseek(bin_file, 1, SEEK_SET);
            fwrite(&BOS, 1, sizeof(long int), bin_file);
            fseek(bin_file, old_BOS+5, SEEK_SET);
            fwrite(&BOS, 1, sizeof(long int), bin_file);
        }
        //Adicionando no final
        else if(new_BOS == -1){
            fseek(bin_file, old_BOS+5, SEEK_SET);
            fwrite(&BOS, 1, sizeof(long int), bin_file);
        }
        //Adicionando no meio
        else{
            fseek(bin_file, old_BOS+5, SEEK_SET);
            fwrite(&BOS, 1, sizeof(long int), bin_file);
            fseek(bin_file, BOS+5, SEEK_SET);
            fwrite(&new_BOS, 1, sizeof(long int), bin_file);
        }
    }
}