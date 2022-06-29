/*  Removes the header from the csv file  */
int remove_header_csv(FILE* file_csv);

/* Reads an integer value from file and removes ',' after the field 
    Returns  1 if everything was correct
             0 if the field is empty
            -1 if there was an error in the reading   */
int read_int_field(FILE* file, int* value);

/*  Read a string field from csv file
    Returns  1 if everything was correct
             0 if the field is empty
            -1 if there was an error in the reading   */
int read_char_field(char* string, FILE* file_csv);

/*  Reads a file field and assigns it to a string variable
    Returns 1 if there is a value
           -1 if it is an empty field   */
int read_word(char* string, FILE* file_csv);

/*  Read a string from the standard input that is surrounded by
    double quotes, removing them                            */
void scan_quote_strings(char* string);

/*  Allocates space and returns an array of 'n' string */
char** create_array_fields(int n);

/*  Read a data entry in the format {field, "string"/value} and 
    indicate if there is a field called ID, returning its position if it exists */
char** read_search_fields(int n, int* is_there_id);

int free_array_fields(char** array, int n);

void binarioNaTela(char *nomeArquivoBinario);