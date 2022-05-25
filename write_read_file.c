#include<stdio.h>
#include<stdlib.h>

int remove_header(FILE* file_csv){
    char c;

    do{
        c = fgetc(file_csv);
    }while(c != EOF && c != '\n');

    return 1;
}

int check_field(FILE* file){
    char c = fgetc(file);
    //verifica se o campo eh nulo
    if(c == ',' || c == '\n' || c == '\r' || c == EOF) 
        return -1;
    else{
        ungetc(c, file);
        return 1;
    }
}

int read_int_field(FILE* file, int* value){
    if(check_field(file) == -1)
        return -1;
    
    fscanf(file, "%d", value);
    
    //remove a ','
    char c; fscanf(file, "%c", &c);
    return 1;
}

int read_char_field(char* string, FILE* file){
    char c = 0;
    int i = 0;

    if(check_field(file) == -1)
        return -1;

    c = fgetc(file);
    
    do{
        string[i] = c;
        i++;
        c = fgetc(file);
    }while(c != ',' && c != '\r' && c != '\n' && c != EOF);

    string[i]  = '\0';

    return 1;
}

int read_word(char* string, FILE* file){
    char c = 0;
    int i = 0;

    c = fgetc(file);
    
    if(c == 0)
        return 1;

    //remove os caracteres desnecessarios antes da palavra
    while(c == '\n' || c == '\r' || c == ' ')
        c = fgetc(file);
    
    do{
        string[i] = c;
        i++;
        c = fgetc(file);
    }while(c != '\n' && c != '\r' && c != ' ' && c != EOF);

    string[i]  = '\0';
    return 1;
}

void scan_quote_strings(char* string){
    char c = 0;
    int i = 0;

    c = getchar();   
    //remove os caracteres desnecessarios antes da palavra
    while(c == '\n' || c == '\r' || c == '"')
        c = getchar();
    
    do{
        string[i] = c;
        i++;
        c = getchar();
    }while(c != '\n' && c != '\r' && c != '"' && c != EOF);

    string[i]  = '\0';
    //return 1;
}

char** create_array_fields_sfw(int n){
    if(n == 0)
        return NULL;

    //vetor = [(campo1, valor1), ..., (campo_n, valor_n)]
    char** array = malloc(n*2 * sizeof(char*));

    for(int i = 0; i < n*2; i++)
        array[i] = malloc(30*sizeof(char));
    
    return array;
}

int free_array_fields_sfw(char** array, int n){
    if(array == NULL || n <= 0)
        return -1;

    for(int i = 0; i < n*2; i++)
        free(array[i]);

    free(array);
    return 1;
}

void binarioNaTela(char *nomeArquivoBinario) { /* Você não precisa entender o código dessa função. */

	/* Use essa função para comparação no run.codes. Lembre-se de ter fechado (fclose) o arquivo anteriormente.
	*  Ela vai abrir de novo para leitura e depois fechar (você não vai perder pontos por isso se usar ela). */

	unsigned long i, cs;
	unsigned char *mb;
	size_t fl;
	FILE *fs;
	if(nomeArquivoBinario == NULL || !(fs = fopen(nomeArquivoBinario, "rb"))) {
		fprintf(stderr, "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): não foi possível abrir o arquivo que me passou para leitura. Ele existe e você tá passando o nome certo? Você lembrou de fechar ele com fclose depois de usar?\n");
		return;
	}
	fseek(fs, 0, SEEK_END);
	fl = ftell(fs);
	fseek(fs, 0, SEEK_SET);
	mb = (unsigned char *) malloc(fl);
	fread(mb, 1, fl, fs);

	cs = 0;
	for(i = 0; i < fl; i++) {
		cs += (unsigned long) mb[i];
	}
	printf("%lf\n", (cs / (double) 100));
	free(mb);
	fclose(fs);
}