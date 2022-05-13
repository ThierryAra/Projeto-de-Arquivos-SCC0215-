#include"write_read_file.h"

typedef struct record_t1 Record_t1;

/*  Aloca espaco para um registro e inicializa seus campos 
    e o retorna                                         */
Record_t1* create_record_t1();

/*  Libera todo o espaco armazenado para r1 */
void free_rec_t1(Record_t1* r1);

/*  Cria um arquivo binario a partir da leitura de um arquivo .csv
    Retorna 1 caso ocorra tudo normalmente
           -1 caso haja erro na leitura na escrita do cabecalho
           -2 caso haja os parametros estejam corrompidos        */
int create_table_t1(FILE* file_csv, FILE* file_bin);

/*  Le um registro do arquivo CSV e armazena em r1 
    Retorna 1 se o registro conseguiu ser lido corretamente
           -1 caso nao haja mais registros a serem lidos
           -2 caso algum dos parametros seja nulo        */
int read_item_t1(FILE* file_csv, Record_t1* r1);

/*  Escreve no arquivo .bin o header dos arquivos de tipo1
    Retorna 1 caso ocorra tudo corretamente
           -2 caso o parametro estejam corrompido       */
int write_header_t1(FILE* file_bin);

/*  Escreve um registro (r1) no arquivo .bin 
    Retorna 1 caso nao haja nenhum erro
           -1 caso os parametros estejam corrompidos */
int write_item_t1(FILE* file_bin, Record_t1* r1);

/*  Retorna todos os registros contidos no arquivo binario
    Retorna 1 caso ocorra tudo normalmente
           -2 caso o parametro esteja corrompido        */
int select_from_r1(FILE* file_bin);

/*  Retorna os registros que possuam correspondencia com os dados
    passados em fields (vetor que contem o campo e o valor do campo
    que quer ser encontrado - valor deve estar entre aspas)     
    Retorna (quantidade de registros encontrados) caso ocorra tudo normalmente
           -1 caso haja erro de leitura (ou fim de arquivo) 
           -2 caso os parametros esteja corrompidos                         */
int select_from_where_r1(FILE* file_bin, char** fields, int n);

/*  Exibe o registro r1 na tela 
    Retorna 1 caso nao haja erro
           -1 caso r1 seja nulo  */
int print_r1(Record_t1* r1);

/*  Busca registro que possua determinado RRN em file
    caso encontre, o transfere para r1
    Retorna 1 caso nao haja erro
           -1 caso o RRN seja invalido
           -2 caso os parametros sejam invalidos   */
int search_rrn(char* type_file, FILE* file, int rrn, Record_t1* r1);