#ifndef SAS_STACK_H
#define SAS_STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>

//===============================================================================================

#define BEGIN_CAPACITY 2
#define CANARY 0xDABADEDABADADABA
#define CANARY1 0xBBAADDDDEEAADDFF

typedef double type_of_stack;

typedef struct
{
    unsigned long long canary1;
    unsigned long long hash;
    void* data;
    unsigned size;
    unsigned capacity;
    unsigned long long canary2;
}Stack;

enum Stack_error
{
    no_errors                       = 0,
    stack_nullptr                   = 1,
    bad_size                        = 2,
    no_free_memory                  = 3,
    stack_is_empty                  = 4,
    stack_canary_left_in_danger     = 5,
    stack_canary_right_in_danger    = 6,
    data_canary_left_in_danger      = 7,
    data_canary_right_in_danger     = 8,
    hash_error                      = 9,
};

//===============================================================================================

void fill_poison (Stack* stack, unsigned prev_capacity);

void* poison_realloc (Stack* stack, unsigned prev_capacity, unsigned capacity);

enum Stack_error construct_stack (Stack* stack);

enum Stack_error verificator (Stack* stack);

enum Stack_error push_stack (Stack* stack, type_of_stack value);

enum Stack_error pop_stack (Stack* stack, type_of_stack* value);

void dump_stack (Stack* stack);

void destruct_stack (Stack* stack);

void test_stack (Stack* stack);

unsigned long long make_hash (Stack* stack);

unsigned long long ROL (unsigned long long value);

unsigned long long ROR (unsigned long long value);

#endif
