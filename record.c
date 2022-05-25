#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"write_read_file.h"
#include"header.h"
#include"record.h"

#define STATIC_REC_SIZE 97       //tamanho do registro
#define STATIC_REC_HEADER 182    //tamanho do cabecalho do registro de tam estatico
#define VARIABLE_REC_HEADER 190  //tamanho do cabecalho do registro de tam variavel
#define STR_SIZE 30              //tamanho da string

int get_record(FILE* bin_file, RECORD* r, HEADER* header, int type_file);
void next_register(FILE* bin_file, int quantity, int type_file);
int read_item_csv(FILE* csv_file, RECORD* r);
int write_item(FILE* bin_file, RECORD* r, HEADER* header, int type_file, int record_size);

// Registro criado para um arquivo contendo registros de tamanho fixo
struct record{
    char removido;
    int id, ano, qtt;
    char sigla[2];
    int tam_cidade;
    char* cidade;
    int tam_marca;
    char* marca;
    int tam_modelo;
    char* modelo;
};

RECORD* create_record(){
    RECORD* r = malloc(sizeof(RECORD));

    r->removido   = '0';
    r->qtt        = -1;
    r->tam_marca  = -1;
    r->marca      = malloc(sizeof(char)*STR_SIZE);
    r->tam_cidade = -1;
    r->cidade     = malloc(sizeof(char)*STR_SIZE);
    r->tam_modelo = -1;
    r->modelo     = malloc(sizeof(char)*STR_SIZE);
    return r;
}

void free_rec(RECORD* r){
    free(r->cidade);
    free(r->marca);
    free(r->modelo);
    free(r);
}

int sum_vars(RECORD* r, int sum_initial){
    if(r->tam_cidade > 0)
        sum_initial += 5 + r->tam_cidade;
    if(r->tam_modelo > 0)
        sum_initial += 5 + r->tam_modelo;
    if(r->tam_marca > 0)
        sum_initial += 5 + r->tam_marca;

    return sum_initial;
}

int create_table(FILE* csv_file, FILE* bin_file, int type_file){
    if(csv_file == NULL || bin_file == NULL)
        return -2;
    
    HEADER* header = create_header();
    
    RECORD* r = create_record();
    if(type_file == 1){
        if(write_header(header, bin_file, 1) == -2)
            return -1;
    }else{
        if(write_header(header, bin_file, 2) == -2)
            return -1;
    }

    int record_size = 0;
    while(read_item_csv(csv_file, r) > 0){        
        if(type_file == 1){
            record_size = sum_vars(r, 19);
            write_item(bin_file, r, header, 1, record_size);
        }else if (type_file == 2){
            record_size = sum_vars(r, 22);
            write_item(bin_file, r, header, 2, record_size);
        }
    }

    // Altera o campo proxRRN ou ProxByteOffset
    if(type_file == 1){
        fseek(bin_file, 0, SEEK_END);
        int BOS = (ftell(bin_file) - STATIC_REC_HEADER) / STATIC_REC_SIZE;
        fseek(bin_file, 174, SEEK_SET);
        fwrite(&BOS, 1, sizeof(int), bin_file);
    }else{
        fseek(bin_file, 0, SEEK_END);
        long int BOS = ftell(bin_file);
        fseek(bin_file, 178, SEEK_SET);
        fwrite(&BOS, 1, sizeof(int), bin_file);
    }

    //muda status para 1 (arquivo consistente de dados)
    atualiza_status(header, bin_file);
    free_rec(r);
    free_header(header);
    return 1;
}

int select_from(FILE* bin_file, int type_file){
    if(bin_file == NULL)    
        return -2;

    //verifica se o arquivo esta inconsistente
    int status = verifica_status(bin_file);
    if(status == 0 || status == -1)
        return -2;

    RECORD* r = create_record();
    HEADER* header = create_header();

    //pula o header
    if(type_file == 1) fseek(bin_file, STATIC_REC_HEADER, SEEK_SET);
    else               fseek(bin_file, VARIABLE_REC_HEADER, SEEK_SET);

    int record_size = 0;
    int i = 0;
    while((record_size = get_record(bin_file, r, header, type_file)) != -2)
        if(record_size != -1) //verifica se o registro nao esta removido
            print_record(r);
    
    free(header);
    free_rec(r);
    return 1;
}

int select_from_where(FILE* bin_file, char** fields, int n, int type_file){
    if(bin_file == NULL || fields == NULL)
        return -2;

    //verifica se o arquivo esta inconsistente
    int status = verifica_status(bin_file);
    if(status == 0 || status == -1)
        return -2;

    //registro que sera comparado
    RECORD* r = create_record();
    HEADER* header = create_header();

    if(type_file == 1) fseek(bin_file, STATIC_REC_HEADER, SEEK_SET);
    else               fseek(bin_file, VARIABLE_REC_HEADER, SEEK_SET);
    
    //ira armazenar quantos registros buscados foram encontrados
    int found = 0;
    //verifica se o registro foi removido e contem o tamanho do registro lido
    int record_size = 0;
    //verifica se o registro possui os campos/valores iguais ao do registro buscado
    int error = 1;

    //indice de fields
    int i = 0;
    while((record_size = get_record(bin_file, r, header, type_file)) != -2){
        if(record_size == -1) // registro excluido
            error = -1;
        
        //mantem enquanto os campos de array sao iguais a r
        while (error > 0 && i < n*2){
            if(strcmp(fields[i], "id") == 0){
                int x = atoi(fields[i + 1]);
                if(x != r->id){
                    error = -1;
                }
            }else if(strcmp(fields[i], "ano") == 0){
                int x = atoi(fields[i + 1]);
                if(x != r->ano){
                    error = -1;
                }
            }else if(strcmp(fields[i], "qtt") == 0){
                int x = atoi(fields[i + 1]);
                if(x != r->qtt){
                    error = -1;
                }
            }else if(strcmp(fields[i], "sigla") == 0){
                if( fields[i+1][0] != r->sigla[0] || 
                    fields[i+1][1] != r->sigla[1] ){
                    
                    error = -1;
                }
            }else if(strcmp(fields[i], "cidade") == 0){
                if(r->tam_cidade <= 0 || strcmp(fields[i+1], r->cidade) != 0){
                    error = -1;
                }
            }else if(strcmp(fields[i], "marca") == 0){
                 if(r->tam_marca <= 0 || strcmp(fields[i+1], r->marca) != 0){
                    error = -1;
                }
            }else if(strcmp(fields[i], "modelo") == 0){
                 if(r->tam_modelo <= 0 || strcmp(fields[i+1], r->modelo) != 0){
                    error = -1;
                }
            }else
                error = -1;

            //proximo campo em fields
            i += 2;
        }

        //verifica se realmente eh o registro buscado
        if(error > 0){
            print_record(r);
            found++;
        }

        //reseta error para uma proxima busca
        error = 1;
        i = 0;
    }

    free_header(header);
    free_rec(r);
    return found;
}

int search_rrn(char* type_file, FILE* bin_file, int rrn, RECORD* r){
    if(bin_file == NULL || strcmp(type_file, "tipo1") != 0)
        return -2;
    
    //verifica se eh um RRN existente
    fseek(bin_file, 174, SEEK_SET);
    int x = 0;
    fread(&x, 1, sizeof(int), bin_file);
    if(rrn >= x)
        return -1;
    
    //manda o ponteiro ate o registro
    fseek(bin_file, (rrn*STATIC_REC_SIZE)+STATIC_REC_HEADER, SEEK_SET);
    HEADER* header = create_header();

    if(get_record(bin_file, r, header, 1) < 1)
        return -1;

    free_header(header);
    return 1;
}

int read_item_csv(FILE* csv_file, RECORD* r){
    if(csv_file == NULL || r == NULL)
        return -2;
        
    char c;
    
    //id (sempre exite um e eh != 0)
    if(read_int_field(csv_file, &(r->id)) == -1)
        return -1;
    //ano
    if(read_int_field(csv_file, &r->ano) == -1)
        r->ano = -1;
    //cidade
    if(read_char_field(r->cidade, csv_file) < 1)   
        r->tam_cidade = 0;
    else
        r->tam_cidade = strlen(r->cidade);
    //quantidade
    if(read_int_field(csv_file, &r->qtt) == -1)
        r->qtt = -1;
    //sigla
    if(read_char_field(r->sigla, csv_file) < 1){
        strcpy(r->sigla, "$$");
        //r->sigla[0] = '$';
        //r->sigla[1] = '$';
    }
    //marca
    if(read_char_field(r->marca, csv_file) < 1)
        r->tam_marca = 0;
    else
        r->tam_marca = strlen(r->marca);
    //modelo
    if(read_char_field(r->modelo, csv_file) < 1)
        r->tam_modelo = 0;
    else
        r->tam_modelo = strlen(r->modelo);

    //remove o '\n'
    c = fgetc(csv_file);
    if(c != '\n')
        ungetc(c, csv_file);

    return 1;
}

/*  Escreve um registro (r) no arquivo .bin 
    Retorna 1 caso nao haja nenhum erro
           -1 caso os parametros estejam corrompidos */
int write_item(FILE* bin_file, RECORD* r, HEADER* header, int type_file, int record_size){
    if(bin_file == NULL || r == NULL)
        return -2;

    int i = -1;
    long int li = -1;

    //----dados estaticos
    fwrite(&r->removido, 1, sizeof(char), bin_file);
    
    //topo
    if(type_file == 1)
        fwrite(&i, 1, sizeof(int), bin_file);    
    else if(type_file == 2){
        //tamanho do registro
        fwrite(&record_size, 1, sizeof(int), bin_file);
        fwrite(&li, 1, sizeof(long int), bin_file);
    }

    fwrite(&r->id, 1, sizeof(int), bin_file);
    fwrite(&r->ano, 1, sizeof(int), bin_file);
    fwrite(&r->qtt, 1, sizeof(int), bin_file);
    fwrite(r->sigla, 2, sizeof(char), bin_file);

    //----dados variaveis
    //os if's verificam se o campo nao eh nulo
    if(r->tam_cidade > 0){
        fwrite(&r->tam_cidade, 1, sizeof(int), bin_file);
        fwrite(&header->codC5, 1, sizeof(char), bin_file);
        fwrite(r->cidade, r->tam_cidade, sizeof(char), bin_file);
    }
    if(r->tam_marca > 0){
        fwrite(&r->tam_marca, 1, sizeof(int), bin_file);
        fwrite(&header->codC6, 1, sizeof(char), bin_file);
        fwrite(r->marca, r->tam_marca, sizeof(char), bin_file);
    }
    if(r->tam_modelo > 0){
        fwrite(&r->tam_modelo, 1, sizeof(int), bin_file);
        fwrite(&header->codC7, 1, sizeof(char), bin_file);
        fwrite(r->modelo, r->tam_modelo, sizeof(char), bin_file);
    }
    //-----------
    // garante que cada registro tenha 97 bytes
    if(type_file == 1)
        for(int i = record_size; i < STATIC_REC_SIZE; i++)
            fwrite("$", 1, sizeof(char), bin_file);

    return 1;
}

/*  Le e adiciona um campo de tamanho variado ah struct r
    Retorna: soma dos campos lidos
             -1 caso nao exista o codigo do campo lido no arquivo */
int add_str_field(FILE* bin_file, RECORD* r, HEADER* header){
    int string_size = 0;
    fread(&string_size, 1, sizeof(int), bin_file);
    char cod = '3';
    fread(&cod, 1, sizeof(char), bin_file);

    if(cod == header->codC5){
        //cidade (0)
        r->tam_cidade = string_size;
        fread(r->cidade, r->tam_cidade, sizeof(char), bin_file);
        r->cidade[string_size] = '\0';
    }else if(cod == header->codC6){
        //marca (1)
        r->tam_marca = string_size;
        fread(r->marca, r->tam_marca, sizeof(char), bin_file);
        r->marca[string_size] = '\0';
    }else if(cod == header->codC7){
        //modelo (2)
        r->tam_modelo = string_size;
        fread(r->modelo, r->tam_modelo, sizeof(char), bin_file);
        r->modelo[string_size] = '\0';
    }

    return string_size+4+1;
}

int read_fields_t1(FILE* bin_file, HEADER* header, RECORD* r, int* record_size){
    char c = 0;

    //verifica se existem mais campos
    fread(&c, 1, sizeof(char), bin_file);
    if(c == '$') return (++(*record_size)); //+1 do fread(&c)    
    else ungetc(c, bin_file);
    *record_size += add_str_field(bin_file, r, header);

    if(*record_size == STATIC_REC_SIZE) return STATIC_REC_SIZE;
    
    //verifica se existem mais campos
    fread(&c, 1, sizeof(char), bin_file);
    if(c == '$') return (++(*record_size)); //+1 do fread(&c)    
    else ungetc(c, bin_file);

    *record_size += add_str_field(bin_file, r, header);
    if(*record_size == STATIC_REC_SIZE) return STATIC_REC_SIZE;
    
    //verifica se existem mais campos
    fread(&c, 1, sizeof(char), bin_file);
    if(c == '$') return (++(*record_size)); //+1 do fread(&c)    
    else ungetc(c, bin_file);

    *record_size += add_str_field(bin_file, r, header);
}

int read_fields_t2(FILE* bin_file, HEADER* header, RECORD* r, int* bytes_lidos, int record_size){
    //verifica se existem mais campos
    if(*bytes_lidos == record_size){
        return record_size;
    }
    
    *bytes_lidos += add_str_field(bin_file, r, header);

    //verifica se existem mais campos
    if(*bytes_lidos == record_size){
        return record_size;
    }

    *bytes_lidos += add_str_field(bin_file, r, header);

    //verifica se existem mais campos
    if(*bytes_lidos == record_size){
        return record_size;
    }

    *bytes_lidos += add_str_field(bin_file, r, header);

    return record_size;
}

/*  Le um registro do arquivo binario passado como parametro e 
    o armazena em r
    Retorna (tamanho_registro em bytes) caso ocorra tudo normalmente
           -1 caso o registro tenha sido removido logicamente
           -2 caso haja erro na leitura (nao ha mais registros)   */
int get_record(FILE* bin_file, RECORD* r, HEADER* header, int type_file){
    int bytes_lidos = 0;

    //campos de tamanho fixo
    if(fread(&r->removido, 1, sizeof(char), bin_file) == 0)
        return -2; 
        
    //verifica se o registro nao foi removido logicamente
    if(r->removido == '1'){
        if(type_file == 1)
            fseek(bin_file, STATIC_REC_SIZE-1, SEEK_CUR);
        else if(type_file == 2){
            fread(&bytes_lidos, 1, sizeof(int), bin_file);
            fseek(bin_file, bytes_lidos, SEEK_CUR);
        }    
        return -1;
    }

    int record_size = 0;
    
    //prox
    if(type_file == 1){
        bytes_lidos = 19;
        int prox = 0;
        fread(&prox, 1, sizeof(int), bin_file);
    }else if(type_file == 2){
        bytes_lidos = 22;
        fread(&record_size, 1, sizeof(int), bin_file);
        long int prox;
        fread(&prox, 1, sizeof(long int), bin_file);
    }
    
    
    //id
    fread(&r->id, 1, sizeof(int), bin_file);
    //ano
    fread(&r->ano, 1, sizeof(int), bin_file);
    //qtt
    fread(&r->qtt, 1, sizeof(int), bin_file);
    //sigla
    fread(&r->sigla, 2, sizeof(char), bin_file);
    
    char c;
    //campos de tamanho variavel
    r->tam_cidade = 0;
    r->tam_marca  = 0;
    r->tam_modelo = 0;
    
    if(type_file == 1)
        read_fields_t1(bin_file, header, r, &bytes_lidos);
    else if(type_file == 2)
        read_fields_t2(bin_file, header, r, &bytes_lidos, record_size);

    //remove o lixo, do registro, do buffer
    if(type_file == 1)
        fseek(bin_file, STATIC_REC_SIZE - bytes_lidos, SEEK_CUR);
    

    return 1;
}

int print_record(RECORD* r){
    if(r == NULL)
        return -1;

    //os if's verificam se o campo nao eh nulo
    if(r->tam_marca > 0)
        printf("MARCA DO VEICULO: %s\n", r->marca);  
    else  
        printf("MARCA DO VEICULO: NAO PREENCHIDO\n");  
    if(r->tam_modelo > 0)
        printf("MODELO DO VEICULO: %s\n", r->modelo);
    else 
        printf("MODELO DO VEICULO: NAO PREENCHIDO\n");
    if(r->ano != -1)
        printf("ANO DE FABRICACAO: %d\n", r->ano); 
    else   
        printf("ANO DE FABRICACAO: NAO PREENCHIDO\n"); 
    if(r->tam_cidade > 0)
        printf("NOME DA CIDADE: %s\n", r->cidade);
    else
        printf("NOME DA CIDADE: NAO PREENCHIDO\n");
    if(r->qtt != -1)
        printf("QUANTIDADE DE VEICULOS: %d\n", r->qtt);
    else
        printf("QUANTIDADE DE VEICULOS: NAO PREENCHIDO\n");

    printf("\n");
    return 1;
}

/*  Remove o lixo de cada registro */
void next_register(FILE* bin_file, int quantity, int type_file){
    if(type_file == 1)
        fseek(bin_file, STATIC_REC_SIZE-1, SEEK_CUR);
    else if(type_file == 2){
        fseek(bin_file, quantity, SEEK_CUR);
    }
}
