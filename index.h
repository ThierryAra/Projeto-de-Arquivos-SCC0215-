#include<stdio.h>

typedef struct index INDEX;

int create_index_id(FILE* bin_file, FILE* index_file, int type_file);

int print_index_file(FILE* index_file, int type_file);

int print_index_table(INDEX* id_indexes, int id_indexes_size, int type_file);

int free_index_array(INDEX* id_indexes);

int recover_rrn(INDEX* id_indexes, int id, int id_indexes_size, 
                    int mode, int type_file, int* rrn, long int* BOS);

INDEX* read_index_file(FILE* bin_file, FILE* index_file,  int* id_indexes_size, int type_file);