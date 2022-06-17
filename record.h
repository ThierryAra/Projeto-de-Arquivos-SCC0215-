#define STATIC_REC_SIZE 97       
#define STATIC_REC_HEADER 182 
#define VARIABLE_REC_HEADER 190 
#define STR_SIZE 30  

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

//-------------------------TRABALHO 2-------------------------------//
int next_register(FILE* bin_file, int type_file);

