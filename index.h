int create_index_id(FILE* bin_file, FILE* index_file, int type_file);

int print_index_file(FILE* index_file, int type_file);

void* create_index_vec(FILE* bin_file, FILE* index_file, int* id_indexes_size, int type_file);

int print_index_table(void* id_indexes, int id_indexes_size, int type_file);

int free_index_table(int* id_indexes);

int recover_rrn(int* id_indexes, int id, int id_indexes_size, int mode);