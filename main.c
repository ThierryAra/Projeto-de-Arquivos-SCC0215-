/* 
    ERRO -> RETORNA -2
    DADO INEXISTENTE -> RETORNA -1
*/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"write_read_file.h"
#include"file_t1.h"
#include"file_t2.h"

int main(){

    //vai armazenar a opcao selecionada
    int option;
    // vai armazenar o retorno das funcoes
    int res;
    char name_csv[20];
    char type_file[6];
    char name_bin[20];
    FILE* bin_file;

    scanf("%d", &option);

    switch (option){
        case 1:      // CREATE TABLE
            read_word(type_file, stdin);
            read_word(name_csv, stdin); 
            read_word(name_bin, stdin);

            FILE* csv_file = fopen(name_csv, "r");
            remove_header(csv_file);
            bin_file = fopen(name_bin, "wb+");

            if(strcmp(type_file, "tipo1") == 0){
                res = create_table_t1(csv_file, bin_file);
                if(res < 1)
                    printf("Falha no processamento do arquivo.");
                else 
                    binarioNaTela(name_bin);
            }else if(strcmp(type_file, "tipo2") == 0){
                res = create_table_t2(csv_file, bin_file);
                if(res < 1)
                    printf("Falha no processamento do arquivo.");
                else 
                    binarioNaTela(name_bin);
            }else    
                printf("Falha no processamento do arquivo.");
            
            if(bin_file != NULL) fclose(bin_file);
            if(csv_file != NULL) fclose(csv_file);  
            break;

        case 2:     // SELECT .. FROM
            read_word(type_file, stdin);
            read_word(name_bin, stdin);
            bin_file = fopen(name_bin, "rb");

            res = 0;
            if(strcmp(type_file, "tipo1") == 0){
                res = select_from_r1(bin_file);
                if(res == -2)
                    printf("Falha no processamento do arquivo.");
            }else if(strcmp(type_file, "tipo2") == 0){
                //res = select_from_r1(bin_file);
                //if(res == -2)
                //    printf("Falha no processamento do arquivo.");
            }else    
                printf("Falha no processamento do arquivo.");

            if(bin_file != NULL) fclose(bin_file);
            break;

        case 3:;     // SELECT .. FROM .. WHERE
            read_word(type_file, stdin);
            read_word(name_bin, stdin);
            bin_file = fopen(name_bin, "rb");

            //quantidade de campos buscados
            int n;
            scanf("%d", &n);

            //array que vai conter os campos e valores a serem buscados
            char** array = create_array_fields_sfw(n);
            
            //{(campo_i, valor_i), ...}
            for (int i = 0; i < n*2; i++){
                read_word(array[i], stdin);
                scan_quote_strings(array[++i]);
            }
            
            res = 1;

            if(strcmp(type_file, "tipo1") == 0){
                res = select_from_where_r1(bin_file, array, n);                
            }else if(strcmp(type_file, "tipo2") == 0){
                //select_from_where_r2(bin_file, array, n);
            }else    
                printf("Falha no processamento do arquivo.");

            if(res == -2)
                printf("Falha no processamento do arquivo.");
            else if(res == 0)
                printf("Registro inexistente.");

            free_array_fields_sfw(array, n);
            if(bin_file != NULL) fclose(bin_file);
            break;

        case 4:     //busca por RRN 
            read_word(type_file, stdin);
            read_word(name_bin, stdin);
            int rrn; scanf("%d", &rrn);

            Record_t1* r1 = create_record_t1();
            FILE* bin_file = fopen(name_bin, "rb");
            
            res = search_rrn(type_file, bin_file, rrn, r1);
            if(res == -2)
                printf("Falha no processamento do arquivo."); 
            else if(res == -1)
                printf("Registro inexistente.");
            else
                print_r1(r1);

            free_rec_t1(r1);
            if(bin_file != NULL) fclose(bin_file);  
            break;
    }

    return 0;
}