#include<stdio.h>

/*  Atuliza o campo numRecRem do cabecalho do arquivo, tanto para
    remocoes como para adicoes de registros
        mode:
             1 -> Remocao
             2 -> Adicao                                       */
void att_numRecRem(FILE* bin_file, int mode, int type_file, int quantity);

//-------------------------------STACK
typedef struct stack STACK;

/*  Aloca espaco para uma pilha, que armazenara os arquivos exluidos para
    arquivos de registros tipo 1                                       */
STACK* create_stack(int stack_size);

/*  Libera o espaco alocado para uma pilha */
void free_stack(STACK* stack);

/*  Busca o topo da pilha em bin_file */
void read_stack_top(FILE* bin_file, STACK* stack);

/*  Adiciona um elemento identificado pelo rrn na stach */
void add_stack(STACK* stack, int rrn);

/*  Transfere a stack para o arquivo bin_file */
int write_stack(FILE* bin_file, STACK* stack);

void print_stack(STACK* stack);

//-------------------------------LIST
typedef struct list LIST;
typedef struct node NODE;

/*  Aloca espaco para uma lista ligada, que armazenara os arquivos 
    exluidos para arquivos de registros tipo 2                  */
LIST* create_list(int list_size);

/*  Libera o espaco alocado para uma lista ligada */
void free_list(LIST* list);

/*  Busca a estrutura da lista ja existente em bin_file */
void read_list(FILE* bin_file, LIST* list);

/*  Adiciona um elemento de tamanho 'rec_size' e ByteOffset 'BOS' na lista */
void add_sorted_to_list(LIST* list, long int BOS, int rec_size);

/*  Transfere a lista ligada para o arquivo bin_file */
int write_list(FILE* bin_file, LIST* list);

void print_list(LIST* list);