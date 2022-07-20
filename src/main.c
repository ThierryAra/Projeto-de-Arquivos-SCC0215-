/*********************************************************************************
 |                  USP - University of São Paulo (Brazil)                       |
 |          ICMC - Institute of Mathematical and Computer Sciences               |
 *********************************************************************************
 |                        Bachelor in Computer Science                           |
 |                                 2022/1                                        |
 |                      SCC0215 - Archives Organization                          |
 |                                                                               |
 |Authors: Gustavo Sampaio Lima (12623992) and Thierry de Souza Araujo (12681094)|
 |Percentage: Gustavo (100%) //  Thierry(100%)                                   |
 *********************************************************************************
  > "Terceiro Trabalho Pratico"
  > Objective: Implement a C program through which the user can insert, remove and
   update data from binary files, as well as create indexes for index these files.
   The indexes are characterized by being of the B-tree type.
 ********************************************************************************/

/* 
    ERROR           -> RETURNS -2
    INEXISTENT DATA -> RETURNS -1
*/
#define INEXISTENT_DATA -1
#define ERROR -2
#define SUCESS 1

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"../headers/record.h"
#include"../headers/useful.h"
#include"../headers/index.h"
#include"../headers/index_B.h"

int main(){

    int option;
    int res = 1;           // Function return
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
            
            if(res == SUCESS && bin_file != NULL){
                fclose(bin_file);
                binarioNaTela(name_bin);
            }

            if(res != SUCESS)
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

            if(res == ERROR)
                printf("Falha no processamento do arquivo.");
            
            break;

        case 3:     // SELECT .. FROM .. WHERE
            read_word(name_bin, stdin);
            bin_file = fopen(name_bin, "rb");

            // Number of searched fields
            int n, id_trash;
            scanf("%d", &n);

            // Array that will contain the fields and values to be searched
            char** array = read_search_fields(n, &id_trash);
            
            if(strcmp(type_file, "tipo1") == 0){
                res = select_from_where(bin_file, array, n, 1);                
            }else if(strcmp(type_file, "tipo2") == 0){
                res = select_from_where(bin_file, array, n, 2);
            }else    
                printf("Falha no processamento do arquivo.");

            if(res == ERROR)
                printf("Falha no processamento do arquivo.");
            else if(res == 0)
                printf("Registro inexistente.");

            printf("res = %d\n", res);
            free_array_fields(array, n);
            break;

        case 4:     // Search by RRN 
            read_word(name_bin, stdin);
            int rrn; scanf("%d", &rrn);

            RECORD* r1 = create_record();
            bin_file = fopen(name_bin, "rb");
            
            res = search_rrn(type_file, bin_file, rrn, r1);
            if(res == ERROR)
                printf("Falha no processamento do arquivo."); 
            else if(res == INEXISTENT_DATA)
                printf("Registro inexistente.");
            else
                print_record(r1);

            free_rec(r1);  
            break;
        case 5:     //CREATE ID INDEX
            read_word(name_bin, stdin);
            read_word(name_index_bin, stdin);
            
            bin_file = fopen(name_bin, "rb");
            bin_index_file = fopen(name_index_bin, "w+b");

            if(strcmp(type_file, "tipo1") == 0){
                res = create_index_id(bin_file, bin_index_file, 1);            
            }else if(strcmp(type_file, "tipo2") == 0){
                res = create_index_id(bin_file, bin_index_file, 2);
            }else{
                fclose(bin_index_file);
                printf("Falha no processamento do arquivo.");
            }    

            fclose(bin_index_file);
            
            if(res != SUCESS){
                printf("Falha no processamento do arquivo."); 
            }else{
                bin_index_file = NULL;

                binarioNaTela(name_index_bin);
            }
            break;  
        
        case 6:     //DELETE ... WHERE
            read_word(name_bin, stdin);
            read_word(name_index_bin, stdin);

            int delete_amount = 0;
            scanf("%d", &delete_amount);
            bin_file = fopen(name_bin, "r+b");

            if(strcmp(type_file, "tipo1") == 0){
                res = delete_where(bin_file, name_index_bin, delete_amount, 1);
            }else if(strcmp(type_file, "tipo2") == 0){
                res = delete_where(bin_file, name_index_bin, delete_amount, 2);
            }else    
                printf("Falha no processamento do arquivo.");

            if(res != SUCESS)
                printf("Falha no processamento do arquivo."); 
            else{
                fclose(bin_file);
                bin_file = NULL;
                
                binarioNaTela(name_bin);
                binarioNaTela(name_index_bin);
            }
            
            break;
        case 7:     //INSERT ... INTO
            read_word(name_bin, stdin);
            read_word(name_index_bin, stdin);

            int insertion_amount = 0;
            scanf("%d\r\n", &insertion_amount);

            bin_file = fopen(name_bin, "r+b");

            if(strcmp(type_file, "tipo1") == 0)
                res = insert_into(bin_file, name_index_bin, insertion_amount, 1, SIMPLE_INDEX);
            else if(strcmp(type_file, "tipo2") == 0)
                res = insert_into(bin_file, name_index_bin, insertion_amount, 2, SIMPLE_INDEX);
            else
                printf("Falha no processamento do arquivo.");

            if(res != SUCESS)
                printf("Falha no processamento do arquivo."); 
            else{
                fclose(bin_file);
                bin_file = NULL;
                
                binarioNaTela(name_bin);
                binarioNaTela(name_index_bin);
            }
            break;
        case 8:;    //UPDATE ... WHERE
            read_word(name_bin, stdin);
            read_word(name_index_bin, stdin);

            int update_amount = 0;
            scanf("%d\r\n", &update_amount);

            bin_file = fopen(name_bin, "r+b");

            if(strcmp(type_file, "tipo1") == 0)
                res = update_where(bin_file, name_index_bin, update_amount, 1);
            else if(strcmp(type_file, "tipo2") == 0)
                res = update_where(bin_file, name_index_bin, update_amount, 2);
            else
                printf("Falha no processamento do arquivo.");

            if(res != SUCESS)
                printf("Falha no processamento do arquivo."); 
            else{
                fclose(bin_file);
                bin_file = NULL;
                
                binarioNaTela(name_bin);
                binarioNaTela(name_index_bin);
            }
        case 9:     //CREATE INDEX (B-TREE)
            read_word(name_bin, stdin);
            read_word(name_index_bin, stdin);

            bin_file = fopen(name_bin, "rb");
            bin_index_file = fopen(name_index_bin, "w+b");

            if(strcmp(type_file, "tipo1") == 0)
                res = create_b_tree_index(bin_file, bin_index_file, 1);
            else if(strcmp(type_file, "tipo2") == 0)
                res = create_b_tree_index(bin_file, bin_index_file, 2);
            else
                printf("Falha no processamento do arquivo.");

            if(res != SUCESS)
                printf("Falha no processamento do arquivo."); 
            else{
                fclose(bin_file);
                fclose(bin_index_file);
                bin_file = NULL;
                bin_index_file = NULL;
                
                binarioNaTela(name_index_bin);
            }

            break;
        case 10:     //SELECT ... WHERE (B-TREE)
            read_word(name_bin, stdin);
            read_word(name_index_bin, stdin);

            bin_file = fopen(name_bin, "rb");
            bin_index_file = fopen(name_index_bin, "rb");

            if(strcmp(type_file, "tipo1") == 0)
                res = search_with_b_tree(bin_file, bin_index_file, 1);
            else if(strcmp(type_file, "tipo2") == 0)
                res = search_with_b_tree(bin_file, bin_index_file, 2);
            else
                printf("Falha no processamento do arquivo.");
            
            if(res == ERROR)
                printf("Falha no processamento do arquivo.");
            else if(res == INEXISTENT_DATA)
                printf("Registro inexistente.");
            break;
        case 11:     //INSERT ... INTO (B-TREE)
            read_word(name_bin, stdin);
            read_word(name_index_bin, stdin);

            int insertion_amt = 0;
            scanf("%d\r\n", &insertion_amt);

            bin_file = fopen(name_bin, "r+b");

            if(strcmp(type_file, "tipo1") == 0)
                res = insert_into(bin_file, name_index_bin, insertion_amt, 1, B_TREE_INDEX);
            else if(strcmp(type_file, "tipo2") == 0)
                res = insert_into(bin_file, name_index_bin, insertion_amt, 2, B_TREE_INDEX);
            else
                printf("Falha no processamento do arquivo.");

            if(res != SUCESS)
                printf("Falha no processamento do arquivo."); 
            else{
                fclose(bin_file);
                bin_file = NULL;
                
                binarioNaTela(name_bin);
                binarioNaTela(name_index_bin);
            }
                
    }

    if(bin_file != NULL) 
        fclose(bin_file);  
    if(bin_index_file != NULL)
        fclose(bin_index_file);
    
    return 0;
}