
typedef struct record_t2 Record_t2;

Record_t2* create_record_t2();

int create_table_t2(FILE* csv_file, FILE* bin_file);

/*  Libera todo o espaco armazenado para r2 */
void free_rec_t2(Record_t2* r2);

int select_from_r2(FILE *bin_file);

int select_from_where_r2(FILE* bin_file, char** fields, int n);