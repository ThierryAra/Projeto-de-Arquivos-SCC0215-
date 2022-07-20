#define STATIC_REC_SIZE 97       
#define STATIC_REC_HEADER 182 
#define VARIABLE_REC_HEADER 190 
#define STR_SIZE 30  

#define SIMPLE_INDEX 4
#define B_TREE_INDEX 5

#include"../headers/header.h"

typedef struct record RECORD;

/*  Allocate space for a record */
RECORD* create_record();

/*  Frees up stored space for a record */
void free_rec(RECORD* r);

/*  type_file = 1 -> fixed length record
    type_file = 2 -> variable length record */

/*  Fetch the data from the CSV file and write it to the binary file;
    type file determines which record will be fetched
    Returns:
         1 if run correctly
        -2 if any parameter is null                            */
int create_table(FILE* csv_file, FILE* bin_file, int type_file);

/*  Display all records in the file on the screen 
     Returns:
         1 if run correctly
        -2 if any parameter is null/corrupted file */
int select_from(FILE* bin_file, int type_file);

/*  From the fields and values in **fields, records that match 
    the data are searched and displayed on the screen 
    Returns:
         number of records found
        -2 if any parameter is null/corrupted file          */
int select_from_where(FILE* bin_file, char** fields, int n, int type_file);

/*  Fetch certain RRN in the file 
    Returns:
        -1 rrn does not exist
         1 if run correctly
        -2 if any parameter is null/corrupted file          */
int search_rrn(char* type_file, FILE* bin_file, int rrn, RECORD* r);

int print_record(RECORD* r);

//---------------------------PART 2---------------------------------//
/*  Skip exactly 1 record when reading the binary file
    RETURN: -1 if type_file is invalid              */
int next_register(FILE* bin_file, int type_file);

/*  Receives from the input k doubles (field, value), 'n' times, that will be 
    searched from the index file (if the search is by id) or sequentially in 
    bin_file, if a record matches the presented fields it will be virtually deleted.
    RETURN:
            -2 if the files are corrupted or do not exist
             1 success                                                            */
int delete_where(FILE* bin_file, char* index_file, int n, int type_file);

/*  From RRN/BOS, give a fseek directly to the position.
    If rrn and BOS are nonzero, only the fseek of rrn will be performed      */
void jump_to_record(FILE* file, int rrn, long int BOS);

/*  Insert 'n' records into 'bin_file' and update index file 'name_index'
    RETURN:
            -2 if the files are corrupted or do not exist
             1 success                                                            */
int insert_into(FILE* bin_file, char* name_index, int n, int type_file, int index_mode);

/*  Update 'n' records in 'bin_file' and update index file 'name_index'
    RETURN:
            -2 if the files are corrupted or do not exist
             1 success                                                            */
int update_where(FILE* bin_file, char* name_index, int n, int type_file);

int get_record(FILE* bin_file, RECORD* r, HEADER* header, int type_file);

//----------------------------PART 3------------------------------------------//

/*  Searches for certain ID's in the B-tree index file and, if found, displays it */
int search_with_b_tree(FILE* bin_file, FILE* index_file, int type_file);

int get_id(RECORD* r);