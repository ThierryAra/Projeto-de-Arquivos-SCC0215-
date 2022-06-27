//#ifdef HEADER_

#include<stdio.h>

typedef struct header{
    char status;
    int top_rrn;
    long int top_BOS;
    char description[40];
    char desC1[22];
    char desC2[19];
    char desC3[24];
    char desC4[8];
    char codC5;
    char desC5[16];
    char codC6;
    char desC6[18];
    char codC7;
    char desC7[19];
    int numRegRem;
}HEADER;

/*  Allocates space for a header and returns it */
HEADER* create_header();

/*  Free the stored space for header  */
void free_header(HEADER* h);

/*  Write the header to the binary file so that it depends on
    type_file parameter (1 -> file1 or 2 -> file2)
    Returns:
             1 if run correctly
            -2 if any parameter is null */
int write_header(HEADER* h, FILE* bin_file, int type_file);

/*  If header status is 1 updates the file to status 0
    and vice versa
    Returns:
            1 if run correctly
            0 if parameter is null */                  
char update_status(FILE* bin_file);

/*  Checks your archive status
    Returns:
             1 = consistent file
             0 = inconsistent file    
            -1 = pointer is at the wrong position */
int check_status(FILE* bin_file);

int ignore_header(FILE* bin_file, int type_file);

int update_header(FILE* bin_file, HEADER* h, int type_file, int next_RRN, long int next_BOS);