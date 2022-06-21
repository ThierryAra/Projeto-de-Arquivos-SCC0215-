#include<stdio.h>
#include<stdlib.h>
#include"record.h"
#include"index.h"
#include"header.h"
#include"list_stack.h"

/*
    Atuliza o campo numRecRem do cabecalho do arquivo, tanto para
    remocoes como para adicoes de registros
        mode: 1 -> Remocao
            : 2 -> Adicao                                       */
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
void add_stack(FILE* bin_file, int rrn){
    if(rrn == -1)
        return;

    int top_stack = 0;
    fseek(bin_file, 1, SEEK_SET);
    fread(&top_stack, 1, sizeof(int), bin_file);
    //printf("top_stack %d\n", top_stack);

    //Escreve o novo topo da pilha
    fseek(bin_file, -4, SEEK_CUR);
    rrn = 0;
    fwrite(&rrn, 1, sizeof(int), bin_file);
    //printf("rrn  %d\n", rrn);

    /* int new_top_stack = 0;
    fseek(bin_file, -4, SEEK_CUR);
    fread(&new_top_stack, 1, sizeof(int), bin_file);
    printf("new_top_stack %d\n", new_top_stack); */
    
    //Verifica se ja existia algum elemento na fila
    if(top_stack != -1){
        //move o ponteiro ate o campo next do penultimo registro removido
        fseek(bin_file, (rrn*STATIC_REC_SIZE)+STATIC_REC_HEADER+1, SEEK_SET);
        fwrite(&top_stack, 1, sizeof(int), bin_file);
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