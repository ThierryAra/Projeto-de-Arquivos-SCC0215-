#include<stdio.h>

/*  Atuliza o campo numRecRem do cabecalho do arquivo, tanto para
    remocoes como para adicoes de registros
        mode:
             1 -> Remocao
             2 -> Adicao                                       */
void att_numRecRem(FILE* bin_file, int mode, int type_file, int quantity);

//-------------------------------STACK
void add_stack(FILE* bin_file, int rrn);

//-------------------------------LIST
void add_list(FILE* bin_file, long int BOS, int record_size);