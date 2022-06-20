#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"header.h"
#include"record.h"

HEADER* create_header(){
    HEADER* header = malloc(sizeof(HEADER));

    header->status = '0';
    strcpy(header->description, "LISTAGEM DA FROTA DOS VEICULOS NO BRASIL");
    strcpy(header->desC1, "CODIGO IDENTIFICADOR: ");
    strcpy(header->desC2, "ANO DE FABRICACAO: ");
    strcpy(header->desC3, "QUANTIDADE DE VEICULOS: ");
    strcpy(header->desC4, "ESTADO: ");
    header->codC5 = '0';
    strcpy(header->desC5, "NOME DA CIDADE: ");
    header->codC6 = '1';
    strcpy(header->desC6, "MARCA DO VEICULO: ");
    header->codC7 = '2';
    strcpy(header->desC7, "MODELO DO VEICULO: ");
    header->numRegRem = 0;

    return header;
}

int free_header(HEADER* h){
    if(h == NULL)
        return -2;

    free(h); h = NULL;
    return 1;
}

int write_header(HEADER* h, FILE* bin_file, int type_file){
    if(bin_file == NULL)    
        return -2;

    //values to be saved as default
    int i = 0, ni = -1; 
    long int li = 0, nli = -1;
    
    fwrite(&h->status, 1, sizeof(char), bin_file);   //status
    
    if(type_file == 1)
        fwrite(&ni, 1, sizeof(int), bin_file);       //topo
    else if(type_file == 2)
        fwrite(&nli, 1, sizeof(long int), bin_file);                   
    
    fwrite(h->description, 40, sizeof(char), bin_file);                           
    fwrite(h->desC1, 22, sizeof(char), bin_file);
    fwrite(h->desC2, 19, sizeof(char), bin_file);
    fwrite(h->desC3, 24, sizeof(char), bin_file); 
    fwrite(h->desC4, 8, sizeof(char), bin_file);
    fwrite(&h->codC5, 1, sizeof(char), bin_file);
    fwrite(h->desC5, 16, sizeof(char), bin_file);
    fwrite(&h->codC6, 1, sizeof(char), bin_file);
    fwrite(h->desC6, 18, sizeof(char), bin_file);
    fwrite(&h->codC7, 1, sizeof(char), bin_file);
    fwrite(h->desC7, 19, sizeof(char), bin_file);
    
    if(type_file == 1)
        fwrite(&i, 1, sizeof(int), bin_file);         //proxRRN               
    else if(type_file == 2)
        fwrite(&li, 1, sizeof(long int), bin_file);   //proxByteOffset

    fwrite(&i, 1, sizeof(int), bin_file);             //nroRegRem
    
    return 1;                          
}

char update_status(FILE* bin_file){
    if(bin_file == NULL)
        return 0;

    char status = 0;
    
    fseek(bin_file, 0, SEEK_SET);
    fread(&status, 1, sizeof(char), bin_file);
    //printf("li %c -> ", status);
    if(status == '0'){
        fseek(bin_file, 0, SEEK_SET);
        fwrite("1", 1, sizeof(char), bin_file);
        status = '1';
    }else{
        fseek(bin_file, 0, SEEK_SET);
        fwrite("0", 1, sizeof(char), bin_file);
        status = '0';
    }

    //printf(" %c\n", status);
    return status;
}

int check_status(FILE* bin_file){
    char status;
    fseek(bin_file, 0, SEEK_SET);

    fread(&status, 1, sizeof(char), bin_file);
    
    if(status == '0')
        return 0;
    else if(status == '1')
        return 1;
    else 
        return -1;
}