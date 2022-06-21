#include<stdio.h>

typedef struct index INDEX;

void write_index(FILE* index_file, INDEX* index, int index_size, int type_file);

int create_index_id(FILE* bin_file, FILE* index_file, int type_file);

int print_index_file(FILE* index_file, int type_file);

int print_index_table(INDEX* index, int index_size, int type_file);

int free_index_array(INDEX* index);

int update_id_index(INDEX* index, int mid, int type_file, int mode, int end);

int recover_rrn(INDEX* index, int id, int index_size, int mode, 
                int type_file, int* rrn, long int* BOS);

INDEX* read_index_file(FILE* index_file, int* index_size, int type_file);

void sort_id_indexes(INDEX* array, int array_size);