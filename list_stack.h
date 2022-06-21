#include<stdio.h>

typedef struct stack STACK;

/*  Atuliza o campo numRecRem do cabecalho do arquivo, tanto para
    remocoes como para adicoes de registros
        mode:
             1 -> Remocao
             2 -> Adicao                                       */
void att_numRecRem(FILE* bin_file, int mode, int type_file, int quantity);

//-------------------------------STACK

STACK* create_stack(int stack_size);

void free_stack(STACK* stack);

void add_stack(STACK* stack, int rrn);

int write_stack(FILE* bin_file, STACK* stack);

void print_stack(STACK* stack);
//-------------------------------LIST
void add_list(FILE* bin_file, long int BOS, int record_size);