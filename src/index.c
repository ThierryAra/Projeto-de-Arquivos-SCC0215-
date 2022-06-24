#include<stdio.h>
#include<stdlib.h>
#include"../headers/header.h"
#include"../headers/record.h"
#include"../headers/useful.h"
#include"../headers/index.h"

INDEX* read_data_file(FILE* bin_file, int* id_index_size, int type_file);

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

    update_status(index_file);
    return 1;
}

INDEX* read_data_file(FILE* bin_file, int* id_index_size, int type_file){
    if(bin_file == NULL)
        return NULL;

    int status = check_status(bin_file);
    if(status == 0)
        return NULL;
    
    INDEX* id_index = NULL;
    
    char removed;
    int next_i;          //campo que deve ser ignorado
    int id = 0, rrn = 0; //valores que serão salvos no vetor de indices
    int i = 0;           //index do vetor
    long int next_li;    //campo que deve ser ignorado
    
    if(type_file == 1){     
        //Busca o maior RRN para ter o tamanho da matriz de indices
        fseek(bin_file, 174, SEEK_SET);
        fread(id_index_size, 1, sizeof(int), bin_file);
        fseek(bin_file, 4, SEEK_CUR); // jump nroRegRem
        //Alocando espaco para a matriz 
        id_index = malloc(sizeof(INDEX)*(*id_index_size));

        while(fread(&removed, 1, sizeof(char), bin_file) != 0){
            if(removed == '0'){
                fread(&next_i, 1, sizeof(int), bin_file);
                fread(&id, 1, sizeof(int), bin_file);

                id_index[i].id  = id;
                id_index[i].rrn = rrn;
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
        id_index = malloc(sizeof(INDEX)*(size));

        long int BOS = VARIABLE_REC_HEADER;
        int record_size = 0;

        while(fread(&removed, 1, sizeof(char), bin_file) != 0){
            if(removed == '0'){
                fread(&record_size, 1, sizeof(int), bin_file);
                fread(&next_li, 1, sizeof(long int), bin_file);
                fread(&id, 1, sizeof(int), bin_file);

                id_index[i].id  = id;
                id_index[i].BOS = BOS;

                //ByteOffset = record_size + 5 (removed(1Byte) + size(4Bytes))
                BOS += record_size + 5;
                fseek(bin_file, record_size - 12, SEEK_CUR);                             
                i++;
            }else
                BOS += next_register(bin_file, type_file);    
        
            if(i >= size){
                size = size + size/5;
                id_index = realloc(id_index, sizeof(INDEX)*size);
            }

        }

        *id_index_size = i - 1;
        if(*id_index_size != size)
            id_index = realloc(id_index, sizeof(INDEX)*i);
    }

    sort_id_index(id_index, *id_index_size);
    return id_index;
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

INDEX* read_index_file(FILE* index_file, int* id_index_size, int type_file){
    if(index_file == NULL)
        return NULL;

    int id = 0, size = 1100, i = 0;
    INDEX* id_index = malloc(sizeof(INDEX)*size);
    
    int rrn = 0;
    long int BOS = 0;

    while(fread(&id, 1, sizeof(int), index_file) != 0){
        id_index[i].id  = id;
        if(type_file == 1){
            fread(&rrn, 1, sizeof(int), index_file);
            id_index[i].rrn = rrn;
        }     
        else if(type_file == 2)
            fread(&BOS, 1, sizeof(long int), index_file);
            id_index[i].BOS = BOS;

        i++;

        if(i >= size){
            //como os arquivos nao constumam ter + de 1000 registros
            //o aumento ocorre gradativamente
            size += size + size/5;
            id_index = realloc(id_index, sizeof(INDEX)*size);
        }
    }
    

    *id_index_size = i;
    if(*id_index_size != size) 
        id_index = realloc(id_index, sizeof(INDEX)*i);
    
    sort_id_index(id_index, *id_index_size);
    return id_index;
}

int print_index_table(INDEX* id_index, int id_index_size, int type_file){
    if(id_index == NULL)
        return -1;

    for(int i = 0; i < id_index_size; i += 1){
        if(type_file == 1)
            printf("[%d] %d  |  ", id_index[i].id, id_index[i].rrn);
        if(type_file == 2)
            printf("[%d] %ld  |  ", id_index[i].id, id_index[i].BOS);
    }
    
    printf("idindexsize %d\n", id_index_size);
    return 1;
}

void free_index_array(INDEX* id_index){
    if(id_index != NULL)
        free(id_index);
}

void update_id_index(
    INDEX* id_index, int mid, 
    int type_file, int mode, int end
){
    if(mode == 1){ //excluindo
        id_index[mid].id = -1;
        
        if(type_file == 1)
            id_index[mid].rrn  = -1;
        else
            id_index[mid].BOS = -1;

        //swap
        INDEX aux = id_index[mid];
        id_index[mid] = id_index[end];
        id_index[end] = aux;
    }
}

int recover_rrn(
    INDEX* id_index, 
    int id, int id_index_size, 
    int type_file, int* rrn, long int* BOS
){  
    //BUSCA BINARIA SIMPLES
    int begin = 0;
    int end   = (id_index_size)-1;
    //printf("\n\nFIM %d id end %d\n\n",end,id_index[end].id);
    //printf("BUSCANDO %d -> ", id);
    int mid = (begin + end)/2; 
    while(begin <= end){
        //printf("[%d] %d [%d] | ", id_index[begin].id, id_index[mid].id, id_index[end].id);
        if(id_index[mid].id == id){
            //printf("\nACHEI O %d\n", id_index[mid].id);
            if(type_file == 1)
                *rrn = id_index[mid].rrn;
            else
                *BOS = id_index[mid].BOS;
            return mid;   
        }
                
        if(id > id_index[mid].id)
            begin = mid + 1;
        else
            end = mid - 1;        

        mid = (begin + end)/2;   
    }
    
    return -1;
}

int qsort_compare(const void* a, const  void* b){
    return ((INDEX*)a)->id - ((INDEX*)b)->id;
}

void sort_id_index(INDEX* array, int array_size){
    qsort(array, array_size, sizeof(INDEX), qsort_compare);
}