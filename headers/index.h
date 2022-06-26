#include<stdio.h>

typedef struct index INDEX;

/*  Le um arquivo de dados e cria um arquivo de indices por ID compativel
    RETURN:
            -2 caso os parametros sejam invalidos
            -1 caso nao haja espaco suficiente para alocar um vetor de indices
             1 sucefull                                                     */
int create_index_id(FILE* bin_file, FILE* index_file, int type_file);

/*  Escreve todo o conteudo de index em index_file */
void write_index(FILE* index_file, INDEX* index, int index_size, int type_file);

/*  Exibe todos os dados de um arquivo de indice */
int print_index_file(FILE* index_file, int type_file);

/*  Exibe todos os dados de um vetor de indices  */
int print_index_table(INDEX* index, int index_size, int type_file);

/*  Libera o espaco alocado para um vetor de indices */
void free_index_array(INDEX* index);

/*  Atualiza um vetor de indice, atualizando ou excluindo determinado registro
    Caso haja a exclusao da posicao 'mid', ocorre um swap entre 'mid' e 'end' 
    @PARAM:
            mid  = posicao onde esta localizado o registro buscado 
            end  = ultima posicao nao nula do vetor de indices
            mode = 1 -> Exclusao de regsitro; 2 -> Adicao de registro        */
void update_id_index(INDEX* index, int mid, int type_file, int mode, int end, int rrn, long int BOS);

/*  Atraves de uma busca binaria no vetor de indice, busca por 'id' no vetor e
    retorna o RRN/BOS (a depender de type_file) do 'id' no registro         
    RETURN: 
            -1 caso 'id' nao seja encontrado
            posicao do 'id' no vetor de indice                              */
int recover_rrn(INDEX* index, int id, int index_size, int type_file, int* rrn, long int* BOS);

/*  Le um arquivo de indice e o transfere para um vetor de tamanho 'index_size' 
    RETURN:
            NULL caso o arquivo nao possa ser aberto ou nao haja espaco para alocacao   
            INDEX* ponteiro para o vetor alocado                                   */
INDEX* read_index_file(FILE* index_file, int* index_size, int type_file);

/*  Ordena o vetor de indice */
void sort_id_index(INDEX* array, int array_size);

void insert_index(INDEX* index, int index_size, int id, int rrn, long int BOS);