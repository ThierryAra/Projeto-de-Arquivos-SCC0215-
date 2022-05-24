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
    r2->tam_registro = 0;
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
    fwrite("NOME DA CIDADE: ", 16, sizeof(char), bin_file);        //desC5
    fwrite("1", 1, sizeof(char), bin_file);                        //codC6
    fwrite("MARCA DO VEICULO: ", 18, sizeof(char), bin_file);      //desC6
    fwrite("2", 1, sizeof(char), bin_file);                        //codC7
    fwrite("MODELO DO VEICULO: ", 19, sizeof(char), bin_file);     //desC7
    fwrite(&li, 1, sizeof(long int), bin_file);                   //proxByteOffset
    fwrite(&i, 1, sizeof(int), bin_file);                          //nroRegRem
    return 1;                          
}

int read_item_t2(FILE* file_csv, Record_t2* r2){
    if(file_csv == NULL || r2 == NULL)
        return -2;

    r2->tam_registro = 22;
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
    }else{
        r2->tam_cidade = strlen(r2->cidade);
        r2->tam_registro += r2->tam_cidade + 5;
    }

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
    }else{
        r2->tam_marca = strlen(r2->marca);
        r2->tam_registro += r2->tam_marca + 5;
    }
    //modelo
    if(read_char_field(r2->modelo, file_csv) < 1){
        r2->modelo[0]   = '\0';
        r2->tam_modelo = 0;
    }else{
        r2->tam_modelo = strlen(r2->modelo);
        r2->tam_registro += r2->tam_modelo + 5;
    }

    //remove o '\n'
    c = fgetc(file_csv);
    if(c != '\n')
        ungetc(c, file_csv);

    return 1;
}

int write_item_t2(FILE* bin_file, Record_t2* r2){
    if(bin_file == NULL || r2 == NULL)
        return -1;

    //BOS means ByteOffset
    long int BOS = 0;
    fseek(bin_file, 178, SEEK_SET);
    fread(&BOS, 1, sizeof(long int), bin_file);
    BOS += BINf_HEADER_SIZE;
    fseek(bin_file, BOS, SEEK_SET);

    //dados estaticos
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
    }
    if(r2->tam_marca > 0){
        fwrite(&r2->tam_marca, 1, sizeof(int), bin_file);
        fwrite(&r2->codC6, 1, sizeof(char), bin_file);
        fwrite(r2->marca, r2->tam_marca, sizeof(char), bin_file);
    }

    if(r2->tam_modelo > 0){
        fwrite(&r2->tam_modelo, 1, sizeof(int), bin_file);
        fwrite(&r2->codC7, 1, sizeof(char), bin_file);
        fwrite(r2->modelo, r2->tam_modelo, sizeof(char), bin_file);
    }
    
    //Adiciona o novo byte offset disponivel
    BOS += r2->tam_registro + 5;
    fseek(bin_file, 178, SEEK_SET);
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


int add_str_field_2(FILE* bin_file, Record_t2* r2){
    
    int string_size = 0;
    fread(&string_size, 1, sizeof(int), bin_file);
    
    char cod = '3';
    
    fread(&cod, 1, sizeof(char), bin_file);

    if(cod == '0'){
        //cidade (0)
        r2->tam_cidade = string_size;
        r2->codC5      = cod;
        fread(r2->cidade, r2->tam_cidade, sizeof(char), bin_file);
        r2->cidade[string_size] = '\0';
    }else if(cod == '1'){
        //marca (1)
        r2->tam_marca = string_size;
        r2->codC6     = cod;
        fread(r2->marca, r2->tam_marca, sizeof(char), bin_file);
        r2->marca[string_size] = '\0';
    }else if(cod == '2'){
        //modelo (2)
        r2->tam_modelo = string_size;
        r2->codC7      = cod;
        fread(r2->modelo, r2->tam_modelo, sizeof(char), bin_file);
        r2->modelo[string_size] = '\0';
    }else
        return -1;

    return string_size+4+1;
}

int get_record_t2(FILE* bin_file, Record_t2* r2, int removed){
    //campos de tamanho fixo
    if(fread(&r2->removido, 1, sizeof(char), bin_file) == 0)
        return -2; 

    fread(&r2->tam_registro, 1, sizeof(int), bin_file);
    
    //verifica se o registro nao foi removido logicamente
    if(r2->removido == '1'){
        fseek(bin_file, r2->tam_registro, SEEK_CUR);
        return -1;
    }

    //prox
    fread(&r2->prox, 1, sizeof(long int), bin_file);
    //id
    fread(&r2->id, 1, sizeof(int), bin_file);
    //ano
    fread(&r2->ano, 1, sizeof(int), bin_file);
    //qtt
    fread(&r2->qtt, 1, sizeof(int), bin_file);
    //sigla
    fread(&r2->sigla, 2, sizeof(char), bin_file);
    
    //campos de tamanho variavel
    int bytes_lidos = 22;
    r2->tam_cidade = 0;
    r2->tam_marca  = 0;
    r2->tam_modelo = 0;
    
    //verifica se existem mais campos
    if(bytes_lidos == r2->tam_registro){
        return r2->tam_registro;
    }
    
    bytes_lidos += add_str_field_2(bin_file, r2);

    //verifica se existem mais campos
    if(bytes_lidos == r2->tam_registro){
        return r2->tam_registro;
    }

    bytes_lidos += add_str_field_2(bin_file, r2);

    //verifica se existem mais campos
    if(bytes_lidos == r2->tam_registro){
        return r2->tam_registro;
    }

    bytes_lidos += add_str_field_2(bin_file, r2);

    return bytes_lidos;
}

int print_r2(Record_t2* r2){
    if(r2 == NULL)
        return -1;

    //os if's verificam se o campo nao eh nulo
    if(r2->tam_marca > 0)
        printf("MARCA DO VEICULO: %s\n", r2->marca);  
    else  
        printf("MARCA DO VEICULO: NAO PREENCHIDO\n");  
    if(r2->tam_modelo > 0)
        printf("MODELO DO VEICULO: %s\n", r2->modelo);
    else 
        printf("MODELO DO VEICULO: NAO PREENCHIDO\n");
    if(r2->ano != -1)
        printf("ANO DE FABRICACAO: %d\n", r2->ano); 
    else   
        printf("ANO DE FABRICACAO: NAO PREENCHIDO\n"); 
    if(r2->tam_cidade > 0)
        printf("NOME DA CIDADE: %s\n", r2->cidade);
    else
        printf("NOME DA CIDADE: NAO PREENCHIDO\n");
    if(r2->qtt != -1)
        printf("QUANTIDADE DE VEICULOS: %d\n", r2->qtt);
    else
        printf("QUANTIDADE DE VEICULOS: NAO PREENCHIDO\n");

    printf("\n");
    return 1;
}

int select_from_r2(FILE *bin_file){
    if(bin_file == NULL)    
        return -2;

    Record_t2* r2 = create_record_t2();

    fseek(bin_file, BINf_HEADER_SIZE, SEEK_SET);

    int record_size = 0;
    int i = 0;
    while((record_size = get_record_t2(bin_file, r2, 0)) != -2){
        if(record_size != -1){
            print_r2(r2);
        }
    }
    
    free_rec_t2(r2);
    return 1;
}


