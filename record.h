typedef struct record RECORD;

RECORD* create_record();

void free_rec(RECORD* r);

int create_table(FILE* csv_file, FILE* bin_file, int type_file);

int select_from(FILE* bin_file, int type_file);

int select_from_where(FILE* bin_file, char** fields, int n, int type_file);

int search_rrn(char* type_file, FILE* bin_file, int rrn, RECORD* r);

int print_record(RECORD* r);