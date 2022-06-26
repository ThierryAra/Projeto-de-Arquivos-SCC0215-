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
/*
    Pula exatamente 1 registo na leitura do arquivo binario
    RETURN: -1 caso type_file seja invalido              */
int next_register(FILE* bin_file, int type_file);

/*
    Recebe da entrada k duplas (campo, valor), 'n' vezes, que serao buscados
    a partir do arquivo de indices (caso a busca seja por id) ou sequencialmente
    em bin_file, caso um registro corresponda aos campos apresentados ele sera
    virtualmente excluido.
    RETURN:
            -2 se os arquivos estao corrompidos ou nao existem
             1 sucesso                                                         */
int delete_where(FILE* bin_file, char* index_file, int n, int type_file);

/*  A partir do RRN/BOS, da um fseek diretamente para a posicao.
    Se rrn e BOS sao nao nulos, apenas o fseek do rrn sera realizado           */
void jump_to_record(FILE* file, int rrn, long int BOS);

int insert_into(FILE* bin_file, char* name_index, int n, int type_file);