#include<stdio.h>
#include<stdlib.h>
#include"header.h"
#include"record.h"
#include"useful.h"
#include"index.h"

int create_index_id(FILE* bin_file, FILE* index_file, int type_file){
    if(bin_file == NULL || index_file == NULL)
        return -2;

    //checks if the file is already available for editing
    int status = check_status(bin_file);
    if(status == -1 || status == 0)
        return -1;

    //write status (header) of the index file
    fwrite("0", 1, sizeof(char), index_file);

    //byteoffset para os registros de tamanho variavel
    long int BOS = 0;

    //remove bin file header
    if(type_file == 1) 
        fseek(bin_file, STATIC_REC_HEADER, SEEK_SET);
    else{
        BOS = VARIABLE_REC_HEADER;
        fseek(bin_file, VARIABLE_REC_HEADER, SEEK_SET); 
    }
    
    //read bin_file and write data to index_file
    char c = 0;
    int id = 0, rrn = 0;
    int trash; long int trash_;

    while(fread(&c, 1, sizeof(char), bin_file) != 0){
        if(c == '0'){
            //remove the field 'next' (type file = 1)
            //or record size          (type_file = 2)
            fread(&trash, 1, sizeof(int), bin_file);

            if(type_file == 1) {
                fread(&id, 1, sizeof(int), bin_file);

                fwrite(&id, 1, sizeof(int), index_file);
                fwrite(&rrn, 1, sizeof(int), index_file);
                fseek(bin_file, STATIC_REC_SIZE - 9, SEEK_CUR);
            }else if (type_file == 2){
                fread(&trash_, 1, sizeof(long int), bin_file); //remove the field 'next'
                //fseek(bin_file, sizeof(long int), SEEK_CUR);

                fread(&id, 1, sizeof(int), bin_file);
                
                fwrite(&id, 1, sizeof(int), index_file);
                fwrite(&BOS, 1, sizeof(long int), index_file);
                
                //ByteOffset = record_size + 5 (removed(1Byte) + size(4Bytes))
                BOS += trash + 5;
                fseek(bin_file, trash - 12, SEEK_CUR);
            }                        
        }else{
            next_register(bin_file, type_file);
        }


        rrn++;
    }
    
    update_status(index_file);
    return 1;
}

int print_index_file(FILE* index_file, int type_file){
    if(index_file == NULL)
        return -1;

    char status = '0';
    int id = 0;
    int rrn = 0; long int BOS = 0;

    fseek(index_file, 0, SEEK_SET);
    if(check_status(index_file) == 0)
        return -1;

    while(fread(&id, 1, sizeof(int), index_file) != 0){
        if(type_file == 1) fread(&rrn, 1, sizeof(int), index_file);
        else               fread(&BOS, 1, sizeof(long int), index_file);
    }
    
    fclose(index_file);
}

void* create_index_vec(FILE* bin_file, FILE* index_file, int* id_indexes_size, int type_file){
    if(bin_file == NULL || index_file == NULL)
        return NULL;

    int status = check_status(index_file);
    if(status == 0)
        return NULL;

    int id = 0;
    
    if(type_file == 1){     
        //Busca o maior RRN para ter o tamanho da matriz de indices
        fseek(bin_file, 174, SEEK_SET);
        fread(id_indexes_size, 1, sizeof(int), bin_file);
        //Alocando espaco para a matriz 
        int* id_indexes = malloc(sizeof(int)*(*id_indexes_size*2));

        char status = '0';
        int rrn = 0;

        //printf("STATUS: %c\n", status);
        int i = 0;
        while(fread(&id, 1, sizeof(int), index_file) != 0){
            fread(&rrn, 1, sizeof(int), index_file);
            //printf("%d %d\n", id, rrn);
            id_indexes[i] = id;
            id_indexes[i+1] = rrn;
            i += 2;
        }

        return id_indexes;
    }else if(type_file == 2){
        long int final_BOS = 0;
        
        fseek(bin_file, 178, SEEK_SET);
        fread(&final_BOS, 1, sizeof(long int), bin_file);
        //Alocando espaco para a matriz (tamanho inicial medio)
        int size = 1000;
        long int* id_indexes = malloc(sizeof(long int)*(size*2));

        long int BOS = 0;
        int i = 0;

        //printf("STATUS: %c\n", status);
        while(fread(&id, 1, sizeof(int), index_file) != 0){
            fread(&BOS, 1, sizeof(int), index_file);
            //printf("%d %ld\n", id, BOS);
            id_indexes[i] = id;
            id_indexes[i+1] = BOS;
            i += 2;

            if(i == size){
                //como os arquivos nao constumam ter + de 1000 registros
                //o aumento ocorre gradativamente
                size += 1000 + size/5;
                id_indexes = realloc(id_indexes, sizeof(long int)*size*2);
            }
        }

        *id_indexes_size = i;
        return id_indexes;
    }

    return NULL;
}

int print_index_table(void* id_indexes, int id_indexes_size, int type_file){
    if(id_indexes == NULL)
        return -1;

    if(type_file == 2)
        id_indexes = (long int*)id_indexes;
    else
        id_indexes = (int*)id_indexes;

    for(int i = 0; i < id_indexes_size*2; i += 2){
        //if(type_file == 1)printf("[%d] %d  |  ", id_indexes[i], id_indexes[i+1]);
        //if(type_file == 2)printf("[%d] %ld  |  ", id_indexes[i], id_indexes[i+1]);

    }
        
    return 1;
}

int free_index_table(int* id_indexes){
    if(id_indexes == NULL)
        return -2;
    
    free(id_indexes);
    return 1;
}

int recover_rrn(int* id_indexes, int id, int id_indexes_size, int mode){
    //valor do rrn caso deva ser removido do vetor e retornado (mode == 1)
    //inicialmente eh um valor nulo
    int rrn = -1;

    //BUSCA BINARIA SIMPLES
    int begin = 0;
    //posicao do ultimo ID no vetor
    int end   = id_indexes_size*2 - 2;

    while(begin < end){
        int mid = (begin + end)/2; 
        
        //garante que sempre estara posicionado em um ID
        if(mid % 2 != 0)
            mid -= 1;

        if(id_indexes[mid] == id){
            if(mode == 1){ //excluindo
                id_indexes[mid]     = -1;
                rrn = id_indexes[mid + 1];
                id_indexes[mid + 1] = -1;
                return rrn;
            }

            return id_indexes[mid + 1];
        }
        
        
        if(id > id_indexes[mid])
            begin = mid+2;
        else
            end = mid;
    }

    if(id_indexes[begin] == id){
        if(mode == 1){ //busca para remocao
            id_indexes[begin] = -1;
            rrn = id_indexes[begin + 1];
            id_indexes[begin + 1] = -1;
        }else
            return id_indexes[begin+1];
    }

    return rrn;
}