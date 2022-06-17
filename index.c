#include<stdio.h>
#include<stdlib.h>
#include"header.h"
#include"record.h"
#include"useful.h"
#include"index.h"

INDEX* read_data_file(FILE* bin_file, int* id_indexes_size, int type_file);

struct index{
    int id;
    int rrn;
    long int BOS;
};

int qsort_compare(const void* a, const  void* b){
    return ((INDEX*)a)->id - ((INDEX*)b)->id;
}

int create_index_id(FILE* bin_file, FILE* index_file, int type_file){
    if(bin_file == NULL || index_file == NULL)
        return -2;

    int indexes_size = 0;
    INDEX* indexes = read_data_file(bin_file, &indexes_size, type_file);
    if(indexes == NULL)
        return -2;

    //write status (header) of the index file
    char c = '0';
    fwrite(&c, 1, sizeof(char), index_file);
    int i = 0;
    int id_a = 0;
    while(i <= indexes_size){
        fwrite(&indexes[i].id, 1, sizeof(int), index_file);
        if(type_file == 1)
            fwrite(&indexes[i].rrn, 1, sizeof(int), index_file);
        else
            fwrite(&indexes[i].BOS, 1, sizeof(long int), index_file);
            
        id_a = indexes[i].id;
        i++;
    }
    
    update_status(index_file);
    free_index_array(indexes);
    return 1;
}

INDEX* read_data_file(FILE* bin_file, int* id_indexes_size, int type_file){
    if(bin_file == NULL)
        return NULL;

    int status = check_status(bin_file);
    if(status == 0)
        return NULL;
    
    INDEX* id_indexes = NULL;
    
    char removed;
    int next_i;          //campo que deve ser ignorado
    int id = 0, rrn = 0; //valores que serão salvos no vetor de indices
    int i = 0;           //index do vetor
    long int next_li;    //campo que deve ser ignorado
    
    if(type_file == 1){     
        //Busca o maior RRN para ter o tamanho da matriz de indices
        fseek(bin_file, 174, SEEK_SET);
        fread(id_indexes_size, 1, sizeof(int), bin_file);
        fseek(bin_file, 4, SEEK_CUR); // jump nroRegRem
        //Alocando espaco para a matriz 
        id_indexes = malloc(sizeof(INDEX)*(*id_indexes_size));

        while(fread(&removed, 1, sizeof(char), bin_file) != 0){
            if(removed == '0'){
                fread(&next_i, 1, sizeof(int), bin_file);
                fread(&id, 1, sizeof(int), bin_file);

                id_indexes[i].id  = id;
                id_indexes[i].rrn = rrn;
                fseek(bin_file, STATIC_REC_SIZE - 9, SEEK_CUR);                             
                i++;
            }else
                next_register(bin_file, type_file);
        
            rrn++;   
        }
    }else if(type_file == 2){
        long int final_BOS = 0;
        
        //??????????
        fseek(bin_file, 178, SEEK_SET);
        fread(&final_BOS, 1, sizeof(long int), bin_file);
        fseek(bin_file, 4, SEEK_CUR); // jump nroRegRem

        //Alocando espaco para o vetor (tamanho inicial medio)
        int size = 1100;
        id_indexes = malloc(sizeof(INDEX)*(size));

        long int BOS = VARIABLE_REC_HEADER;
        int record_size = 0;

        while(fread(&removed, 1, sizeof(char), bin_file) != 0){
            if(removed == '0'){
                fread(&record_size, 1, sizeof(int), bin_file);
                fread(&next_li, 1, sizeof(long int), bin_file);
                fread(&id, 1, sizeof(int), bin_file);

                id_indexes[i].id  = id;
                id_indexes[i].BOS = BOS;

                //ByteOffset = record_size + 5 (removed(1Byte) + size(4Bytes))
                BOS += record_size + 5;
                fseek(bin_file, record_size - 12, SEEK_CUR);                             
                i++;
            }else
                BOS += next_register(bin_file, type_file);    
        
            if(i >= size){
                size = size + size/5;
                id_indexes = realloc(id_indexes, sizeof(INDEX)*size);
            }

        }

        *id_indexes_size = i - 1;
        if(*id_indexes_size != size)
            id_indexes = realloc(id_indexes, sizeof(INDEX)*i);
    }

    qsort(id_indexes, *id_indexes_size, sizeof(INDEX), qsort_compare);
    return id_indexes;
}

int print_index_file(FILE* index_file, int type_file){
    if(index_file == NULL)
        return -1;

    char status = '0';
    int id = 0;
    int rrn = 0; long int BOS = 0, idli = 0;

    fseek(index_file, 0, SEEK_SET);
    if(check_status(index_file) == 0)
        return -1;

    if(type_file == 1){
        while(fread(&id, 1, sizeof(int), index_file) != 0){
            fread(&rrn, 1, sizeof(int), index_file);
            printf("%d %d |", id, rrn);
        }
    }else{
        while(fread(&id, 1, sizeof(int), index_file) != 0){
            fread(&BOS, 1, sizeof(long int), index_file);
            printf("%d %ld |", id, BOS);
        }
    }
    
    fclose(index_file);
}

void* read_index_file(
    FILE* bin_file, FILE* index_file, 
    int* id_indexes_size,     
    int type_file
){
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

        int i = 0;
        while(fread(&id, 1, sizeof(int), index_file) != 0){
            fread(&rrn, 1, sizeof(int), index_file);
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
        int size = 2000;
        long int* id_indexes = malloc(sizeof(long int)*(size));

        long int BOS = 0;
        int i = 0;

        while(fread(&id, 1, sizeof(int), index_file) != 0){
            fread(&BOS, 1, sizeof(int), index_file);
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

int print_index_table(INDEX* id_indexes, int id_indexes_size, int type_file){
    if(id_indexes == NULL)
        return -1;

    for(int i = 0; i < id_indexes_size*2; i += 2){
        if(type_file == 1)
            printf("[%d] %d  |  ", id_indexes[i].id, id_indexes[i+1].rrn);
        if(type_file == 2)
            printf("[%d] %ld  |  ", id_indexes[i].id, id_indexes[i+1].BOS);
    }
        
    return 1;
}

int free_index_array(INDEX* id_indexes){
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
                id_indexes[mid]      = -1;
                rrn = id_indexes[mid + 1];
                id_indexes[mid + 1]  = -1;
                return rrn;
            }

            return id_indexes[mid + 1];
        }
        
        
        if(id > id_indexes[mid])
            begin = mid+2;
        else
            end = mid - 2; //verificar
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