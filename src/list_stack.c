#include<stdio.h>
#include<stdlib.h>
#include"../headers/record.h"
#include"../headers/index.h"
#include"../headers/header.h"
#include"../headers/list_stack.h"

/*
    Validates the 'numRecRem' field of the file header, both for
    removals and additions of records
        mode
             1 -> Removal
             2 -> Addition                                       */
void att_numRecRem(FILE* bin_file, int mode, int type_file, int quantity){
    int numRecRem = 0;
    if(type_file == 1){
        fseek(bin_file, 178, SEEK_SET);
        fread(&numRecRem, 1, sizeof(int), bin_file);
    }else{
        fseek(bin_file, 186, SEEK_SET);
        fread(&numRecRem, 1, sizeof(int), bin_file);
    }

    //Removing a record
    if(mode == 1)
        if(numRecRem <= 0)
            numRecRem = quantity;
        else
            numRecRem += quantity;
    else //Adding a record
        numRecRem -= quantity;
        
    fseek(bin_file, -sizeof(int), SEEK_CUR);
    fwrite(&numRecRem, 1, sizeof(int), bin_file);
}

//-------------------------------STACK
struct node_stack{
    int rrn;
    int next;
};

struct stack{
    int rec_amount;
    int begin;
    NODE_stack* r_stack;
};

STACK* create_stack(int stack_size){
    STACK* stack = malloc(sizeof(STACK));

    stack->rec_amount = 0;
    stack->r_stack    = malloc(stack_size*sizeof(NODE_stack));
    stack->begin      = -1;
    return stack;
}

void free_stack(STACK* stack){
    if(stack != NULL){
        if(stack->r_stack != NULL)
            free(stack->r_stack);
        
        free(stack);
    }
}

void read_stack_top(FILE* bin_file, STACK* stack){
    fseek(bin_file, 1, SEEK_SET);
    int top_stack = -1;

    fread(&top_stack, 1, sizeof(int), bin_file);
    NODE_stack node_stack;

    node_stack.rrn  = top_stack;
    node_stack.next = -1;

    if(top_stack != -1){
        stack->r_stack[0] = node_stack;
        stack->begin = 0;
        stack->rec_amount++;
    }
}

void read_stack(FILE* bin_file, STACK* stack){
    int rrn = -1;
    int next_rrn = -1;

    //Reading the top of the stack
    fseek(bin_file, 1, SEEK_SET);
    fread(&rrn, 1, sizeof(int), bin_file);
    
    //initializing the top
    if(rrn != -1)
        stack->begin = 0;
    
    char c = 0;
    NODE_stack node;
    node.rrn = rrn;
    
    while(node.rrn != -1){
        //printf("node -> %d + ", node.rrn);
        jump_to_record(bin_file, node.rrn, 0);
        fseek(bin_file, 1, SEEK_CUR);
        fread(&next_rrn, 1, sizeof(int), bin_file);

        //adding the node on the stack
        node.next = stack->rec_amount+1;
        //printf("next %d\n", node.next);
        stack->r_stack[stack->rec_amount++] = node;

        //seeking the next node
        node.rrn = next_rrn;
    }

    if(stack->rec_amount != 0)
        stack->r_stack[stack->rec_amount-1].next = -1;
}

void add_stack(STACK* stack, int rrn){
    if(stack == NULL || rrn == -1)
        return;

    //printf("ADICIONADO NA STACK %d [%d]\n", rrn, stack->rec_amount);
    NODE_stack node_stack;
    node_stack.rrn  = rrn;
    node_stack.next = stack->begin;

    //Adding the new rrn and updating the top
    stack->begin = stack->rec_amount;
    stack->r_stack[stack->rec_amount++] = node_stack;
}

int write_stack(FILE* bin_file, STACK* stack){
    if(stack->rec_amount == 0)
        return -1;

    NODE_stack node = stack->r_stack[stack->begin];
    //updates the top of the stack
    fseek(bin_file, 1, SEEK_SET);
    fwrite(&node.rrn, 1, sizeof(int), bin_file);

    NODE_stack old_node;
    old_node.next = 0;
        
    //Adds the structure of the deleted records in the file to the stack
    while(stack->rec_amount != 1){
        old_node = node;
        if(node.next != -1)
            node = stack->r_stack[node.next];

        jump_to_record(bin_file, old_node.rrn, 0);
        fwrite("1", 1, sizeof(char), bin_file);
        fwrite(&node.rrn, 1, sizeof(int), bin_file);

        stack->rec_amount--;
    }

    jump_to_record(bin_file, node.rrn, 0);
    fwrite("1", 1, sizeof(char), bin_file);
}

void print_stack(STACK* stack){
    int i = 0;

    NODE_stack node = stack->r_stack[stack->begin];

    while(node.next != -1){
        printf("%d | ", node.rrn);
        node = stack->r_stack[node.next];
    }

    printf("%d \n", node.rrn);
}

int remove_from_stack(STACK* stack){
    if(stack->rec_amount == 0){
        printf("pilha vazia\n");
        return -1;
    }

    stack->begin = stack->r_stack[stack->begin].next;
    return 1;
}

int return_stack_top(STACK* stack){  
    if(stack->begin != -1)
        return stack->r_stack[stack->begin].rrn;
    else
        return -1;
}

//-------------------------------LIST
struct node_list{
    long int BOS;
    int rec_size;
    int next;
};

struct list{
    int rec_amount;
    int begin;
    NODE_list* r_list;
};

LIST* create_list(int list_size){
    LIST* list = malloc(sizeof(LIST));

    list->rec_amount = 0;
    list->r_list     = malloc(list_size*sizeof(NODE_list));
    list->begin      = -1;
    return list; 
}

void free_list(LIST* list){
    if(list != NULL){
        if(list->r_list != NULL)
            free(list->r_list);
        
        free(list);
    }
}

/*  Adds an element to the list, sequentially */
void add_list(LIST* list, long int BOS, int rec_size, long int next_BOS){
    list->r_list[list->rec_amount].BOS = BOS;
    list->r_list[list->rec_amount].rec_size = rec_size;
    
    // Since they are already in the file in order, if there is a proxBOS,
    // it will be at the next position of the vector
    if(next_BOS != -1)
        list->r_list[list->rec_amount].next = list->rec_amount + 1;
    else
        list->r_list[list->rec_amount].next = -1;

    list->rec_amount++;
}

void read_list(FILE* bin_file, LIST* list){
    if(bin_file == NULL)
        return;

    fseek(bin_file, 1, SEEK_SET);

    long int BOS = 0;
    fread(&BOS, 1, sizeof(long int), bin_file);

    int rec_size = 0;
    long int next_BOS = 0;

    if(BOS != -1){
        // As long as there are removed records already saved in the file, the loop continues
        while(BOS != -1){
            // Searches the register data
            fseek(bin_file, BOS+1, SEEK_SET);
            fread(&rec_size, 1, sizeof(int), bin_file);
            fread(&next_BOS, 1, sizeof(long int), bin_file);

            //add to the list
            add_list(list, BOS, rec_size, next_BOS);

            BOS = next_BOS;
        }

        list->begin = 0;
    }
}

void add_sorted_to_list(LIST* list, long int BOS, int rec_size){
    NODE_list node_list;
    node_list.rec_size = rec_size;
    node_list.BOS      = BOS;
    node_list.next     = -1;
    // loop control
    int i = 0;

    int actual_node = list->begin;
    int last_node   = -1;
    
    // Loop to search for the optimal position to add the new record 
    // in the list sorted from rec_size
    while(actual_node != -1 && list->r_list[actual_node].rec_size > rec_size){
        last_node = actual_node;
        
        //next node in the list
        actual_node = list->r_list[actual_node].next;
        i++;
    }
    
    
    if(actual_node == list->begin){    //insertion at the beginning
        if(list->rec_amount != 0)
            node_list.next = list->begin;

        list->begin = list->rec_amount;
    }else if(actual_node == -1){       //insertion at the end
        list->r_list[last_node].next = list->rec_amount;
    }else{                             //insertion at the mid
        node_list.next = actual_node;
        list->r_list[last_node].next = list->rec_amount;
    }

    list->r_list[list->rec_amount] = node_list;
    (list->rec_amount)++;
}

void print_list(LIST* list){
    if(list->rec_amount == 0){
        printf("LISTA VAZIA\n");
        return;
    }

    int actual = list->begin;

    while (actual != -1){
        printf("%d [%ld] | ", list->r_list[actual].rec_size, list->r_list[actual].BOS);
        actual = list->r_list[actual].next;
    }
    printf("\n\n");
}

int write_list(FILE* bin_file, LIST* list){
    if(list->rec_amount == 0)
        return -1;
    
    fseek(bin_file, 1, SEEK_SET);
    // loop control
    NODE_list node = list->r_list[list->begin];

    //Updating the top
    fwrite(&node.BOS, 1, sizeof(long int), bin_file);

    //Adds all records removed from the list to the data file
    while(node.next != -1){
        fseek(bin_file, node.BOS, SEEK_SET);

        //marks as removed
        fwrite("1", 1, sizeof(char), bin_file);
        if(node.next != -1)
            node = list->r_list[node.next];
            
        //add nextBOS
        fseek(bin_file, sizeof(int), SEEK_CUR);  
        fwrite(&node.BOS, 1, sizeof(long int), bin_file);
    }

    fseek(bin_file, node.BOS, SEEK_SET);
    fwrite("1", 1, sizeof(char), bin_file);

    return 1;
}

int remove_from_list(LIST* list){
    if(list->rec_amount == 0){
        printf("lista vazia\n");
        return -1;
    }

    list->begin = list->r_list[list->begin].next;
    return 1;
}

long int return_list_top(LIST* list, int* size){
    if(list->begin == -1)
        return -1;
    *size = list->r_list[list->begin].rec_size;
    return list->r_list[list->begin].BOS;
}