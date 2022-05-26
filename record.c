#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"write_read_file.h"
#include"header.h"
#include"record.h"

#define STATIC_REC_SIZE 97       
#define STATIC_REC_HEADER 182 
#define VARIABLE_REC_HEADER 190 
#define STR_SIZE 30   

int get_record(FILE* bin_file, RECORD* r, HEADER* header, int type_file);
void next_register(FILE* bin_file, int quantity, int type_file);
int read_item_csv(FILE* csv_file, RECORD* r);
int write_item(FILE* bin_file, RECORD* r, HEADER* header, int type_file, int record_size);

// Record created for a file containing fixed-length records
struct record{
    char removed;
    int id, year, amount;
    char abbreviation[2];
    int city_size;
    char* city;
    int brand_size;
    char* brand;
    int model_size;
    char* model;
};

RECORD* create_record(){
    RECORD* r = malloc(sizeof(RECORD));

    r->removed   = '0';
    r->amount        = -1;
    r->brand_size  = -1;
    r->brand      = malloc(sizeof(char)*STR_SIZE);
    r->city_size = -1;
    r->city     = malloc(sizeof(char)*STR_SIZE);
    r->model_size = -1;
    r->model     = malloc(sizeof(char)*STR_SIZE);
    return r;
}

void free_rec(RECORD* r){
    free(r->city);
    free(r->brand);
    free(r->model);
    free(r);
}

int sum_vars(RECORD* r, int initial_sum){
    if(r->city_size > 0)
        initial_sum += 5 + r->city_size;
    if(r->model_size > 0)
        initial_sum += 5 + r->model_size;
    if(r->brand_size > 0)
        initial_sum += 5 + r->brand_size;

    return initial_sum;
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

    // Changes these fields: 'nextRRN' ou 'nextByteOffset'
    if(type_file == 1){
        fseek(bin_file, 0, SEEK_END);
        int BOS = (ftell(bin_file) - STATIC_REC_HEADER) / STATIC_REC_SIZE;
        fseek(bin_file, 174, SEEK_SET);
        fwrite(&BOS, 1, sizeof(int), bin_file);
    }else{
        fseek(bin_file, 0, SEEK_END);
        long int BOS = ftell(bin_file);
        fseek(bin_file, 178, SEEK_SET);
        fwrite(&BOS, 1, sizeof(long int), bin_file);
    }

    //changes status to 1 (consistent data file)
    update_status(header, bin_file);
    free_rec(r);
    free_header(header);
    return 1;
}

int select_from(FILE* bin_file, int type_file){
    if(bin_file == NULL)    
        return -2;

    //checks if the file is inconsistent
    int status = check_status(bin_file);
    if(status == 0 || status == -1)
        return -2;

    RECORD* r = create_record();
    HEADER* header = create_header();

    //jump the header
    if(type_file == 1) fseek(bin_file, STATIC_REC_HEADER, SEEK_SET);
    else               fseek(bin_file, VARIABLE_REC_HEADER, SEEK_SET);

    int record_size = 0;
    int i = 0;
    while((record_size = get_record(bin_file, r, header, type_file)) != -2)
        if(record_size != -1) //checks that the registry is not removed
            print_record(r);
    
    free(header);
    free_rec(r);
    return 1;
}

int select_from_where(FILE* bin_file, char** fields, int n, int type_file){
    if(bin_file == NULL || fields == NULL)
        return -2;

    //checks if the file is inconsistent
    int status = check_status(bin_file);
    if(status == 0 || status == -1)
        return -2;

    //record that will be compared
    RECORD* r = create_record();
    HEADER* header = create_header();

    if(type_file == 1) fseek(bin_file, STATIC_REC_HEADER, SEEK_SET);
    else               fseek(bin_file, VARIABLE_REC_HEADER, SEEK_SET);
    
    //will store how many searched records were found
    int found = 0;
    //checks whether the record was removed and contains the size of the record read
    int record_size = 0;
    //checks if the record has the same fields/values as the searched record
    int error = 1;

    //fields index
    int i = 0;
    while((record_size = get_record(bin_file, r, header, type_file)) != -2){
        if(record_size == -1) // excluded register
            error = -1;
        
        //holds as long as the array fields are equal to r
        while (error > 0 && i < n*2){
            if(strcmp(fields[i], "id") == 0){
                int x = atoi(fields[i + 1]);
                if(x != r->id){
                    error = -1;
                }
            }else if(strcmp(fields[i], "year") == 0){
                int x = atoi(fields[i + 1]);
                if(x != r->year){
                    error = -1;
                }
            }else if(strcmp(fields[i], "amount") == 0){
                int x = atoi(fields[i + 1]);
                if(x != r->amount){
                    error = -1;
                }
            }else if(strcmp(fields[i], "abbreviation") == 0){
                if( fields[i+1][0] != r->abbreviation[0] || 
                    fields[i+1][1] != r->abbreviation[1] ){
                    
                    error = -1;
                }
            }else if(strcmp(fields[i], "city") == 0){
                if(r->city_size <= 0 || strcmp(fields[i+1], r->city) != 0){
                    error = -1;
                }
            }else if(strcmp(fields[i], "brand") == 0){
                 if(r->brand_size <= 0 || strcmp(fields[i+1], r->brand) != 0){
                    error = -1;
                }
            }else if(strcmp(fields[i], "model") == 0){
                 if(r->model_size <= 0 || strcmp(fields[i+1], r->model) != 0){
                    error = -1;
                }
            }else
                error = -1;

            //next field on 'fields' 
            i += 2;
        }

        //checks if it is really the record you are looking for
        if(error > 0){
            print_record(r);
            found++;
        }

        //reset error for a next search
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
    
    //check if it is an existing RRN
    fseek(bin_file, 174, SEEK_SET);
    int x = 0;
    fread(&x, 1, sizeof(int), bin_file);
    if(rrn >= x)
        return -1;
    
    //sends the pointer to the register
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
    
    //there is always an id and is != 0
    if(read_int_field(csv_file, &(r->id)) == -1)
        return -1;
    
    if(read_int_field(csv_file, &r->year) == -1)
        r->year = -1;
    
    if(read_char_field(r->city, csv_file) < 1)   
        r->city_size = 0;
    else
        r->city_size = strlen(r->city);
   
    if(read_int_field(csv_file, &r->amount) == -1)
        r->amount = -1;

    if(read_char_field(r->abbreviation, csv_file) < 1)
        strcpy(r->abbreviation, "$$");
    
    if(read_char_field(r->brand, csv_file) < 1)
        r->brand_size = 0;
    else
        r->brand_size = strlen(r->brand);
    
    if(read_char_field(r->model, csv_file) < 1)
        r->model_size = 0;
    else
        r->model_size = strlen(r->model);

    //removes '\n'
    c = fgetc(csv_file);
    if(c != '\n')
        ungetc(c, csv_file);

    return 1;
}

/*  Write a record (r) to the .bin file 
    Returns 1 if there is no error
           -1 if the parameters are corrupted */
int write_item(FILE* bin_file, RECORD* r, HEADER* header, int type_file, int record_size){
    if(bin_file == NULL || r == NULL)
        return -2;

    int i = -1;
    long int li = -1;

    //----static data
    fwrite(&r->removed, 1, sizeof(char), bin_file);
    
    //top
    if(type_file == 1)
        fwrite(&i, 1, sizeof(int), bin_file);    
    else if(type_file == 2){
        fwrite(&record_size, 1, sizeof(int), bin_file);
        fwrite(&li, 1, sizeof(long int), bin_file);
    }

    fwrite(&r->id, 1, sizeof(int), bin_file);
    fwrite(&r->year, 1, sizeof(int), bin_file);
    fwrite(&r->amount, 1, sizeof(int), bin_file);
    fwrite(r->abbreviation, 2, sizeof(char), bin_file);

    //----variable data
    //the if's check if the field is not null
    if(r->city_size > 0){
        fwrite(&r->city_size, 1, sizeof(int), bin_file);
        fwrite(&header->codC5, 1, sizeof(char), bin_file);
        fwrite(r->city, r->city_size, sizeof(char), bin_file);
    }
    if(r->brand_size > 0){
        fwrite(&r->brand_size, 1, sizeof(int), bin_file);
        fwrite(&header->codC6, 1, sizeof(char), bin_file);
        fwrite(r->brand, r->brand_size, sizeof(char), bin_file);
    }
    if(r->model_size > 0){
        fwrite(&r->model_size, 1, sizeof(int), bin_file);
        fwrite(&header->codC7, 1, sizeof(char), bin_file);
        fwrite(r->model, r->model_size, sizeof(char), bin_file);
    }

    //ensures that each record is 97 bytes long
    if(type_file == 1)
        for(int i = record_size; i < STATIC_REC_SIZE; i++)
            fwrite("$", 1, sizeof(char), bin_file);

    return 1;
}

/*  Read and add a field of varying size ah struct r
    Returns: sum of the fields read
            -1 if the read field code does not exist in the file */
int add_str_field(FILE* bin_file, RECORD* r, HEADER* header){
    int string_size = 0;
    fread(&string_size, 1, sizeof(int), bin_file);
    char cod = '3';
    fread(&cod, 1, sizeof(char), bin_file);

    if(cod == header->codC5){
        //city (0)
        r->city_size = string_size;
        fread(r->city, r->city_size, sizeof(char), bin_file);
        r->city[string_size] = '\0';
    }else if(cod == header->codC6){
        //brand (1)
        r->brand_size = string_size;
        fread(r->brand, r->brand_size, sizeof(char), bin_file);
        r->brand[string_size] = '\0';
    }else if(cod == header->codC7){
        //model (2)
        r->model_size = string_size;
        fread(r->model, r->model_size, sizeof(char), bin_file);
        r->model[string_size] = '\0';
    }

    return string_size+4+1;
}

int read_fields_t1(FILE* bin_file, HEADER* header, RECORD* r, int* record_size){
    char c = 0;

    //checks if there are more fields
    fread(&c, 1, sizeof(char), bin_file);
    if(c == '$') return (++(*record_size)); //+1 from fread(&c)    
    else ungetc(c, bin_file);
    *record_size += add_str_field(bin_file, r, header);

    if(*record_size == STATIC_REC_SIZE) return STATIC_REC_SIZE;
    
    //checks if there are more fields
    fread(&c, 1, sizeof(char), bin_file);
    if(c == '$') return (++(*record_size)); //+1 from fread(&c)    
    else ungetc(c, bin_file);

    *record_size += add_str_field(bin_file, r, header);
    if(*record_size == STATIC_REC_SIZE) return STATIC_REC_SIZE;
    
    //checks if there are more fields
    fread(&c, 1, sizeof(char), bin_file);
    if(c == '$') return (++(*record_size)); //+1 from fread(&c)    
    else ungetc(c, bin_file);

    *record_size += add_str_field(bin_file, r, header);
}

int read_fields_t2(FILE* bin_file, HEADER* header, RECORD* r, int* bytes_scanned, int record_size){
    //checks if there are more fields
    if(*bytes_scanned == record_size){
        return record_size;
    }
    
    *bytes_scanned += add_str_field(bin_file, r, header);

    //checks if there are more fields
    if(*bytes_scanned == record_size){
        return record_size;
    }

    *bytes_scanned += add_str_field(bin_file, r, header);

    //checks if there are more fields
    if(*bytes_scanned == record_size){
        return record_size;
    }

    *bytes_scanned += add_str_field(bin_file, r, header);

    return record_size;
}

/*  Reads one record from the binary file passed as parameter and 
    stores it in r
    Returns (record_size in bytes) if everything happens normally
           -1 if the record was logically removed
           -2 if there is a read error (no more records)   */
int get_record(FILE* bin_file, RECORD* r, HEADER* header, int type_file){
    int bytes_scanned = 0;

    //fixed length fields
    if(fread(&r->removed, 1, sizeof(char), bin_file) == 0)
        return -2; 
        
    //checks if the record was not logically removed
    if(r->removed == '1'){
        if(type_file == 1)
            fseek(bin_file, STATIC_REC_SIZE-1, SEEK_CUR);
        else if(type_file == 2){
            fread(&bytes_scanned, 1, sizeof(int), bin_file);
            fseek(bin_file, bytes_scanned, SEEK_CUR);
        }    
        return -1;
    }

    int record_size = 0;
    
    //next
    if(type_file == 1){
        bytes_scanned = 19;
        int next = 0;
        fread(&next, 1, sizeof(int), bin_file);
    }else if(type_file == 2){
        bytes_scanned = 22;
        fread(&record_size, 1, sizeof(int), bin_file);
        long int next;
        fread(&next, 1, sizeof(long int), bin_file);
    }
    
    fread(&r->id, 1, sizeof(int), bin_file);

    fread(&r->year, 1, sizeof(int), bin_file);
  
    fread(&r->amount, 1, sizeof(int), bin_file);

    fread(&r->abbreviation, 2, sizeof(char), bin_file);
    
    char c;
    //variable size fields
    r->city_size = 0;
    r->brand_size  = 0;
    r->model_size = 0;
    
    if(type_file == 1)
        read_fields_t1(bin_file, header, r, &bytes_scanned);
    else if(type_file == 2)
        read_fields_t2(bin_file, header, r, &bytes_scanned, record_size);

    //removes garbage from registry and from buffer
    if(type_file == 1)
        fseek(bin_file, STATIC_REC_SIZE - bytes_scanned, SEEK_CUR);
    
    return 1;
}

int print_record(RECORD* r){
    if(r == NULL)
        return -1;

    //the if's check if the field is not null
    if(r->brand_size > 0)
        printf("MARCA DO VEICULO: %s\n", r->brand);  
    else  
        printf("MARCA DO VEICULO: NAO PREENCHIDO\n");  
    if(r->model_size > 0)
        printf("MODELO DO VEICULO: %s\n", r->model);
    else 
        printf("MODELO DO VEICULO: NAO PREENCHIDO\n");
    if(r->year != -1)
        printf("ANO DE FABRICACAO: %d\n", r->year); 
    else   
        printf("ANO DE FABRICACAO: NAO PREENCHIDO\n"); 
    if(r->city_size > 0)
        printf("NOME DA CIDADE: %s\n", r->city);
    else
        printf("NOME DA CIDADE: NAO PREENCHIDO\n");
    if(r->amount != -1)
        printf("QUANTIDADE DE VEICULOS: %d\n", r->amount);
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
