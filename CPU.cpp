#include "CPU.h"

//------------------------------------------------FUNCTIONS---------------------------------------------

void CPU_construct (CPU* proc)
{
    proc->stack_data = (Stack*) calloc(1, sizeof(Stack));
    proc->stack_call = (Stack*) calloc(1, sizeof(Stack));
    construct_stack(proc->stack_data);
    construct_stack(proc->stack_call);
    proc->registers = (double*) calloc(4, sizeof(double));
}

//------------------------------------------------------------------------------------------------------

void CPU_destruct (CPU* proc)
{
    free(proc->registers);
    destruct_stack(proc->stack_data);
    destruct_stack(proc->stack_call);
    free(proc->stack_data);
    free(proc->stack_call);
}

//------------------------------------------------------------------------------------------------------

size_t count_of_commands (FILE* text)
{
    char c = 0;
    size_t counter = 0;

    while ((c = getc(text)) != EOF)
    {
        if (c == '\n' || c == ' ')
            counter++;
    }
    rewind(text);
    return counter;
}

//------------------------------------------------------------------------------------------------------

buff* reading_file (buff* buffer, char* name_of_file)
{
    assert(name_of_file);

    FILE* input = fopen(name_of_file, "r");
    assert(input && "Problems with opening the file");

    buffer->size = count_of_commands(input);

    buffer->text = (double*) calloc(buffer->size, sizeof(double));

    for (int i = 0; i < buffer->size; i++)
    {
        fscanf(input, "%lg", buffer->text + i);
    }

    fclose(input);

    return buffer;
}

//------------------------------------------------------------------------------------------------------

void processor_doing_brrr (CPU* proc, buff* buffer)
{
    for (proc->rip = 0; proc->rip < buffer->size;)
    {
        int cpu_counter = buffer->text[proc->rip++];
        switch (cpu_counter)
        {
            #define CPU(name_of_command, name_code_of_command, code_of_command, in_handler, out_handler, cpu_func)      \
            case code_of_command:                                                                                       \
                cpu_func(proc, buffer);                                                                                 \
                break;

            #include "commands.h"
            #undef CPU
        }
    }
}

//------------------------------------------------------------------------------------------------------

void hlt (CPU* proc, buff* buffer)
{
    proc->rip = buffer->size;
}

//------------------------------------------------------------------------------------------------------

void add (CPU* proc, buff* buffer)
{
    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    push_stack(proc->stack_data, a + b);
}

//------------------------------------------------------------------------------------------------------

void sub (CPU* proc, buff* buffer)
{
    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    push_stack(proc->stack_data, b - a);
}

//------------------------------------------------------------------------------------------------------

void mul (CPU* proc, buff* buffer)
{
    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    push_stack(proc->stack_data, a * b);
}

//------------------------------------------------------------------------------------------------------

void div (CPU* proc, buff* buffer)
{
    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    push_stack(proc->stack_data, b / a);
}

//------------------------------------------------------------------------------------------------------

void classic_pop (CPU* proc, buff* buffer)
{
    double a = 0;
    pop_stack(proc->stack_data, &a);
}

//------------------------------------------------------------------------------------------------------

void reg_pop (CPU* proc, buff* buffer)
{
    double a = 0;
    pop_stack (proc->stack_data, &a);
    proc->registers[(int) buffer->text[proc->rip++]] = a;
}

//------------------------------------------------------------------------------------------------------

void number_push (CPU* proc, buff* buffer)
{
    push_stack (proc->stack_data, buffer->text[proc->rip++]);
}

//------------------------------------------------------------------------------------------------------

void reg_push (CPU* proc, buff* buffer)
{
    push_stack (proc->stack_data, proc->registers[(int) buffer->text[proc->rip++]]);
}

//------------------------------------------------------------------------------------------------------

void push (CPU* proc, buff* buffer)
{
    printf("error of asm push\n");
}

//------------------------------------------------------------------------------------------------------

void pop (CPU* proc, buff* buffer)
{
    printf("error of asm pop\n");
}

//------------------------------------------------------------------------------------------------------

void pow (CPU* proc, buff* buffer)
{
    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    push_stack(proc->stack_data, pow(a, b));
}

//------------------------------------------------------------------------------------------------------

void fsqrt (CPU* proc, buff* buffer)
{
    double a = 0;

    pop_stack(proc->stack_data, &a);

    push_stack(proc->stack_data, sqrt(a));
}

//------------------------------------------------------------------------------------------------------

void out (CPU* proc, buff* buffer)
{
    double a = 0;
    if (proc->stack_data->size)
    {
        pop_stack(proc->stack_data, &a);
        printf("%lg\n", a);
    }
    else
    {
        printf("stack is empty\n");
    }
}

//------------------------------------------------------------------------------------------------------

void in (CPU* proc, buff* buffer)
{
    double a = 0;

    scanf("%lg", &a);

    push_stack(proc->stack_data, a);
}

//------------------------------------------------------------------------------------------------------

void jmp (CPU* proc, buff* buffer)
{
    proc->rip = buffer->text[proc->rip];
}

//------------------------------------------------------------------------------------------------------

void jb (CPU* proc, buff* buffer)
{
    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    if(b < a)
    {
        proc->rip = buffer->text[proc->rip];
    }
    else
        proc->rip++;
}

//------------------------------------------------------------------------------------------------------

void jbe (CPU* proc, buff* buffer)
{
    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    if(b <= a)
    {
        proc->rip = buffer->text[proc->rip];
    }
    else
        proc->rip++;

}

//------------------------------------------------------------------------------------------------------

void ja (CPU* proc, buff* buffer)
{
    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    if(b > a)
    {
        proc->rip = buffer->text[proc->rip];
    }
    else
        proc->rip++;

}

//------------------------------------------------------------------------------------------------------

void jae (CPU* proc, buff* buffer)
{
    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    if(b >= a)
    {
        proc->rip = buffer->text[proc->rip];
    }
    else
        proc->rip++;

}

//------------------------------------------------------------------------------------------------------

void je (CPU* proc, buff* buffer)
{
    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    if(b == a)
    {
        proc->rip = buffer->text[proc->rip];
    }
    else
         proc->rip++;

}

//------------------------------------------------------------------------------------------------------

void jne (CPU* proc, buff* buffer)
{
    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    if(b != a)
    {
        proc->rip = buffer->text[proc->rip];
    }
    else
        proc->rip++;

}

//------------------------------------------------------------------------------------------------------

void call (CPU* proc, buff* buffer)
{
    push_stack(proc->stack_call, proc->rip + 1);
    proc->rip = buffer->text[proc->rip];
}

//------------------------------------------------------------------------------------------------------

void ret (CPU* proc, buff* buffer)
{
    double a = 0;

    pop_stack(proc->stack_call, &a);
    proc->rip = (int) a;
}

//------------------------------------------------------------------------------------------------------

void free_buffer (buff* buffer)
{
    free(buffer->text);
    free(buffer);
}