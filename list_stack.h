#include<stdio.h>

/*  Atuliza o campo numRecRem do cabecalho do arquivo, tanto para
    remocoes como para adicoes de registros
        mode:
             1 -> Remocao
             2 -> Adicao                                       */
void att_numRecRem(FILE* bin_file, int mode, int type_file, int quantity);

//-------------------------------STACK
typedef struct stack STACK;

STACK* create_stack(int stack_size);

void free_stack(STACK* stack);

void read_stack_top(FILE* bin_file, STACK* stack);

void add_stack(STACK* stack, int rrn);

int write_stack(FILE* bin_file, STACK* stack);

void print_stack(STACK* stack);
//-------------------------------LIST
typedef struct list LIST;
typedef struct node NODE;

LIST* create_list(int list_size);

void free_list(LIST* list);

void add_list(LIST* list, long int BOS, int rec_size, long int next_BOS);

void read_list(FILE* bin_file, LIST* list);

void add_sorted_to_list(LIST* list, long int BOS, int rec_size);

void print_list(LIST* list);

int write_list(FILE* bin_file, LIST* list);