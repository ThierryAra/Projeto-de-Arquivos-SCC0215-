//#ifdef HEADER_

#include<stdio.h>

typedef struct header HEADER;

/*  Aloca espaco para um header e o retorna */
HEADER* create_header();

/*  Libera o espaco armazenado para header
    Retorna:
             1 caso execute corretamente
            -2 caso algum parametro seja nulo */
int free_header(HEADER* h);

/*  Escreve o header no arquivo binario de forma a depender do
    parametro type_file (1 -> arquivo1 ou 2 -> arquivo2)
    Retorna:
             1 caso execute corretamente
            -2 caso algum parametro seja nulo                */
int write_header(HEADER* h, FILE* bin_file, int type_file);

/*  Caso status do header seja 1 atualiza o arquivo para status 0
    e vice e versa
    Retorna:
             1 caso execute corretamente
            -2 caso algum parametro seja nulo                   */
int atualiza_status(HEADER* h, FILE* bin_file);

/*  verifica qual o seu estado do arquivo
    Retorna:
             1 = arquivo consistente
             0 = arquivo inconsistente    
            -1 = pointeiro esta na posicao incorreta */
int verifica_status(FILE* bin_file);