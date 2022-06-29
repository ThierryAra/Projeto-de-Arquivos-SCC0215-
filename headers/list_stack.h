#include<stdio.h>

/*  Updates the 'numRecRem' field of the file header, both for
    removals and additions of records
        mode:
             1 -> Removal
             2 -> Addition                                       */
void att_numRecRem(FILE* bin_file, int mode, int type_file, int quantity);

//-------------------------------STACK
typedef struct stack STACK;
typedef struct node_stack NODE_stack;

/*  Allocates space for a stack, which will store the deleted files for
    type 1 log files                                       */
STACK* create_stack(int stack_size);

/*  Frees space allocated to a stack */
void free_stack(STACK* stack);

/*  Read entire stack structure present in bin_file */
void read_stack(FILE* bin_file, STACK* stack);

/*  Fetch top of stack in bin_file */
void read_stack_top(FILE* bin_file, STACK* stack);

/*  Adds an element identified by rrn in the stack */
void add_stack(STACK* stack, int rrn);

/*  Transfer the stack to the 'bin_file' file */
int write_stack(FILE* bin_file, STACK* stack);

/*  Remove the top of the stack */
int remove_from_stack(STACK* stack);

int return_stack_top(STACK* stack);

void print_stack(STACK* stack);

//-------------------------------LIST
typedef struct list LIST;
typedef struct node_list NODE_list;

/*  Allocates space for a linked list, which will store the excluded files 
    files for type 2 log files                 */
LIST* create_list(int list_size);

/*  Frees up space allocated for a linked list */
void free_list(LIST* list);

/*  Fetches the structure of the already existing list in bin_file */
void read_list(FILE* bin_file, LIST* list);

/*  Adds an element of size 'rec_size' and ByteOffset 'BOS' to the list */
void add_sorted_to_list(LIST* list, long int BOS, int rec_size);

/*  Transfers the linked list to bin_file */
int write_list(FILE* bin_file, LIST* list);

int remove_from_list(LIST* list);

void print_list(LIST* list);

long int return_list_top(LIST* list, int* size);
