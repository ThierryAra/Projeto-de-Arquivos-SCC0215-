#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"../headers/useful.h"
#include"../headers/header.h"
#include"../headers/record.h" 
#include"../headers/index.h"
#include"../headers/list_stack.h"

int read_item_csv(FILE* csv_file, RECORD* r);
void jump_header(FILE* file, int type_file);

/*  Reads one record from the binary file passed as parameter and 
    stores it in r
    Returns (record_size in bytes) if everything happens normally
           -1 if the record was logically removed
           -2 if there is a read error (no more records)   */
int get_record(FILE* bin_file, RECORD* r, HEADER* header, int type_file);

/*  Removes garbage from each record */
int next_register(FILE* bin_file, int type_file);

/*  Write a record (r) to the .bin file 
    Returns 1 if there is no error
           -1 if the parameters are corrupted */
int write_item(FILE* bin_file, RECORD* r, HEADER* header, 
                int type_file, int record_size, int update);

/*  Sums the variable-size fields to the record size */
int sum_vars(RECORD* r, int initial_sum);

/*  Performs a parameterized search in 'bin_file' */
RECORD* parameterized_search(FILE* bin_file, HEADER* header, char** fields, int n, 
                            int type_file, int* rec_size, int old_next_rrn, long int old_next_BOS);

/*  Read a current record from the FILE input (fields separated by ',' or ' ') */
int read_rec_input(FILE* file, RECORD* r);

/*  Updates the fields of a record from the fields in 'fields_u'. */
void update_record(FILE* bin_file, RECORD* r, char** fields_u, int amt_fields, int* rec_size);

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

    r->removed    = '0';
    r->amount     = -1;
    r->brand_size = -1;
    r->brand      = malloc(sizeof(char)*STR_SIZE);
    r->city_size  = -1;
    r->city       = malloc(sizeof(char)*STR_SIZE);
    r->model_size = -1;
    r->model      = malloc(sizeof(char)*STR_SIZE);
    return r;
}

void free_rec(RECORD* r){
    if(r != NULL){
        free(r->city);
        free(r->brand);
        free(r->model);
        free(r);
    }
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
    while(read_rec_input(csv_file, r) > 0){        
        if(type_file == 1){
            record_size = sum_vars(r, 19);
            write_item(bin_file, r, header, 1, record_size, 0);
        }else if (type_file == 2){
            record_size = sum_vars(r, 22);
            write_item(bin_file, r, header, 2, record_size, 0);
        }
    }

    // Changes these fields: 'nextRRN' or 'nextByteOffset'
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
    update_status(bin_file);
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
    jump_header(bin_file, type_file);

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
        
    jump_header(bin_file, type_file);
    
    //record that will be compared
    RECORD* r = NULL;
    HEADER* header = create_header();
    int rec_size = 0;
    int rec_found = 0;

    while((r = parameterized_search(bin_file, header, fields, n, type_file, &rec_size, -1, -1)) != NULL){
        print_record(r);
        free_rec(r);
        rec_found++;
    }

    free_header(header);
    return rec_found;
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

RECORD* parameterized_search(
    FILE* bin_file, 
    HEADER* header,
    char** fields, 
    int n, int type_file,
    int* rec_size, 
    int old_next_rrn, long int old_next_bos
){
    RECORD* r = create_record();
    //checks whether the record was removed and contains the size of the record read
    int record_size = 0;
    //checks if the record has the same fields/values as the searched record
    int error = 1;

    //fields index
    int i = 0;

    //rrn counter
    int j = 0;
    
    while((record_size = get_record(bin_file, r, header, type_file)) != -2){
       printf("OLD NEXT BOS -> %ld\n", old_next_bos);
       j++;
        if(record_size == -1) // excluded register
            error = -1;
        
        if(old_next_bos != -1 && ftell(bin_file) >= old_next_bos
          || old_next_rrn != -1 && j >= old_next_rrn){

            return NULL;
        }

        //print_record(r);
        //holds as long as the array fields are equal to r
        while (error > 0 && i < n*2){
            if(strcmp(fields[i], "id") == 0){
                int x = atoi(fields[i + 1]);
                if(x != r->id){
                    error = -1;
                }
            }else if(strcmp(fields[i], "ano") == 0){
                int x = atoi(fields[i + 1]);
                if(x != r->year){
                    error = -1;
                }
            }else if(strcmp(fields[i], "qtt") == 0){
                int x = atoi(fields[i + 1]);
                if(x != r->amount){
                    error = -1;
                }
            }else if(strcmp(fields[i], "sigla") == 0){
                if( fields[i+1][0] != r->abbreviation[0] || 
                    fields[i+1][1] != r->abbreviation[1] ){
                    error = -1;
                }
            }else if(strcmp(fields[i], "cidade") == 0){
                if(r->city_size <= 0 || strcmp(fields[i+1], r->city) != 0){
                    error = -1;
                }
            }else if(strcmp(fields[i], "marca") == 0){
                 if(r->brand_size <= 0 || strcmp(fields[i+1], r->brand) != 0){
                    error = -1;
                }
            }else if(strcmp(fields[i], "modelo") == 0){
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
            *rec_size = record_size;
            return r;
        }
        
        //reset error for a next search
        error = 1;
        i = 0;
    }

    free_rec(r);
    return NULL;
}

int read_rec_input(FILE* file, RECORD* r){
    if(file == NULL || r == NULL)
        return -2;
        
    char c;
    
    int rec_size = 27;
    //there is always an id and is != 0
    if(read_int_field(file, &(r->id)) == -1)
        return -1;
    
    if(read_int_field(file, &r->year) == -1)
        r->year = -1;

    if(read_char_field(r->city, file) < 1)   
        r->city_size = 0;
    else{
        r->city_size = strlen(r->city);
        rec_size += r->city_size + 5;
    }
   
    if(read_int_field(file, &r->amount) == -1)
        r->amount = -1;

    if(read_char_field(r->abbreviation, file) < 1)
        strcpy(r->abbreviation, "$$");
    
    if(read_char_field(r->brand, file) < 1)
        r->brand_size = 0;
    else{
        r->brand_size = strlen(r->brand);
        rec_size += r->brand_size + 5;
    }

    if(read_char_field(r->model, file) < 1)
        r->model_size = 0;
    else{
        r->model_size = strlen(r->model);
        rec_size += r->model_size + 5;
    }

    //removes '\n'
    c = fgetc(file);
    if(c != '\n')
        ungetc(c, file);

    return rec_size;
}

int read_insert_data(FILE* file, RECORD* r){
    if(file == NULL || r == NULL)
        return -2;
        
    char c;
    
    int rec_size = 27;
    //there is always an id and is != 0
    if(read_int_field(file, &(r->id)) == -1)
        return -1;
    
    if(read_int_field(file, &r->year) == -1)
        r->year = -1;

    if(read_int_field(file, &r->amount) == -1)
        r->amount = -1;

    char string[5];
    if(read_char_field(string, file) < 1)
        strcpy(r->abbreviation, "$$");
    else
        strncpy(r->abbreviation, string, 2);
    
    if(read_char_field(r->city, file) < 1)   
        r->city_size = 0;
    else{
        r->city_size = strlen(r->city);
        rec_size += r->city_size + 5;
    }
   
    if(read_char_field(r->brand, file) < 1)
        r->brand_size = 0;
    else{
        r->brand_size = strlen(r->brand);
        rec_size += r->brand_size + 5;
    }

    if(read_char_field(r->model, file) < 1)
        r->model_size = 0;
    else{
        r->model_size = strlen(r->model);
        rec_size += r->model_size + 5;
    }

    //removes '\n'
    c = fgetc(file);
    if(c != '\n')
        ungetc(c, file);

    return rec_size;
}

int write_item(
    FILE* bin_file, 
    RECORD* r, HEADER* header, 
    int type_file, 
    int record_size, int update
){
    if(bin_file == NULL || r == NULL)
        return -2;
        
    int bytes_written = 0;
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
        bytes_written += r->city_size + 5;
    }
    if(r->brand_size > 0){
        fwrite(&r->brand_size, 1, sizeof(int), bin_file);
        fwrite(&header->codC6, 1, sizeof(char), bin_file);
        fwrite(r->brand, r->brand_size, sizeof(char), bin_file);
        bytes_written += r->brand_size + 5;
    }
    if(r->model_size > 0){
        fwrite(&r->model_size, 1, sizeof(int), bin_file);
        fwrite(&header->codC7, 1, sizeof(char), bin_file);
        fwrite(r->model, r->model_size, sizeof(char), bin_file);
        bytes_written += r->model_size + 5;
    }

    if(type_file == 1){
        //ensures that each record is 97 bytes long
        bytes_written += 19;
        for(int i = bytes_written; i < STATIC_REC_SIZE; i++)
            fwrite("$", 1, sizeof(char), bin_file);
    }
    else{
        bytes_written += 22;
        //Update the space left with garbage
        if(update == 1){
            for (int i = bytes_written; i < record_size; i++){
                //printf("ESCREVENDO LIXO | ");
                fwrite("$", 1, sizeof(char), bin_file);  
            }
        }
    }

    return 1;
}

/*  Read and add a field of varying size on struct r
    Returns: sum of the fields read               */
int add_str_field(FILE* bin_file, RECORD* r, HEADER* header){
    int string_size = 0;
    fread(&string_size, 1, sizeof(int), bin_file);
    //printf("TAMANHO DE [%d]\n", string_size+5);
    char cod = '3';
    fread(&cod, 1, sizeof(char), bin_file);

    if(cod == header->codC5){
        //city (0)
        r->city_size = string_size;
        fread(r->city, r->city_size, sizeof(char), bin_file);
        r->city[string_size] = '\0';
        //printf("%d %s\n", string_size, r->city);
    }else if(cod == header->codC6){
        //brand (1)
        r->brand_size = string_size;
        fread(r->brand, r->brand_size, sizeof(char), bin_file);
        r->brand[string_size] = '\0';
        //printf("%d %s\n", string_size, r->brand);
    }else if(cod == header->codC7){
        //model (2)
        r->model_size = string_size;
        fread(r->model, r->model_size, sizeof(char), bin_file);
        r->model[string_size] = '\0';
        //printf("%d %s\n", string_size, r->model);
    }

    return string_size+5;
}

int read_fields_t1(FILE* bin_file, HEADER* header, RECORD* r, int* record_size){
    char c = 0;

    for (int i = 0; i < 3; i++){
        //checks if there are more fields
        fread(&c, 1, sizeof(char), bin_file);
        if(c == '$') return (++(*record_size)); //+1 from fread(&c)  
        else ungetc(c, bin_file);
        *record_size += add_str_field(bin_file, r, header);

        if(*record_size == STATIC_REC_SIZE) return STATIC_REC_SIZE;
    }
}

int read_fields_t2(FILE* bin_file, HEADER* header, RECORD* r, int* bytes_scanned, int record_size){
    char c = 0;
    printf("record size [%d]\n", record_size);
    while(*bytes_scanned != record_size){
        printf("STRING COMECA EM %ld [%d] -> ", ftell(bin_file), *bytes_scanned);
        fread(&c, 1, sizeof(char), bin_file);

        if(c == '$'){
            printf("\n\nACHEI LIXO! (%c)\n\n", c);
            printf("FIM %d  [%d] %ld\n\n", *bytes_scanned, record_size, ftell(bin_file));
            return -1;
        }else
            ungetc(c, bin_file);
        
        *bytes_scanned += add_str_field(bin_file, r, header);
    }

    printf("FIM %d  [%d] %ld\n\n", *bytes_scanned, record_size, ftell(bin_file));

    return record_size;
}

int get_record(FILE* bin_file, RECORD* r, HEADER* header, int type_file){
    int bytes_scanned = 0;

    //fixed length fields
    if(fread(&r->removed, 1, sizeof(char), bin_file) == 0)
        return -2; 

     //checks if the record was not logically removed
    if(r->removed == '1'){
        next_register(bin_file, type_file);
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
    
    //removes garbage from registry and from buffer
    if(type_file == 1){
        read_fields_t1(bin_file, header, r, &bytes_scanned);
        fseek(bin_file, STATIC_REC_SIZE - bytes_scanned, SEEK_CUR);
    }else if(type_file == 2){
        if(read_fields_t2(bin_file, header, r, &bytes_scanned, record_size) == -1)
            fseek(bin_file, record_size-bytes_scanned-1, SEEK_CUR);
    }

    
    
    return record_size;
}

int print_record(RECORD* r){
    if(r == NULL)
        return -1;

    printf("ID DO VEICULO: %d\n", r->id);
    printf("SIGLA: %c%c\n", r->abbreviation[0], r->abbreviation[1]);
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

int sum_vars(RECORD* r, int initial_sum){
    if(r->city_size > 0)
        initial_sum += 5 + r->city_size;
    if(r->model_size > 0)
        initial_sum += 5 + r->model_size;
    if(r->brand_size > 0)
        initial_sum += 5 + r->brand_size;

    return initial_sum;
}

void jump_header(FILE* file, int type_file){
    if(type_file == 1) fseek(file, STATIC_REC_HEADER, SEEK_SET);
    else               fseek(file, VARIABLE_REC_HEADER, SEEK_SET);
}
/* --------------------------SECOND PART-------------------------------- */
int next_register(FILE* bin_file, int type_file){
    if(type_file == 1) 
        fseek(bin_file, STATIC_REC_SIZE-1, SEEK_CUR);
    else if(type_file == 2){
        int size_rec = 0;
        fread(&size_rec, 1, sizeof(int), bin_file);
        fseek(bin_file, size_rec, SEEK_CUR);
        return size_rec + 5;           
    }else{
        return -1;
    }    

    return 1;
}

int delete_record(
    FILE* bin_file, int type_file, 
    INDEX* index, int *index_size, 
    STACK* stack, LIST* list,
    int pos, int rrn, long int BOS,
    int rec_size
){
    if(type_file == 1){
        jump_to_record(bin_file, rrn, 0);
        add_stack(stack, rrn);
    }else{
        add_sorted_to_list(list, BOS, rec_size);
    }
    
    //updates the index vector
    (*index_size)--;
    update_id_index(index, pos, type_file, 1, *index_size, -1, -1, -1);
    sort_id_index(index, *index_size);
}   

int delete_where(FILE* bin_file, char* name_index, int n, int type_file){
    FILE* index_file = fopen(name_index, "r+b");
    if(bin_file == NULL || index_file == NULL)
        return -2;
    if(!check_status(bin_file) && !check_status(index_file))
        return -2;
    
    //Updates file status as inconsistent to make changes
    update_status(bin_file);
    update_status(index_file);

    //Aux Variables
    RECORD* r = NULL;
    HEADER* h = create_header();

    //Linked list and Stack that will help the exclusion structures
    STACK* stack = NULL;
    LIST*  list  = NULL;

    //Allocates space for only one of the structures
    if(type_file == 1){
        stack = create_stack(500);
        read_stack_top(bin_file, stack);
    }else{
        list = create_list(500);
        read_list(bin_file, list);
    }

    //number of records removed
    int removed_amount = 0;
    
    //Vector that will store the id/(rrn/BOS) of the index file (index_file)
    int index_size = 0;
    INDEX* index = read_index_file(index_file, &index_size, type_file);
     
    //number of fields to be searched
    int amt_fields = 0;
    //variable that stores the position where there is a search for id in 'fileds
    int is_there_id = -1;

    //search for removal n times
    for(int i = 0; i < n; i++){
        scanf("%d", &amt_fields);

        char** fields = read_search_fields(amt_fields, &is_there_id);  
        
        int pos = 0; //position of the fetched record in the index array
        int rrn = 0;      
        long int BOS = 0;
        int rec_size = -1;

        //if there is a search by ID the record is quickly retrieved, if it exists
        if(is_there_id != -1){
            //fetches the record in the index vector and assigns its location
            //the rrn or BOS
            pos = recover_rrn(index, atoi(fields[is_there_id+1]), 
                                   index_size, type_file, &rrn, &BOS);
            
            //if it exists, the other search fields are compared with the record
            if(pos != -1){
                jump_to_record(bin_file, rrn, BOS);
                r = parameterized_search(bin_file, h, fields, amt_fields, type_file, &rec_size, -1, -1);
                
                //if the record contains the searched fields, it will be deleted
                if(r != NULL){
                    //print_record(r);
                    delete_record(bin_file, type_file, index, &index_size,
                                 stack, list, pos, rrn, BOS, rec_size);
		            removed_amount++;
		            free_rec(r); 
                }              
            }                 
        //sequential search
        }else{
            char c = 0;
            jump_header(bin_file, type_file);

            //as long as there is a record, a parametrized search will be performed
            //for all records
            while(fread(&c, 1, sizeof(char), bin_file) != 0){
                ungetc(c, bin_file);
                r = parameterized_search(bin_file, h, fields, amt_fields, type_file, &rec_size, -1, -1);
                if(r != NULL){
                    pos = recover_rrn(index, r->id, index_size, 
                                           type_file, &rrn, &BOS);
                    if(pos > 0){
                        //print_record(r);
                        delete_record(bin_file, type_file, index, &index_size,
                                     stack, list, pos, rrn, BOS, rec_size);
                        removed_amount++;
                    }   
                    
                    free_rec(r);
                }
            }
        }
        
        free_array_fields(fields, amt_fields);
        is_there_id = -1;
    }

    //transfers the structure of the stack/linked list to the file
    if(removed_amount != 0){
        if(type_file == 1)
            write_stack(bin_file, stack); 
        else
            write_list(bin_file, list);

        //Reopen the index file to be rewritten and delete the previous contents
        fclose(index_file);
        index_file = fopen(name_index, "w+b");
        write_index(index_file, index, index_size, type_file);
        
        update_status(index_file); 
        fclose(index_file);
    
        //updates the number of removed records
        att_numRecRem(bin_file, 1, type_file, removed_amount);  
    }
     
    
    update_status(bin_file);
    
    free_index_array(index);
    free_stack(stack);
    free_list(list);
    free_rec(r);
    free_header(h);
    return 1;
}

void jump_to_record(FILE* file, int rrn, long int BOS){
    if(rrn != -1)
        fseek(file, (rrn*STATIC_REC_SIZE)+STATIC_REC_HEADER, SEEK_SET);
    else if(BOS !=  -1)
        fseek(file, BOS, SEEK_SET);
}

void insert_record(
    FILE* bin_file, HEADER* h, 
    STACK* stack, LIST* list, 
    int type_file, int* next_RRN,
    long int* next_BOS, 
    int rec_size, int old_rec_size,
    RECORD* r, INDEX* index, int* index_size
){
    int rrn = -1;
    long int BOS = -1;

    long int list_top = -1;
    int list_top_size = -1;

    if(type_file == 1){
        h->top_rrn = return_stack_top(stack);
        if(h->top_rrn != -1){
            jump_to_record(bin_file, h->top_rrn, -1);
            rrn = (ftell(bin_file)-STATIC_REC_HEADER)/STATIC_REC_SIZE;
            
            remove_from_stack(stack);
            h->numRegRem--;
        }
        else{
            rrn = *next_RRN;
            jump_to_record(bin_file, (*next_RRN)++, -1);
        }

        insert_index(index, *index_size, r->id, rrn, -1);
        write_item(bin_file, r, h, type_file, list_top_size, 1);
    }else{
        list_top = return_list_top(list, &list_top_size);
        //printf("LIST TOP %ld %d [%d]\n", list_top, list_top_size, rec_size);
        if(list_top != -1 && rec_size <= list_top_size){
            jump_to_record(bin_file, -1, list_top);
            BOS = list_top;
            write_item(bin_file, r, h, type_file, list_top_size, 1);
            
            remove_from_list(list);
            h->numRegRem--;
        }
        else{
            BOS = *next_BOS;
            jump_to_record(bin_file, -1, *next_BOS);
            write_item(bin_file, r, h, type_file, old_rec_size, 0);
            
            //(*next_BOS) += old_rec_size+5;
            (*next_BOS) = ftell(bin_file);
        }

        insert_index(index, *index_size, r->id, -1, BOS);
    }

    //printf("INSERI NA POSICAO %ld\n", ftell(bin_file));
    
    (*index_size)++;
    sort_id_index(index, *index_size);
}

int insert_into(FILE* bin_file, char* name_index, int n, int type_file){
    FILE* index_file = fopen(name_index, "r+b");
    if(bin_file == NULL || index_file == NULL)
        return -2;
    if(!check_status(bin_file) && !check_status(index_file))
        return -2;
    
    //fseek(index_file, 0, SEEK_SET);
    //Updates file staus as inconsistent to make changes
    update_status(bin_file);
    update_status(index_file);

    int index_size = 0;
    INDEX* index = read_index_file(index_file, &index_size, type_file);
    
    //Linked list and Stack that will help the exclusion structures
    STACK* stack = NULL;
    LIST*  list  = NULL;

    //Allocates space for only one of the structures
    if(type_file == 1){
        stack = create_stack(500);
        read_stack(bin_file, stack);
    }else{
        list = create_list(500);
        read_list(bin_file, list);
    }

    RECORD* r = create_record();
    HEADER* h = create_header();
    
    int rec_size = 0, next_RRN = 0;
    long int next_BOS = 0;
    
    if(type_file == 1){
        fseek(bin_file, 174, SEEK_SET);
        fread(&next_RRN, 1, sizeof(int), bin_file);
    }else{
        fseek(bin_file, 178, SEEK_SET);
        fread(&next_BOS, 1, sizeof(long int), bin_file);
    }
    fread(&h->numRegRem, 1, sizeof(int), bin_file);
    

    int list_top_size = 0;
    int rrn = -1, stack_top = 0;
    long int BOS = -1, list_top = 0;

    char c;
    for(int i = 0; i < n; i++){
        rec_size = read_insert_data(stdin, r);

        insert_record(bin_file, h, stack, list, type_file, &next_RRN, 
                     &next_BOS, rec_size, rec_size, r, index, &index_size);
    }

    //rewriting the headline
    if(type_file == 1) h->top_rrn = return_stack_top(stack);
    else               h->top_BOS = return_list_top(list, &rec_size);
    update_header(bin_file, h, type_file, next_RRN, next_BOS);

    //rewriting the index file
    fclose(index_file);
    index_file = fopen(name_index, "w+b");
    write_index(index_file, index, index_size, type_file);
    update_status(index_file);

    free_rec(r);
    free_header(h);
    free_index_array(index);
    fclose(index_file);
    free_stack(stack);
    free_list(list);
    return 1;
}

int update_where(FILE* bin_file, char* name_index, int n, int type_file){
    FILE* index_file = fopen(name_index, "r+b");
    fseek(index_file, 0, SEEK_SET);
    if(bin_file == NULL || index_file == NULL)
        return -2;
    if(!check_status(bin_file) && !check_status(index_file))
        return -2;
    
    //Updates file staus as inconsistent to make changes
    update_status(bin_file);
    update_status(index_file);

    int index_size = 0;
    INDEX* index = read_index_file(index_file, &index_size, type_file);

    //Linked list and Stack that will help the exclusion structures
    STACK* stack = NULL;
    LIST*  list  = NULL;

    //Allocates space for only one of the structures
    if(type_file == 1){
        stack = create_stack(500);
        read_stack(bin_file, stack);
    }else{
        list = create_list(500);
        read_list(bin_file, list);
    }

    RECORD* r = NULL;
    HEADER* h = create_header();
    
    int rec_size = 0;

    int next_RRN = -1;
    long int next_BOS = -1;
    
    if(type_file == 1){
        fseek(bin_file, 174, SEEK_SET);
        fread(&next_RRN, 1, sizeof(int), bin_file);
    }else{
        fseek(bin_file, 178, SEEK_SET);
        fread(&next_BOS, 1, sizeof(long int), bin_file);
    }
    fread(&h->numRegRem, 1, sizeof(int), bin_file);
    
    int amt_fields_s = 0, amt_fields_u = 0;
    int is_there_id_u = -1;
    int is_there_id_s = -1;
    //print_index_table(index, index_size, type_file);
    

    //search for removal n times
    for(int i = 0; i < n; i++){
        long int old_next_BOS = next_BOS;
        int old_next_rrn = next_RRN;

        scanf("%d", &amt_fields_s);
        char** fields_s = read_search_fields(amt_fields_s, &is_there_id_s);  
        for(int j = 0; j < amt_fields_s*2; j++){
            printf("%s -> ", fields_s[j]);
            j++;
            printf("%s | ", fields_s[j]);
        }
        printf("\n");
        scanf("%d", &amt_fields_u);
        char** fields_u = read_search_fields(amt_fields_u, &is_there_id_u);  
       /*  for(int j = 0; j < amt_fields_u*2; j++){
            printf("%s -> ", fields_u[j]);
            j++;
            printf("%s | ", fields_u[j]);
        }
        printf("\n"); */
        int pos = 0; //position of the fetched record in the index array
        int rrn = 0;      
        long int BOS = 0;
        
        int stack_top = -1;
        long int list_top = -1;
        int size_top = 0;

        int rec_size = -1;
        int old_rec_size = -1;
        
        if(is_there_id_s != -1){
            // Searches the record in the index array and returns 
            //its position in the file (rrn/BOS) and in the array
            pos = recover_rrn(index, atoi(fields_s[is_there_id_s+1]), 
                             index_size, type_file, &rrn, &BOS);

            //Compara os campos, diferentes do ID, da busca
            if(pos != -1){
                jump_to_record(bin_file, rrn, BOS);
                r = parameterized_search(bin_file, h, fields_u, amt_fields_u, type_file,
                                         &old_rec_size, next_RRN, next_BOS);
            }   

            if(r != NULL){
                //updates the fields
                rec_size = old_rec_size;
                //printf("OLD RECORD: \n");
                //print_record(r);
                
                update_record(bin_file, r, fields_u, amt_fields_u, &rec_size);
                
                //printf("\nNEW RECORD: \n");
                //print_record(r);

                if(type_file == 1){
                    jump_to_record(bin_file, rrn, BOS);
                    write_item(bin_file, r, h, type_file, rec_size, 1);
                }else{
                    //Checks whether the record can be overwritten
                    if(rec_size > old_rec_size){
                        //Deletes the record and adds it in another position
                        delete_record(bin_file, type_file, index, &index_size,
                                     stack, list, pos, rrn, BOS, old_rec_size);
                        //Searches for the ideal place to insert the updated record
                        insert_record(bin_file, h, stack, list, type_file, &next_RRN,
                            &next_BOS, rec_size, old_rec_size, r, index, &index_size);
                    } else{
                        jump_to_record(bin_file, rrn, BOS);
                        write_item(bin_file, r, h, type_file, old_rec_size, 1);
                    }
                }

                //Checks if the ID should be changed and populates the index vector
                if(is_there_id_u != -1)
                    update_id_index(index, pos, type_file, 3, index_size,
                                    rrn, BOS, atoi(fields_u[is_there_id_u+1]));
                else
                    update_id_index(index, pos, type_file, 3, index_size, rrn, BOS, -1);
                  
                sort_id_index(index, index_size);
                free_rec(r);
                r = NULL;
            }
        //Sequential search
        }else{
            char c = 0;
            jump_header(bin_file, type_file);
            long int pos_atual = 0;
            //as long as there is a record, a parametrized search will be performed
            //for all records
            //while(fread(&c, 1, sizeof(char), bin_file) != 0){
            while((r = parameterized_search(bin_file, h, fields_s, amt_fields_s, 
                       type_file, &old_rec_size, old_next_rrn, old_next_BOS)) != NULL){
                //(r = parameterized_search(bin_file, h, fields_s, amt_fields_s, 
                //                            type_file, &old_rec_size));
        
                //posicao para continuar a busca, apois atualizado do registro
                pos_atual = ftell(bin_file);
                
                rec_size = old_rec_size;
                
                //updates the fields
                update_record(bin_file, r, fields_u, amt_fields_u, &rec_size);
                
                pos = recover_rrn(index, r->id, index_size, type_file, &rrn, &BOS);
                
                if(pos != -1){
                    if(type_file == 1){
                        jump_to_record(bin_file, rrn, BOS);
                        write_item(bin_file, r, h, type_file, rec_size, 1);
                    }else{
                        //Checks whether the record can be overwritten
                        if(rec_size > old_rec_size){
                            //Deletes the record and adds it in another position
                            delete_record(bin_file, type_file, index, &index_size,
                                        stack, list, pos, rrn, BOS, old_rec_size);

                            //Searches for the ideal place to insert the updated record
                            insert_record(bin_file, h, stack, list, type_file, &next_RRN,
                                &next_BOS, rec_size, old_rec_size, r, index, &index_size);
                        }else{
                            jump_to_record(bin_file, rrn, BOS);
                            write_item(bin_file, r, h, type_file, old_rec_size, 1);
                        }
                    }
                }

                //Checks if the ID should be changed and populates the index vector
                if(is_there_id_u != -1)
                    update_id_index(index, pos, type_file, 3, index_size,
                                    rrn, BOS, atoi(fields_u[is_there_id_u+1]));
                else
                    update_id_index(index, pos, type_file, 3, index_size, rrn, BOS, -1);
                
                sort_id_index(index, index_size);
                
                free_rec(r);
                r = NULL;
                
                fseek(bin_file, pos_atual, SEEK_SET);
                
                
            }
        }

        is_there_id_s = -1;
        is_there_id_u = -1;
        free_array_fields(fields_s, amt_fields_s);
        free_array_fields(fields_u, amt_fields_u);
    }
    
    if(type_file == 1) h->top_rrn = return_stack_top(stack);
    else               h->top_BOS = return_list_top(list, &rec_size);
    update_header(bin_file, h, type_file, next_RRN, next_BOS);

    fclose(index_file);
    index_file = fopen(name_index, "wb");
    write_index(index_file, index, index_size, type_file);
    update_status(index_file);

    free_rec(r);
    free_header(h);
    free_stack(stack);
    free_list(list);

    free_index_array(index);
    fclose(index_file);
    return 1;
}

int add_valid_field(char** fields, int i, char* string, int *rec_size, int str_size, int type);

void update_record(FILE* bin_file, RECORD* r, char** fields_u, int amt_fields, int* rec_size){
    int size_str = 0;

    for (int i = 0; i < amt_fields*2; i += 2){
        if(strcmp(fields_u[i], "id") == 0)
            r->id = add_valid_field(fields_u, i, NULL, rec_size, -1, 1);

        else if(strcmp(fields_u[i], "ano") == 0)
            r->year = add_valid_field(fields_u, i, NULL, rec_size, -1, 1);

        else if(strcmp(fields_u[i], "qtt") == 0)
            r->amount = add_valid_field(fields_u, i, NULL, rec_size, -1, 1);

        else if(strcmp(fields_u[i], "sigla") == 0){
            if(strcmp(fields_u[i+1],"NULO") != 0)
                strcpy(r->abbreviation, fields_u[i+1]);
            else
                strcpy(r->abbreviation, "$$");
        }
        else if(strcmp(fields_u[i], "cidade") == 0)
            r->city_size = add_valid_field(fields_u, i, r->city, rec_size, r->city_size, 2);

        else if(strcmp(fields_u[i], "marca") == 0)
            r->brand_size = add_valid_field(fields_u, i, r->brand, rec_size, r->brand_size, 2);

        else if(strcmp(fields_u[i], "modelo") == 0)
            r->model_size = add_valid_field(fields_u, i, r->model, rec_size, r->model_size, 2);

    }
}

/*  Verifica a validade dos campos que deverao ser atualizados */
int add_valid_field(char** fields, int i, char* string, int *rec_size, int str_size, int type){  
    if(type == 1){
        if(strcmp(fields[i+1],"NULO") != 0)
            return atoi(fields[i+1]);
        else
            return -1;
    }else{
        int size = strlen(fields[i+1]);
        if(strcmp(fields[i+1],"NULO") != 0){
            strncpy(string, fields[i+1], size);
            string[size] = '\0';
            //printf("MUDEI PRA %s\n", string);
            if(str_size > 0)
                *rec_size += - str_size + size;
            else //CODstr + str_size + string
                *rec_size += 1 + 4 + size; 
            
            return size;
        }
        else{
            *rec_size += - 1 - 4 - str_size; 
            return 0;
        }

    }
}