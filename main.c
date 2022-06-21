/*********************************************************************************
 |                  USP - University of São Paulo (Brazil)                       |
 |          ICMC - Institute of Mathematical and Computer Sciences               |
 *********************************************************************************
 |                        Bachelor in Computer Science                           |
 |                                 2022/1                                        |
 |                      SCC0215 - Archives Organization                          |
 |                                                                               |
 |Authors: Gustavo Sampaio Lima (12623992) and Thierry de Souza Araujo (12681094)|
 *********************************************************************************
  > "Primeiro Trabalho Pratico"
  > Objective: Code that can store and retrieve data related
    related to the vehicle fleet list in Brazil, which is maintained by Denatran. 
    The files can have fixed or variable sizes
 ********************************************************************************/

/* 
    ERROR           -> RETURNS -2
    INEXISTENT DATA -> RETURNS -1
*/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"useful.h"
#include"record.h"
#include"index.h"

int main(){

    int option;
    int res = 1;           // function return
    char name_csv[20];
    char type_file[6];
    char name_bin[200];
    FILE* bin_file = NULL;

    char name_index_bin[20];
    FILE* bin_index_file = NULL;

    scanf("%d", &option);
    read_word(type_file, stdin);

    switch (option){
        case 1:      // CREATE TABLE
            read_word(name_csv, stdin); 
            read_word(name_bin, stdin);

            FILE* csv_file = fopen(name_csv, "r");
            remove_header_csv(csv_file);
            bin_file = fopen(name_bin, "wb+");

            if(strcmp(type_file, "tipo1") == 0){
                res = create_table(csv_file, bin_file, 1);
            }else if(strcmp(type_file, "tipo2") == 0){
                res = create_table(csv_file, bin_file, 2);
            }else    
                printf("Falha no processamento do arquivo.");
            
            if(res == 1 && bin_file != NULL){
                fclose(bin_file);
                binarioNaTela(name_bin);
            }

            if(res < 1)
                printf("Falha no processamento do arquivo.");

            if(csv_file != NULL) fclose(csv_file);  
            break;

        case 2:     // SELECT .. FROM
            read_word(name_bin, stdin);
            bin_file = fopen(name_bin, "rb");

            if(strcmp(type_file, "tipo1") == 0){
                res = select_from(bin_file, 1);
            }else if(strcmp(type_file, "tipo2") == 0){
                res = select_from(bin_file, 2);
            }else    
                printf("Falha no processamento do arquivo.");

            if(res == -2)
                printf("Falha no processamento do arquivo.");
            
            break;

        case 3:;     // SELECT .. FROM .. WHERE
            read_word(name_bin, stdin);
            bin_file = fopen(name_bin, "rb");

            //number of searched fields
            int n, id_trash;
            scanf("%d", &n);

            //array that will contain the fields and values to be searched
            char** array = read_search_fields(n, &id_trash);
            
            if(strcmp(type_file, "tipo1") == 0){
                res = select_from_where(bin_file, array, n, 1);                
            }else if(strcmp(type_file, "tipo2") == 0){
                res = select_from_where(bin_file, array, n, 2);
            }else    
                printf("Falha no processamento do arquivo.");

            if(res == -2)
                printf("Falha no processamento do arquivo.");
            else if(res == 0)
                printf("Registro inexistente.");

            free_array_fields(array, n);
            break;

        case 4:     //search by RRN 
            read_word(name_bin, stdin);
            int rrn; scanf("%d", &rrn);

            RECORD* r1 = create_record();
            bin_file = fopen(name_bin, "rb");
            
            res = search_rrn(type_file, bin_file, rrn, r1);
            if(res == -2)
                printf("Falha no processamento do arquivo."); 
            else if(res == -1)
                printf("Registro inexistente.");
            else
                print_record(r1);

            free_rec(r1);  
            break;
        case 5: ;
            read_word(name_bin, stdin);
            read_word(name_index_bin, stdin);
            
            bin_file = fopen(name_bin, "rb");
            bin_index_file = fopen(name_index_bin, "w+b");

            if(strcmp(type_file, "tipo1") == 0){
                res = create_index_id(bin_file, bin_index_file, 1);            
            }else if(strcmp(type_file, "tipo2") == 0){
                res = create_index_id(bin_file, bin_index_file, 2);
            }else    
                printf("Falha no processamento do arquivo.");

            if(res < 0)
                printf("Falha no processamento do arquivo."); 
            else{
                //print_index_file(bin_index_file, 2);
                fclose(bin_index_file);
                bin_index_file = NULL;

                binarioNaTela(name_index_bin);
            }

            break;  
        
        case 6:  ;
            read_word(name_bin, stdin);
            read_word(name_index_bin, stdin);

            int delete_amount = 0;
            scanf("%d", &delete_amount);

            bin_file = fopen(name_bin, "r+b");
            bin_index_file = fopen(name_index_bin, "r+b");

            if(strcmp(type_file, "tipo1") == 0){
                res = delete_where(bin_file, bin_index_file, delete_amount, 1);
            }else if(strcmp(type_file, "tipo2") == 0){
                res = delete_where(bin_file, bin_index_file, delete_amount, 2);
            }else    
                printf("Falha no processamento do arquivo.");

            if(res < 0)
                printf("Falha no processamento do arquivo."); 
            else{
                //print_index_file(bin_index_file, 2);
                fclose(bin_index_file);
                fclose(bin_file);
                bin_index_file = NULL;
                bin_file       = NULL;

                binarioNaTela(name_bin);
                binarioNaTela(name_index_bin);
            }
            break;
        case 7:;
            
            //read_word(name, stdin);;
            char name2[200] = "/home/thierry/Documentos/USP/C/3_semestre/Arquivos/Projeto-de-Aquivos-SCC0215-/arquivos/depois/indice5.bin";
            
            bin_file = fopen(name2, "rb");
            
            int size = 0;
            INDEX* index= read_index_file(bin_file, &size, 1);
            print_index_table(index, size, 1);
    }

    if(bin_file != NULL) 
        fclose(bin_file);  
    
    return 0;
}