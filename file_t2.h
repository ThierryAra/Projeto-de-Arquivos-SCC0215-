#include "write_read_file.h"

typedef struct record_t2 Record_t2;

Record_t2* create_record_t2();

int create_table_t2(FILE* file_csv, FILE* file_bin);

/*  Libera todo o espaco armazenado para r2 */
void free_rec_t2(Record_t2* r2);

