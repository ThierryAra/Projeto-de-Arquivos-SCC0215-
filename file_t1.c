#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"file_t1.h"

#define REG_SIZE 97          //register size
#define BINf_HEADER_SIZE 182 //binary file_bin header size
#define STR_SIZE 30          //string size

int get_register_t1(FILE* file_bin, Register_t1* r1, int removed);

// Registro criado para um arquivo contendo registros de tamanho fixo
struct register_t1{
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

Register_t1* create_register_t1(){
    Register_t1* r1 = malloc(sizeof(Register_t1));

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
    r1->prox       = 0;
    return r1;
}

void free_reg_t1(Register_t1* r1){
    free(r1->cidade);
    free(r1->marca);
    free(r1->modelo);
    free(r1);
}

/*  Remove o lixo de cada registro */
void remove_trash(FILE* file, int quantity){
    char trash[REG_SIZE];

    fread(trash, REG_SIZE-quantity, sizeof(char), file);
}

int create_table_t1(FILE* file_csv, FILE* file_bin){
    if(file_bin == NULL || file_csv == NULL)
        return -2;
    
    Register_t1* r1 = create_register_t1();
    if(write_header_t1(file_bin) == -1)
        return -1;

    while(read_item_t1(file_csv, r1) > 0){
        write_item_t1(file_bin, r1);
    }

    //muda status para 1 (arquivo consistente de dados)
    fseek(file_bin, 0, SEEK_SET);
    fwrite("1", 1, sizeof(char), file_bin);
    free_reg_t1(r1);
    return 1;
}

int select_from_r1(FILE* file_bin){
    if(file_bin == NULL)    
        return -2;

    Register_t1* r1 = create_register_t1();
    //remover header
    char header[BINf_HEADER_SIZE];
    fread(header, BINf_HEADER_SIZE, sizeof(char), file_bin);

    int tam_registro = 0;
    while((tam_registro = get_register_t1(file_bin, r1, 0)) != -2){
        //remove o lixo restante ao fim de cada registro
        if(tam_registro != -1){
            remove_trash(file_bin, tam_registro);
            print_r1(r1);
        }
    }
    
    free_reg_t1(r1);
    return 1;
}

int select_from_where_r1(FILE* file_bin, char** fields, int n){
    if(file_bin == NULL || fields == NULL)
        return -2;

    //registro que sera comparado
    Register_t1* r1 = create_register_t1();
    //armazena n para ser retomado ao final do loop interno
    int N = n;
    //quantidade de registros a serem buscados
    int max_rrn = 0;
    fseek(file_bin, 174, SEEK_SET);
    fread(&max_rrn, 1, sizeof(int), file_bin);
    fseek(file_bin, 4, SEEK_CUR);
    
    //ira armazenar quantos registros buscados foram encontrados
    int encontrado = 0;
    //verifica se o registro foi removido e contem o tamanho do registro lido
    int status;
    //verifica se o registro possui os campos/valores iguais ao do registro buscado
    int error = 1;

    while(max_rrn-- > 0){
        status = get_register_t1(file_bin, r1, 0);
        if(status == -1) // registro excluido
            error = -1;
        else if(status == -2){ // erro de leitura
            free_reg_t1(r1);
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
            encontrado++;
        }

        //reseta error para uma proxima busca
        error = 1;

        //manda o ponteiro ate o proximo registro
        if(status > 0 && status < REG_SIZE)
            fseek(file_bin, REG_SIZE-status, SEEK_CUR);
    }

    free_reg_t1(r1);
    return encontrado;
}

int search_rrn(char* type_file, FILE* file_bin, int rrn, Register_t1* r1){
    if(file_bin == NULL || strcmp(type_file, "tipo1") != 0)
        return -2;
    
    //verifica se eh um RRN existente
    fseek(file_bin, 174, SEEK_SET);
    int x = 0;
    fread(&x, 1, sizeof(int), file_bin);
    
    if(rrn >= x)
        return -1;
    
    //manda o ponteiro ate o registro
    fseek(file_bin, (rrn*REG_SIZE)+BINf_HEADER_SIZE, SEEK_SET);

    if(get_register_t1(file_bin, r1, 0) < 1)
        return -1;

    return 1;
}

int write_header_t1(FILE* file_bin){
    if(file_bin == NULL)    
        return -2;

    //valores para serem salvos como padrao
    int i = 0, ni = -1;
    fwrite("0", 1, sizeof(char), file_bin);                        //status
    fwrite(&ni, 1, sizeof(int), file_bin);                         //topo
    fwrite("LISTAGEM DA FROTA DOS VEICULOS NO BRASIL",
            40, sizeof(char), file_bin);                           //descricao
    fwrite("CODIGO IDENTIFICADOR: ", 22, sizeof(char), file_bin);  //desC1
    fwrite("ANO DE FABRICACAO: ", 19, sizeof(char), file_bin);     //desC2
    fwrite("QUANTIDADE DE VEICULOS: ", 24, sizeof(char), file_bin);//desC3
    fwrite("ESTADO: ", 8, sizeof(char), file_bin);                 //desC4
    fwrite("0", 1, sizeof(char), file_bin);                        //codC5
    fwrite("NOME DA CIDADE: ", 16, sizeof(char), file_bin);        //desC5
    fwrite("1", 1, sizeof(char), file_bin);                        //codC6
    fwrite("MARCA DO VEICULO: ", 18, sizeof(char), file_bin);      //desC6
    fwrite("2", 1, sizeof(char), file_bin);                        //codC7
    fwrite("MODELO DO VEICULO: ", 19, sizeof(char), file_bin);     //desC7
    fwrite(&i, 1, sizeof(int), file_bin);                          //proxRRN
    fwrite(&i, 1, sizeof(int), file_bin);                          //nroRegRem
    return 1;
}

int read_item_t1(FILE* file_csv, Register_t1* r1){
    if(file_csv == NULL || r1 == NULL)
        return -2;

    char c;
    //id (sempre exite um e eh != 0)
    if(read_field_int(file_csv, &(r1->id)) == -1)
        return -1;
    //ano
    if(read_field_int(file_csv, &r1->ano) == -1)
        r1->ano = -1;
    else{
        //remove ".0" do ano
        fscanf(file_csv, "%c", &c); 
        fscanf(file_csv, "%c", &c);
    }
    //cidade
    if(read_field_char(r1->cidade, file_csv) < 1){
        r1->cidade[0]  = '\0';
        r1->tam_cidade = 0;
    }else
        r1->tam_cidade = strlen(r1->cidade);
    
    //quantidade
    if(read_field_int(file_csv, &r1->qtt) == -1)
        r1->qtt = -1;
    //sigla
    if(read_field_char(r1->sigla, file_csv) < 1){
        r1->sigla[0] = '$';
        r1->sigla[1] = '$';
    }
    //marca
    if(read_field_char(r1->marca, file_csv) < 1){
        r1->removido = '1';
        r1->marca[0]   = '\0';
        r1->tam_marca = 0;
    }else
        r1->tam_marca = strlen(r1->marca);
    //modelo
    if(read_field_char(r1->modelo, file_csv) < 1){
        r1->modelo[0]   = '\0';
        r1->tam_modelo = 0;
    }else
        r1->tam_modelo = strlen(r1->modelo);

    //remove o '\n'
    c = fgetc(file_csv);
    if(c != '\n')
        ungetc(c, file_csv);

    return 1;
}

int write_item_t1(FILE* file_bin, Register_t1* r1){
    if(file_bin == NULL || r1 == NULL)
        return -1;

    //busca a posicao que deve ser adicionado o registro
    fseek(file_bin, 174, SEEK_SET);
    int rrn = 0;
    fread(&rrn, 1, sizeof(int), file_bin);
    fseek(file_bin, BINf_HEADER_SIZE+(REG_SIZE*rrn), SEEK_SET);

    //dados estaticos
    int tam_registro = 19;
    fwrite(&r1->removido, 1, sizeof(char), file_bin);
    fwrite(&r1->prox, 1, sizeof(int), file_bin);
    fwrite(&r1->id, 1, sizeof(int), file_bin);
    fwrite(&r1->ano, 1, sizeof(int), file_bin);
    fwrite(&r1->qtt, 1, sizeof(int), file_bin);
    fwrite(r1->sigla, 2, sizeof(char), file_bin);

    //os if's verificam se o campo nao eh nulo
    if(r1->tam_cidade > 0){
        fwrite(&r1->tam_cidade, 1, sizeof(int), file_bin);
        fwrite(&r1->codC5, 1, sizeof(char), file_bin);
        fwrite(r1->cidade, r1->tam_cidade, sizeof(char), file_bin);
        tam_registro += 4 + 1 + r1->tam_cidade;
    }
    if(r1->tam_marca > 0){
        fwrite(&r1->tam_marca, 1, sizeof(int), file_bin);
        fwrite(&r1->codC6, 1, sizeof(char), file_bin);
        fwrite(r1->marca, r1->tam_marca, sizeof(char), file_bin);
        tam_registro += 4 + 1 + r1->tam_marca;
    }

    if(r1->tam_modelo > 0){
        fwrite(&r1->tam_modelo, 1, sizeof(int), file_bin);
        fwrite(&r1->codC7, 1, sizeof(char), file_bin);
        fwrite(r1->modelo, r1->tam_modelo, sizeof(char), file_bin);
        tam_registro += 4 + 1 + r1->tam_modelo;
    }
    //-----------
    // garante que cada registro tenha 97 bytes
    if(tam_registro < REG_SIZE)
        for(int i = tam_registro; i < REG_SIZE; i++)
            fwrite("$", 1, sizeof(char), file_bin);

    //Adiciona +1 em proxRRN
    int proxRRN = 0;
    //buscando o valor e somando
    fseek(file_bin, 174, SEEK_SET);
    fread(&proxRRN, 1, sizeof(int), file_bin);
    proxRRN++;
    //retornando para escrever
    fseek(file_bin, -4, SEEK_CUR);
    fwrite(&proxRRN, 1, sizeof(int), file_bin);

    return 1;
}

/*  Le e adiciona um campo de tamanho variado ah struct r1
    Retorna: soma dos campos lidos
             -1 caso nao exista o codigo do campo lido no arquivo */
int add_str_field(FILE* file_bin, Register_t1* r1){
    int tam_str = 0;
    fread(&tam_str, 1, sizeof(int), file_bin);
    char cod = '3';
    fread(&cod, 1, sizeof(char), file_bin);

    if(cod == 48){
        //cidade
        r1->tam_cidade = tam_str;
        r1->codC5      = cod;
        fread(r1->cidade, r1->tam_cidade, sizeof(char), file_bin);
        r1->cidade[tam_str] = '\0';
    }else if(cod == 49){
        //marca
        r1->tam_marca = tam_str;
        r1->codC6      = cod;
        fread(r1->marca, r1->tam_marca, sizeof(char), file_bin);
        r1->marca[tam_str] = '\0';
    }else if(cod == 50){
        //modelo
        r1->tam_modelo = tam_str;
        r1->codC7      = cod;
        fread(r1->modelo, r1->tam_modelo, sizeof(char), file_bin);
        r1->modelo[tam_str] = '\0';
    }else
        return -1;

    return tam_str+4+1;
}

/*  Le um registro do arquivo binario passado como parametro e 
    o armazena em r1
    Retorna (tamanho_registro em bytes) caso ocorra tudo normalmente
           -1 caso o registro tenha sido removido logicamente
           -2 caso haja erro na leitura (nao ha mais registros)   */
int get_register_t1(FILE* file_bin, Register_t1* r1, int removed){
    int tam_registro = 19;
    
    //campos de tamanho fixo
    if(fread(&r1->removido, 1, sizeof(char), file_bin) == 0)
        return -2; 

    //verifica se o registro nao foi removido logicamente
    //caso removed seja 1, ja se sabe que o campo foi removido e deve-se pular
    if(removed == 0 && r1->removido == '1'){
        ungetc(r1->removido, file_bin);
        //removendo o registro removido do buffer
        tam_registro = get_register_t1(file_bin, r1, 1);
        remove_trash(file_bin, tam_registro);

        return -1;
    }
    //prox
    fread(&r1->prox, 1, sizeof(int), file_bin);
    //id
    fread(&r1->id, 1, sizeof(int), file_bin);
    //ano
    fread(&r1->ano, 1, sizeof(int), file_bin);
    //qtt
    fread(&r1->qtt, 1, sizeof(int), file_bin);
    //sigla
    fread(&r1->sigla, 2, sizeof(char), file_bin);
    
    char c;
    //campos de tamanho variavel
    r1->tam_cidade = 0;
    r1->tam_marca  = 0;
    r1->tam_modelo = 0;
    
    //verifica se existem mais campos
    fread(&c, 1, sizeof(char), file_bin);
    if(c == '$') return tam_registro+1; //+1 do fread(&c)    
    else ungetc(c, file_bin);

    tam_registro += add_str_field(file_bin, r1);

    //verifica se existem mais campos
    fread(&c, 1, sizeof(char), file_bin);
    if(c == '$') return tam_registro+1; //+1 do fread(&c)
    else ungetc(c, file_bin);

    tam_registro += add_str_field(file_bin, r1);

    //verifica se existem mais campos
    fread(&c, 1, sizeof(char), file_bin);
    if(c == '$') return tam_registro+1; //+1 do fread(&c)
    else ungetc(c, file_bin);

    tam_registro += add_str_field(file_bin, r1);

    return tam_registro;
}

int print_r1(Register_t1* r1){
    if(r1 == NULL)
        return -1;

    //os if's verificam se o campo nao eh nulo
    if(r1->tam_marca > 0)
        printf("MARCA DO VEICULO: %s\n", r1->marca);    
    if(r1->tam_modelo > 0)
        printf("MODELO DO VEICULO: %s\n", r1->modelo); 
    if(r1->ano != -1)
        printf("ANO DE FABRICACAO: %d\n", r1->ano);    
    if(r1->tam_cidade > 0)
        printf("NOME DA CIDADE: %s\n", r1->cidade);
    if(r1->qtt != -1)
        printf("QUANTIDADE DE VEICULOS: %d\n", r1->qtt);

    printf("\n");
    return 1;
}