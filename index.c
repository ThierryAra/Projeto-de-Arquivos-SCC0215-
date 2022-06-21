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

void write_index(FILE* index_file, INDEX* index, int index_size, int type_file){
    //write status (header) of the index file
    char c = '0';
    fwrite(&c, 1, sizeof(char), index_file);
    
    //control
    int i = 0;
    
    //Percorre todas as posicoes do vetor de indices
    while(i < index_size){
        fwrite(&index[i].id, 1, sizeof(int), index_file);

        if(type_file == 1)
            fwrite(&index[i].rrn, 1, sizeof(int), index_file);
        else
            fwrite(&index[i].BOS, 1, sizeof(long int), index_file);
        
        i++;
    }
}

int create_index_id(FILE* bin_file, FILE* index_file, int type_file){
    if(bin_file == NULL || index_file == NULL)
        return -2;

    int index_size = 0;
    INDEX* index = read_data_file(bin_file, &index_size, type_file);
    if(index == NULL)
        return -1;
    
    write_index(index_file, index, index_size, type_file);

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

    sort_id_indexes(id_indexes, *id_indexes_size);
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
}

INDEX* read_index_file(FILE* index_file, int* id_indexes_size, int type_file){
    if(index_file == NULL)
        return NULL;

    int id = 0, size = 1100, i = 0;
    INDEX* id_indexes = malloc(sizeof(INDEX)*size);
    
    if(type_file == 1){     
        int rrn = 0;

        while(fread(&id, 1, sizeof(int), index_file) != 0){
            fread(&rrn, 1, sizeof(int), index_file);
            id_indexes[i].id  = id;
            id_indexes[i].rrn = rrn;
            i++;

            if(i >= size){
                //como os arquivos nao constumam ter + de 1000 registros
                //o aumento ocorre gradativamente
                size += size + size/5;
                id_indexes = realloc(id_indexes, sizeof(INDEX)*size);
            }
        }
    }else if(type_file == 2){
        id_indexes = malloc(sizeof(INDEX)*(size));

        long int BOS = 0;

        while(fread(&id, 1, sizeof(int), index_file) != 0){
            fread(&BOS, 1, sizeof(int), index_file);
            id_indexes[i].id  = id;
            id_indexes[i].BOS = BOS;
            i++;

            if(i >= size){
                size += size + size/5;
                id_indexes = realloc(id_indexes, sizeof(INDEX)*size);
            }
        }
    }

    *id_indexes_size = i;
    if(*id_indexes_size != size) id_indexes = realloc(id_indexes, sizeof(INDEX)*i);
    return id_indexes;
}

int print_index_table(INDEX* id_indexes, int id_indexes_size, int type_file){
    if(id_indexes == NULL)
        return -1;

    for(int i = 0; i < id_indexes_size; i += 1){
        if(type_file == 1)
            printf("[%d] %d  |  ", id_indexes[i].id, id_indexes[i].rrn);
        if(type_file == 2)
            printf("[%d] %ld  |  ", id_indexes[i].id, id_indexes[i].BOS);
    }
    
    printf("idindexsize %d\n", id_indexes_size);
    return 1;
}

int free_index_array(INDEX* id_indexes){
    if(id_indexes == NULL)
        return -2;
    
    free(id_indexes);
    return 1;
}

void update_id_index(
    INDEX* id_indexes, int mid, 
    int type_file, int mode, int end
){
    if(mode == 1){ //excluindo
        id_indexes[mid].id = -1;
        
        if(type_file == 1)
            id_indexes[mid].rrn  = -1;
        else
            id_indexes[mid].BOS = -1;

        //swap
        INDEX aux = id_indexes[mid];
        id_indexes[mid] = id_indexes[end];
        id_indexes[end] = aux;
    }
}

int recover_rrn(
    INDEX* id_indexes, 
    int id, int id_indexes_size, 
    int type_file, int* rrn, long int* BOS
){  
    //BUSCA BINARIA SIMPLES
    int begin = 0;
    int end   = (id_indexes_size)-1;
    //printf("\n\nFIM %d id end %d\n\n",end,id_indexes[end].id);
    //printf("BUSCANDO %d -> ", id);
    int mid = (begin + end)/2; 
    while(begin <= end){
        //printf("[%d] %d [%d] | ", id_indexes[begin].id, id_indexes[mid].id, id_indexes[end].id);
        if(id_indexes[mid].id == id){
            //printf("\nACHEI O %d\n", id_indexes[mid].id);
            if(type_file == 1)
                *rrn = id_indexes[mid].rrn;
            else
                *BOS = id_indexes[mid].BOS;
            return mid;   
        }
                
        if(id > id_indexes[mid].id)
            begin = mid + 1;
        else
            end = mid - 1;        

        mid = (begin + end)/2;   
    }
    //printf("\n");
    return -1;
}

int qsort_compare(const void* a, const  void* b){
    return ((INDEX*)a)->id - ((INDEX*)b)->id;
}

int qsort_compare_inverse(const void* a, const  void* b){
    return -(((INDEX*)a)->id - ((INDEX*)b)->id);
}

void sort_id_indexes(INDEX* array, int array_size){
    qsort(array, array_size, sizeof(INDEX), qsort_compare);
}