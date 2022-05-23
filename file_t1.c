#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_t1.h"
#include "write_read_file.h"

#define REC_SIZE 97          //record size
#define BINf_HEADER_SIZE 182 //binary bin_file header size
#define STR_SIZE 30          //string size

int get_record_t1(FILE* bin_file, Record_t1* r1, int removed);
void remove_trash(FILE* bin_file, int quantity);
int read_item_t1(FILE* csv_file, Record_t1* r1);

// Registro criado para um arquivo contendo registros de tamanho fixo
struct record_t1{
    char removido;
    int prox, id;
    int ano, qtt;
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

Record_t1* create_record_t1(){
    Record_t1* r1 = malloc(sizeof(Record_t1));

    r1->removido   = '0';
    r1->prox       = -1;
    r1->qtt        = -1;
    r1->tam_marca  = -1;
    r1->marca      = malloc(sizeof(char)*STR_SIZE);
    r1->tam_cidade = -1;
    r1->cidade     = malloc(sizeof(char)*STR_SIZE);
    r1->tam_modelo = -1;
    r1->modelo     = malloc(sizeof(char)*STR_SIZE);
    r1->codC5      = '0';
    r1->codC6      = '1';
    r1->codC7      = '2';
    r1->prox       = -1;
    return r1;
}

void free_rec_t1(Record_t1* r1){
    free(r1->cidade);
    free(r1->marca);
    free(r1->modelo);
    free(r1);
}

int create_table_t1(FILE* csv_file, FILE* bin_file){
    if(csv_file == NULL || bin_file == NULL)
        return -2;
    
    Record_t1* r1 = create_record_t1();
    if(write_header_t1(bin_file) == -2)
        return -1;

    while(read_item_t1(csv_file, r1) > 0){
        write_item_t1(bin_file, r1);
    }

    //muda status para 1 (arquivo consistente de dados)
    fseek(bin_file, 0, SEEK_SET);
    fwrite("1", 1, sizeof(char), bin_file);
    free_rec_t1(r1);
    return 1;
}

int select_from_r1(FILE* bin_file){
    if(bin_file == NULL)    
        return -2;

    Record_t1* r1 = create_record_t1();
    //remover header
    char header[BINf_HEADER_SIZE];
    fread(header, BINf_HEADER_SIZE, sizeof(char), bin_file);

    int record_size = 0;
    int i = 0;
    while((record_size = get_record_t1(bin_file, r1, 0)) != -2){
        //remove o lixo restante ao fim de cada registro
        if(record_size != -1){
            remove_trash(bin_file, record_size);
            print_r1(r1);
        }
    }
    
    free_rec_t1(r1);
    return 1;
}

int select_from_where_r1(FILE* bin_file, char** fields, int n){
    if(bin_file == NULL || fields == NULL)
        return -2;

    //registro que sera comparado
    Record_t1* r1 = create_record_t1();
    //armazena n para ser retomado ao final do loop interno
    int N = n;
    //quantidade de registros a serem buscados
    int max_rrn = 0;
    fseek(bin_file, 174, SEEK_SET);
    fread(&max_rrn, 1, sizeof(int), bin_file);
    fseek(bin_file, 4, SEEK_CUR);
    
    //ira armazenar quantos registros buscados foram encontrados
    int finded = 0;
    //verifica se o registro foi removido e contem o tamanho do registro lido
    int status;
    //verifica se o registro possui os campos/valores iguais ao do registro buscado
    int error = 1;

    while(max_rrn-- > 0){
        status = get_record_t1(bin_file, r1, 0);
        if(status == -1) // registro excluido
            error = -1;
        else if(status == -2){ // erro de leitura
            free_rec_t1(r1);
            return -1;
        }

        //indice de fields
        int i = 0;
        //mantem enquanto os campos de array sao iguais a r1
        while (error > 0 && i < n*2){
            if(strcmp(fields[i], "id") == 0){
                int x = atoi(fields[i + 1]);
                if(x != r1->id){
                    error = -1;
                }
            }else if(strcmp(fields[i], "ano") == 0){
                int x = atoi(fields[i + 1]);
                if(x != r1->ano){
                    error = -1;
                }
            }else if(strcmp(fields[i], "qtt") == 0){
                int x = atoi(fields[i + 1]);
                if(x != r1->qtt){
                    error = -1;
                }
            }else if(strcmp(fields[i], "sigla") == 0){
                if( fields[i+1][0] != r1->sigla[0] || 
                    fields[i+1][1] != r1->sigla[1] ){
                    
                    error = -1;
                }
            }else if(strcmp(fields[i], "cidade") == 0){
                if(r1->tam_cidade <= 0 || strcmp(fields[i+1], r1->cidade) != 0){
                    error = -1;
                }
            }else if(strcmp(fields[i], "marca") == 0){
                 if(r1->tam_marca <= 0 || strcmp(fields[i+1], r1->marca) != 0){
                    error = -1;
                }
            }else if(strcmp(fields[i], "modelo") == 0){
                 if(r1->tam_modelo <= 0 || strcmp(fields[i+1], r1->modelo) != 0){
                    error = -1;
                }
            }

            //proximo campo em fields
            i += 2;
        }

        //verifica se realmente eh o registro buscado
        if(error > 0){
            print_r1(r1);
            finded++;
        }

        //reseta error para uma proxima busca
        error = 1;

        //manda o ponteiro ate o proximo registro
        if(status > 0 && status < REC_SIZE)
            fseek(bin_file, REC_SIZE-status, SEEK_CUR);
    }

    free_rec_t1(r1);
    return finded;
}

int search_rrn(char* type_file, FILE* bin_file, int rrn, Record_t1* r1){
    if(bin_file == NULL || strcmp(type_file, "tipo1") != 0)
        return -2;
    
    //verifica se eh um RRN existente
    fseek(bin_file, 174, SEEK_SET);
    int x = 0;
    fread(&x, 1, sizeof(int), bin_file);
    if(rrn >= x)
        return -1;
    
    //manda o ponteiro ate o registro
    fseek(bin_file, (rrn*REC_SIZE)+BINf_HEADER_SIZE, SEEK_SET);

    if(get_record_t1(bin_file, r1, 0) < 1)
        return -1;

    return 1;
}

int write_header_t1(FILE* bin_file){
    if(bin_file == NULL)    
        return -2;

    //valores para serem salvos como padrao
    int i = 0, ni = -1;
    fwrite("0", 1, sizeof(char), bin_file);                        //status
    fwrite(&ni, 1, sizeof(int), bin_file);                         //topo
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
    fwrite(&i, 1, sizeof(int), bin_file);                          //proxRRN
    fwrite(&i, 1, sizeof(int), bin_file);                          //nroRegRem
    return 1;
}

int read_item_t1(FILE* csv_file, Record_t1* r1){
    if(csv_file == NULL || r1 == NULL)
        return -2;
        
    char c;
    char num[20];
    //id (sempre exite um e eh != 0)
    if(read_int_field(csv_file, &(r1->id)) == -1)
        return -1;
    //ano
    if(read_int_field(csv_file, &r1->ano) == -1)
        r1->ano = -1;
    //cidade
    if(read_char_field(r1->cidade, csv_file) < 1)   
        r1->tam_cidade = 0;
    else
        r1->tam_cidade = strlen(r1->cidade);
    //quantidade
    if(read_int_field(csv_file, &r1->qtt) == -1)
        r1->qtt = -1;
    //sigla
    if(read_char_field(r1->sigla, csv_file) < 1){
        r1->sigla[0] = '$';
        r1->sigla[1] = '$';
    }
    //marca
    if(read_char_field(r1->marca, csv_file) < 1)
        r1->tam_marca = 0;
    else
        r1->tam_marca = strlen(r1->marca);
    //modelo
    if(read_char_field(r1->modelo, csv_file) < 1)
        r1->tam_modelo = 0;
    else
        r1->tam_modelo = strlen(r1->modelo);

    //remove o '\n'
    c = fgetc(csv_file);
    if(c != '\n')
        ungetc(c, csv_file);

    return 1;
}

int write_item_t1(FILE* bin_file, Record_t1* r1){
    if(bin_file == NULL || r1 == NULL)
        return -2;

    //busca a posicao que deve ser adicionado o registro
    fseek(bin_file, 174, SEEK_SET);
    int rrn = 0;
    fread(&rrn, 1, sizeof(int), bin_file);
    fseek(bin_file, BINf_HEADER_SIZE+(REC_SIZE*rrn), SEEK_SET);

    //dados estaticos
    int record_size = 19;
    fwrite(&r1->removido, 1, sizeof(char), bin_file);
    r1->prox = -1;
    fwrite(&r1->prox, 1, sizeof(int), bin_file);
    fseek(bin_file, -4, SEEK_CUR);
    int i;
    fread(&i, 1, sizeof(int), bin_file);
    printf("%d ", i);
    fwrite(&r1->id, 1, sizeof(int), bin_file);
    fwrite(&r1->ano, 1, sizeof(int), bin_file);
    fwrite(&r1->qtt, 1, sizeof(int), bin_file);
    fwrite(r1->sigla, 2, sizeof(char), bin_file);

    //dados variaveis
    //os if's verificam se o campo nao eh nulo
    if(r1->tam_cidade > 0){
        fwrite(&r1->tam_cidade, 1, sizeof(int), bin_file);
        fwrite(&r1->codC5, 1, sizeof(char), bin_file);
        fwrite(r1->cidade, r1->tam_cidade, sizeof(char), bin_file);
        record_size += 4 + 1 + r1->tam_cidade;
    }
    if(r1->tam_marca > 0){
        fwrite(&r1->tam_marca, 1, sizeof(int), bin_file);
        fwrite(&r1->codC6, 1, sizeof(char), bin_file);
        fwrite(r1->marca, r1->tam_marca, sizeof(char), bin_file);
        record_size += 4 + 1 + r1->tam_marca;
    }
    if(r1->tam_modelo > 0){
        fwrite(&r1->tam_modelo, 1, sizeof(int), bin_file);
        fwrite(&r1->codC7, 1, sizeof(char), bin_file);
        fwrite(r1->modelo, r1->tam_modelo, sizeof(char), bin_file);
        record_size += 4 + 1 + r1->tam_modelo;
    }
    //-----------
    // garante que cada registro tenha 97 bytes
    if(record_size < REC_SIZE)
        for(int i = record_size; i < REC_SIZE; i++)
            fwrite("$", 1, sizeof(char), bin_file);

    //Adiciona +1 em proxRRN
    rrn += 1;
    fseek(bin_file, 174, SEEK_SET);
    fwrite(&rrn, 1, sizeof(int), bin_file);

    return 1;
}

/*  Le e adiciona um campo de tamanho variado ah struct r1
    Retorna: soma dos campos lidos
             -1 caso nao exista o codigo do campo lido no arquivo */
int add_str_field(FILE* bin_file, Record_t1* r1){
    int string_size = 0;
    fread(&string_size, 1, sizeof(int), bin_file);
    char cod = '3';
    fread(&cod, 1, sizeof(char), bin_file);

    if(cod == 48){
        //cidade (0)
        r1->tam_cidade = string_size;
        r1->codC5      = cod;
        fread(r1->cidade, r1->tam_cidade, sizeof(char), bin_file);
        r1->cidade[string_size] = '\0';
    }else if(cod == 49){
        //marca (1)
        r1->tam_marca = string_size;
        r1->codC6      = cod;
        fread(r1->marca, r1->tam_marca, sizeof(char), bin_file);
        r1->marca[string_size] = '\0';
    }else if(cod == 50){
        //modelo (2)
        r1->tam_modelo = string_size;
        r1->codC7      = cod;
        fread(r1->modelo, r1->tam_modelo, sizeof(char), bin_file);
        r1->modelo[string_size] = '\0';
    }else
        return -1;

    return string_size+4+1;
}

/*  Le um registro do arquivo binario passado como parametro e 
    o armazena em r1
    Retorna (tamanho_registro em bytes) caso ocorra tudo normalmente
           -1 caso o registro tenha sido removido logicamente
           -2 caso haja erro na leitura (nao ha mais registros)   */
int get_record_t1(FILE* bin_file, Record_t1* r1, int removed){
    int record_size = 19;
    
    //campos de tamanho fixo
    if(fread(&r1->removido, 1, sizeof(char), bin_file) == 0)
        return -2; 

    //verifica se o registro nao foi removido logicamente
    //caso removed seja 1, ja se sabe que o campo foi removido e deve-se pular
    if(removed == 0 && r1->removido == '1'){
        ungetc(r1->removido, bin_file);
        //removendo o registro removido do buffer
        record_size = get_record_t1(bin_file, r1, 1);
        remove_trash(bin_file, record_size);

        return -1;
    }
    //prox
    fread(&r1->prox, 1, sizeof(int), bin_file);
    //id
    fread(&r1->id, 1, sizeof(int), bin_file);
    //ano
    fread(&r1->ano, 1, sizeof(int), bin_file);
    //qtt
    fread(&r1->qtt, 1, sizeof(int), bin_file);
    //sigla
    fread(&r1->sigla, 2, sizeof(char), bin_file);
    
    char c;
    //campos de tamanho variavel
    r1->tam_cidade = 0;
    r1->tam_marca  = 0;
    r1->tam_modelo = 0;
    
    //verifica se existem mais campos
    fread(&c, 1, sizeof(char), bin_file);
    if(c == '$') return record_size+1; //+1 do fread(&c)    
    else ungetc(c, bin_file);

    record_size += add_str_field(bin_file, r1);

    //verifica se existem mais campos
    fread(&c, 1, sizeof(char), bin_file);
    if(c == '$') return record_size+1; //+1 do fread(&c)
    else ungetc(c, bin_file);

    record_size += add_str_field(bin_file, r1);

    //verifica se existem mais campos
    fread(&c, 1, sizeof(char), bin_file);
    if(c == '$') return record_size+1; //+1 do fread(&c)
    else ungetc(c, bin_file);

    record_size += add_str_field(bin_file, r1);

    return record_size;
}

int print_r1(Record_t1* r1){
    if(r1 == NULL)
        return -1;

    //os if's verificam se o campo nao eh nulo
    if(r1->tam_marca > 0)
        printf("MARCA DO VEICULO: %s\n", r1->marca);  
    else  
        printf("MARCA DO VEICULO: NAO PREENCHIDO\n");  
    if(r1->tam_modelo > 0)
        printf("MODELO DO VEICULO: %s\n", r1->modelo);
    else 
        printf("MODELO DO VEICULO: NAO PREENCHIDO\n");
    if(r1->ano != -1)
        printf("ANO DE FABRICACAO: %d\n", r1->ano); 
    else   
        printf("ANO DE FABRICACAO: NAO PREENCHIDO\n"); 
    if(r1->tam_cidade > 0)
        printf("NOME DA CIDADE: %s\n", r1->cidade);
    else
        printf("NOME DA CIDADE: NAO PREENCHIDO\n");
    if(r1->qtt != -1)
        printf("QUANTIDADE DE VEICULOS: %d\n", r1->qtt);
    else
        printf("QUANTIDADE DE VEICULOS: NAO PREENCHIDO\n");

    printf("\n");
    return 1;
}

/*  Remove o lixo de cada registro */
void remove_trash(FILE* file, int quantity){
    char trash[REC_SIZE];

    fread(trash, REC_SIZE-quantity, sizeof(char), file);
}
