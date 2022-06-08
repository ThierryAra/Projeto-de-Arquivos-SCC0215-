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
    ERROR -> RETURNS -2
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
    int res;           // function return
    char name_csv[20];
    char type_file[6];
    char name_bin[200];
    FILE* bin_file;

    scanf("%d", &option);

    switch (option){
        case 1:      // CREATE TABLE
            read_word(type_file, stdin);
            read_word(name_csv, stdin); 
            read_word(name_bin, stdin);

            FILE* csv_file = fopen(name_csv, "r");
            remove_header_csv(csv_file);
            bin_file = fopen(name_bin, "wb+");

            if(strcmp(type_file, "tipo1") == 0){
                res = create_table(csv_file, bin_file, 1);
                
                if(res < 1)
                    printf("Falha no processamento do arquivo.");
            }else if(strcmp(type_file, "tipo2") == 0){
                res = create_table(csv_file, bin_file, 2);
                
                if(res < 1)
                    printf("Falha no processamento do arquivo.");
            }else    
                printf("Falha no processamento do arquivo.");
            
            if(res == 1 && bin_file != NULL){
                fclose(bin_file);
                binarioNaTela(name_bin);
            }
            if(csv_file != NULL) fclose(csv_file);  
            break;

        case 2:     // SELECT .. FROM
            read_word(type_file, stdin);
            read_word(name_bin, stdin);
            bin_file = fopen(name_bin, "rb");

            res = 0;
            if(strcmp(type_file, "tipo1") == 0){
                res = select_from(bin_file, 1);
                
                if(res == -2)
                    printf("Falha no processamento do arquivo.");
            }else if(strcmp(type_file, "tipo2") == 0){
                res = select_from(bin_file, 2);
                
                if(res == -2)
                    printf("Falha no processamento do arquivo.");
            }else    
                printf("Falha no processamento do arquivo.");

            break;

        case 3:;     // SELECT .. FROM .. WHERE
            read_word(type_file, stdin);
            read_word(name_bin, stdin);
            bin_file = fopen(name_bin, "rb");

            //number of searched fields
            int n;
            scanf("%d", &n);

            //array that will contain the fields and values to be searched
            char** array = create_array_fields_sfw(n);
            
            //{(field_i, value_i), ...}
            for (int i = 0; i < n*2; i++){
                read_word(array[i], stdin);
                scan_quote_strings(array[++i]);
            }
            
            res = 1;

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

            free_array_fields_sfw(array, n);
            break;

        case 4:     //search by RRN 
            read_word(type_file, stdin);
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
            char name_index_bin[20];

            read_word(type_file, stdin);
            read_word(name_bin, stdin);
            read_word(name_index_bin, stdin);
            
            bin_file = fopen(name_bin, "rb");
            FILE* bin_index_file = fopen(name_index_bin, "w+b");

             if(strcmp(type_file, "tipo1") == 0){
                res = create_index_id(bin_file, bin_index_file, 1);            
            }else if(strcmp(type_file, "tipo2") == 0){
                res = create_index_id(bin_file, bin_index_file, 2);
            }else    
                printf("Falha no processamento do arquivo.");

            if(res < 0)
                printf("Falha no processamento do arquivo."); 
            else{
                fclose(bin_index_file);
                bin_index_file = NULL;

                binarioNaTela(name_index_bin);
            }

            if(bin_index_file != NULL) fclose(bin_index_file);  
            break;            
    }

    if(bin_file == NULL)
        printf("ARQUIVO NULL");
    if(bin_file != NULL) fclose(bin_file);  
    return 0;
}