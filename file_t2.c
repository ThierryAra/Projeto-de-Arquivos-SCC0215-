#include<stdio.h>
#include<stdlib.h>
#include"file_t2.h"

// Registro criado para um arquivo contendo registros
// de tamanho variavel
struct record_t2{
    char removido;
    int tam_registro, prox;
    int id, ano, qtt;
    char sigla[2];
    int tam_cidade;
    char codC5;
    char* cidade;
    int tam_marca;
    char codC6;
    char* marca;
    int tam_modelo;
    char codC7;
    char* modelo;
};

Record_t2* create_record_t2(){
    Record_t2* r2 = malloc(sizeof(Record_t2));

    r2->removido   = 0;
    r2->marca      = malloc(sizeof(char)*30);
    r2->cidade     = malloc(sizeof(char)*30);
    r2->modelo     = malloc(sizeof(char)*30);
    r2->codC5      = '0';
    r2->codC6      = '1';
    r2->codC7      = '2';
    r2->prox       = 0;
    return r2;
}

void free_reg_t2(Record_t2* r2){
    free(r2->cidade);
    free(r2->marca);
    free(r2->modelo);
    free(r2);
}

void write_header_t2(FILE* file){
    //valores para serem salvos como padrao
    long int li  = 0; int i = 0; 
    long int nli = -1;
    fwrite("0", 1, sizeof(char), file);                        //status
    fwrite(&nli, 1, sizeof(long int), file);                   //topo
    fwrite(&i, 1, sizeof(int), file);                          //topo
    fwrite("LISTAGEM DA FROTA DOS VEICULOS NO BRASIL",
            40, sizeof(char), file);                           //descricao
    fwrite("CODIGO IDENTIFICADOR: ", 22, sizeof(char), file);  //desC1
    fwrite("ANO DE FABRICACAO: ", 19, sizeof(char), file);     //desC2
    fwrite("QUANTIDADE DE VEICULOS: ", 24, sizeof(char), file);//desC3
    fwrite("ESTADO: ", 8, sizeof(char), file);                 //desC4
    fwrite("0", 1, sizeof(char), file);                        //codC5
    fwrite("NOME DA CIDADE: ", 17, sizeof(char), file);        //desC5
    fwrite("1", 1, sizeof(char), file);                        //codC6
    fwrite("MARCA DO VEICULO: ", 18, sizeof(char), file);      //desC6
    fwrite("2", 1, sizeof(char), file);                        //codC7
    fwrite("MODELO DO VEICULO: ", 19, sizeof(char), file);     //desC7
    fwrite(&li, 1, sizeof(long int), file);                    //proxRRN
    fwrite(&i, 1, sizeof(int), file);                          //nroRegRem
}