#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_t2.h"
#include "write_read_file.h"

#define BINf_HEADER_SIZE 190

// Registro criado para um arquivo contendo registros
// de tamanho variavel
struct record_t2{
    char removido;
    int tam_registro; 
    long int prox;
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

    r2->removido   = '0';
    r2->tam_registro = -1;
    r2->marca      = malloc(sizeof(char)*30);
    r2->cidade     = malloc(sizeof(char)*30);
    r2->modelo     = malloc(sizeof(char)*30);
    r2->codC5      = '0';
    r2->codC6      = '1';
    r2->codC7      = '2';
    r2->prox       = -1;
    return r2;
}

void free_rec_t2(Record_t2* r2){
    free(r2->cidade);
    free(r2->marca);
    free(r2->modelo);
    free(r2);
}

int write_header_t2(FILE* bin_file){
    if(bin_file == NULL)    
        return -2;

    //valores para serem salvos como padrao
    long int li  = 0; int i = 0; 
    long int nli = -1;
    fwrite("0", 1, sizeof(char), bin_file);                        //status
    fwrite(&nli, 1, sizeof(long int), bin_file);                   //topo
    fwrite("LISTAGEM DA FROTA DOS VEICULOS NO BRASIL",
            40, sizeof(char), bin_file);                           //descricao
    fwrite("CODIGO IDENTIFICADOR: ", 22, sizeof(char), bin_file);  //desC1
    fwrite("ANO DE FABRICACAO: ", 19, sizeof(char), bin_file);     //desC2
    fwrite("QUANTIDADE DE VEICULOS: ", 24, sizeof(char), bin_file);//desC3
    fwrite("ESTADO: ", 8, sizeof(char), bin_file);                 //desC4
    fwrite("0", 1, sizeof(char), bin_file);                        //codC5
    fwrite("NOME DA CIDADE: ", 17, sizeof(char), bin_file);        //desC5
    fwrite("1", 1, sizeof(char), bin_file);                        //codC6
    fwrite("MARCA DO VEICULO: ", 18, sizeof(char), bin_file);      //desC6
    fwrite("2", 1, sizeof(char), bin_file);                        //codC7
    fwrite("MODELO DO VEICULO: ", 19, sizeof(char), bin_file);     //desC7
    fwrite(&li, 1, sizeof(long int), bin_file);                    //proxRRN
    fwrite(&i, 1, sizeof(int), bin_file);                          //nroRegRem
    return 1;                          
}

int read_item_t2(FILE* file_csv, Record_t2* r2){
    if(file_csv == NULL || r2 == NULL)
        return -2;

    char c;
    //id (sempre exite um e eh != 0)
    if(read_int_field(file_csv, &(r2->id)) == -1)
        return -1;
    //ano
    if(read_int_field(file_csv, &r2->ano) == -1)
        r2->ano = -1;

    //cidade
    if(read_char_field(r2->cidade, file_csv) < 1){
        r2->cidade[0]  = '\0';
        r2->tam_cidade = 0;
    }else
        r2->tam_cidade = strlen(r2->cidade);
    
    //quantidade
    if(read_int_field(file_csv, &r2->qtt) == -1)
        r2->qtt = -1;
    //sigla
    if(read_char_field(r2->sigla, file_csv) < 1){
        r2->sigla[0] = '$';
        r2->sigla[1] = '$';
    }
    //marca
    if(read_char_field(r2->marca, file_csv) < 1){
        r2->marca[0]   = '\0';
        r2->tam_marca = 0;
    }else
        r2->tam_marca = strlen(r2->marca);
    //modelo
    if(read_char_field(r2->modelo, file_csv) < 1){
        r2->modelo[0]   = '\0';
        r2->tam_modelo = 0;
    }else
        r2->tam_modelo = strlen(r2->modelo);

    //remove o '\n'
    c = fgetc(file_csv);
    if(c != '\n')
        ungetc(c, file_csv);

    return 1;
}

int write_item_t2(FILE* bin_file, Record_t2* r2){
    if(bin_file == NULL || r2 == NULL)
        return -1;

    fseek(bin_file, 178, SEEK_SET);
    //BOS means ByteOffset
    long int BOS = 0;
    fread(&BOS, 1, sizeof(long int), bin_file);
    fseek(bin_file, BINf_HEADER_SIZE + BOS, SEEK_SET);

    //dados estaticos
    int record_size = 27;
    fwrite(&r2->removido, 1, sizeof(char), bin_file);
    fwrite(&r2->tam_registro, 1, sizeof(int), bin_file);
    fwrite(&r2->prox, 1, sizeof(long int), bin_file);
    fwrite(&r2->id, 1, sizeof(int), bin_file);
    fwrite(&r2->ano, 1, sizeof(int), bin_file);
    fwrite(&r2->qtt, 1, sizeof(int), bin_file);
    fwrite(r2->sigla, 2, sizeof(char), bin_file);

    //os if's verificam se o campo nao eh nulo
    if(r2->tam_cidade > 0){
        fwrite(&r2->tam_cidade, 1, sizeof(int), bin_file);
        fwrite(&r2->codC5, 1, sizeof(char), bin_file);
        fwrite(r2->cidade, r2->tam_cidade, sizeof(char), bin_file);
        record_size += 4 + 1 + r2->tam_cidade;
    }
    if(r2->tam_marca > 0){
        fwrite(&r2->tam_marca, 1, sizeof(int), bin_file);
        fwrite(&r2->codC6, 1, sizeof(char), bin_file);
        fwrite(r2->marca, r2->tam_marca, sizeof(char), bin_file);
        record_size += 4 + 1 + r2->tam_marca;
    }

    if(r2->tam_modelo > 0){
        fwrite(&r2->tam_modelo, 1, sizeof(int), bin_file);
        fwrite(&r2->codC7, 1, sizeof(char), bin_file);
        fwrite(r2->modelo, r2->tam_modelo, sizeof(char), bin_file);
        record_size += 4 + 1 + r2->tam_modelo;
    }
    
    fseek(bin_file, 178, SEEK_SET);
    //Adiciona o novo byte offset disponivel
    fread(&BOS, 1, sizeof(long int), bin_file);
    //buscando o valor e somando
    BOS = BOS + record_size;
    //retornando para escrever
    fseek(bin_file, -8, SEEK_CUR);
    fwrite(&BOS, 1, sizeof(long int), bin_file);

    return 1;
}

int create_table_t2(FILE* file_csv, FILE* bin_file){
    if(bin_file == NULL || file_csv == NULL)
        return -2;

    Record_t2* r2 = create_record_t2();
    if(write_header_t2(bin_file) == -2)
        return -1;
    int i = 0;
    while (read_item_t2(file_csv, r2) > 0){
        write_item_t2(bin_file, r2);
        if(i == 3)
            break;
        i++;
    } 

    //muda status para 1 (arquivo consistente de dados)
    fseek(bin_file, 0, SEEK_SET);
    fwrite("1", 1, sizeof(char), bin_file);
    free_rec_t2(r2);
    return 1;
}
