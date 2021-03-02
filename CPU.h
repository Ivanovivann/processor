#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>
#include "stack.h"

typedef struct buff{
    double* text;
    size_t size;
}buff;

typedef struct
{
    Stack* stack_data;
    Stack* stack_call;
    unsigned rip;
    double* registers;
}CPU;

//------------------------------------------------------------------------------------------------------

void CPU_construct (CPU* proc);

void CPU_destruct (CPU* proc);

size_t count_of_commands (FILE* text);

buff* reading_file (buff* text, char* name_of_file);

void processor_doing_brrr (CPU* proc, buff* buffer);

void hlt (CPU* proc, buff* buffer);

void add (CPU* proc, buff* buffer);

void sub (CPU* proc, buff* buffer);

void mul (CPU* proc, buff* buffer);

void div (CPU* proc, buff* buffer);

void classic_pop (CPU* proc, buff* buffer);

void reg_pop (CPU* proc, buff* buffer);

void number_push (CPU* proc, buff* buffer);

void reg_push (CPU* proc, buff* buffer);

void push (CPU* proc, buff* buffer);

void pop (CPU* proc, buff* buffer);

void pow (CPU* proc, buff* buffer);

void fsqrt (CPU* proc, buff* buffer);

void out (CPU* proc, buff* buffer);

void in (CPU* proc, buff* buffer);

void jmp (CPU* proc, buff* buffer);

void jb (CPU* proc, buff* buffer);

void jbe (CPU* proc, buff* buffer);

void ja (CPU* proc, buff* buffer);

void jae (CPU* proc, buff* buffer);

void je (CPU* proc, buff* buffer);

void jne (CPU* proc, buff* buffer);

void call (CPU* proc, buff* buffer);

void ret (CPU* proc, buff* buffer);

void free_buffer (buff* buffer);

#endif