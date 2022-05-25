/* Retorna 1 se ha uma palavra, 0 se nao ha ou -1 se houve erro */
int check_field(FILE* file);

/* Le um valor inteiro em file e remove a ',' apos o campo 
    Retorna 1 se ocorreu tudo correto
            0 se o campo eh vazio
           -1 se houve erro na leitura                  */
int read_int_field(FILE* file, int* value);

/*  Remove o header do arquivo csv */
int remove_header(FILE* file_csv);

/*  Le um campo de string do arquivo csv
    Retorna 1 se ocorreu tudo correto
            0 se o campo eh vazio
           -1 se houve erro na leitura  */
int read_char_field(char* string, FILE* file_csv);

/*  Le um campo de file e atribui a variavel string
    Retorna 1 se ha valor
           -1 se eh um campo vazio               */
int read_word(char* string, FILE* file_csv);

/*  Le uma string da entrada padrao que esteja cercada por
    aspas duplas, as removendo                          */
void scan_quote_strings(char* string);

char** create_array_fields_sfw(int n);

int free_array_fields_sfw(char** array, int n);

void binarioNaTela(char *nomeArquivoBinario);