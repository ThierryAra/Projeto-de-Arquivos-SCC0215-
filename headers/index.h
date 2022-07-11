#ifndef INDEX_
#define INDEX_

#include<stdio.h>

typedef struct index INDEX;
struct index{
    int id;
    int rrn;
    long int BOS;
};

INDEX* initialize_index();
void free_index(INDEX* index);
/*  Read a data file and create an index file by matching ID
    RETURN:
            -2 if the parameters are invalid
            -1 if there is not enough space to allocate a vector of indices
             1 sucefull                                                     */
int create_index_id(FILE* bin_file, FILE* index_file, int type_file);

/*  Write the entire contents of index to index_file */
void write_index(FILE* index_file, INDEX* index, int index_size, int type_file);

/*  Displays all data from an index file */
int print_index_file(FILE* index_file, int type_file);

/*  Displays all data from an index vector  */
int print_index_table(INDEX* index, int index_size, int type_file);

/*  Frees the space allocated for an array of indices */
void free_index_array(INDEX* index);

/*  Updates an index vector, updating or deleting a given record
    If the 'mid' position is excluded, there is a swap between 'mid' and 'end' 
    @PARAM:
            mid  = position where the searched record is located 
            end  = last non-null position of the indices vector
            mode = 1 -> Exclusion 2 -> Addition 3 ->  Update              */
void update_id_index(INDEX* index, int mid, int type_file, int mode, 
                    int end, int rrn, long int BOS, int new_id);

/*  Through a binary search in the index vector, search for 'id' in the vector and
    returns the RRN/BOS (depending on type_file) of the 'id' in the registry       
    RETURN: 
            -1 if 'id' is not found
            position of the 'id' in the index vector                              */
int recover_rrn(INDEX* index, int id, int index_size, int type_file, int* rrn, long int* BOS);

/*  Read an index file and transfer it to an array of size 'index_size'
    RETURN:
            NULL if the file cannot be opened or there is no space for allocation   
            INDEX* pointer to allocated vector                                   */
INDEX* read_index_file(FILE* index_file, int* index_size, int type_file);

/*  Sort the index vector */
void sort_id_index(INDEX* array, int array_size);

/*  Insert a record with ID and rrn/BOS into the index vector */
void insert_index(INDEX* index, int index_size, int id, int rrn, long int BOS);

#endif