#include "CPU.h"

//------------------------------------------------FUNCTIONS---------------------------------------------

void CPU_construct (CPU* proc)
{
    assert(proc);
    proc->stack_data = (Stack*) calloc(1, sizeof(Stack));
    assert(proc->stack_data);
    proc->stack_call = (Stack*) calloc(1, sizeof(Stack));
    assert(proc->stack_call);
    construct_stack(proc->stack_data);
    construct_stack(proc->stack_call);
    proc->registers = (double*) calloc(4, sizeof(double));
    assert(proc->registers);
}

//------------------------------------------------------------------------------------------------------

void CPU_destruct (CPU* proc)
{
    free(proc->registers);
    proc->registers = nullptr;
    destruct_stack(proc->stack_data);
    destruct_stack(proc->stack_call);
    free(proc->stack_data);
    free(proc->stack_call);
    proc->stack_data = nullptr;
    proc->stack_call = nullptr;
}

//------------------------------------------------------------------------------------------------------

buff* reading_file (buff* buffer, char* name_of_file)
{
    assert(buffer);
    assert(name_of_file);

    FILE* input = fopen(name_of_file, "r");
    assert(input && "Problems with opening the file");

    char argument[100];
    double version_of_commands = 0;

    fscanf(input, "%s", argument);
    assert(!strcmp(argument, "iwanou_222_brrr!"));
    
    fscanf(input, "%lg", &version_of_commands);
    assert(version_of_commands == 2);

    fscanf(input, "%ld", &buffer->size);
    assert(buffer->size);
    
    buffer->text = (double*) calloc(buffer->size, sizeof(double));
    assert(buffer->text);

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
    assert(proc);
    assert(buffer);
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
    assert(proc);
    assert(buffer);
    proc->rip = buffer->size;
}

//------------------------------------------------------------------------------------------------------

void add (CPU* proc, buff* buffer)
{
    assert(proc);
    assert(buffer);
    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    push_stack(proc->stack_data, a + b);
}

//------------------------------------------------------------------------------------------------------

void sub (CPU* proc, buff* buffer)
{
    assert(proc);
    assert(buffer);
    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    push_stack(proc->stack_data, b - a);
}

//------------------------------------------------------------------------------------------------------

void mul (CPU* proc, buff* buffer)
{
    assert(proc);
    assert(buffer);
    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    push_stack(proc->stack_data, a * b);
}

//------------------------------------------------------------------------------------------------------

void div (CPU* proc, buff* buffer)
{
    assert(proc);
    assert(buffer);
    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    push_stack(proc->stack_data, b / a);
}

//------------------------------------------------------------------------------------------------------

void classic_pop (CPU* proc, buff* buffer)
{
    assert(proc);
    assert(buffer);
    double a = 0;
    pop_stack(proc->stack_data, &a);
}

//------------------------------------------------------------------------------------------------------

void reg_pop (CPU* proc, buff* buffer)
{
    assert(proc);
    assert(buffer);
    double a = 0;
    pop_stack (proc->stack_data, &a);
    proc->registers[(int) buffer->text[proc->rip++]] = a;
}

//------------------------------------------------------------------------------------------------------

void number_push (CPU* proc, buff* buffer)
{
    assert(proc);
    assert(buffer);
    push_stack (proc->stack_data, buffer->text[proc->rip++]);
}

//------------------------------------------------------------------------------------------------------

void reg_push (CPU* proc, buff* buffer)
{
    assert(proc);
    assert(buffer);
    push_stack (proc->stack_data, proc->registers[(int) buffer->text[proc->rip++]]);
}

//------------------------------------------------------------------------------------------------------

void push (CPU* proc, buff* buffer)
{
    assert(proc);
    assert(buffer);
    printf("error of asm push\n");
}

//------------------------------------------------------------------------------------------------------

void pop (CPU* proc, buff* buffer)
{
    assert(proc);
    assert(buffer);
    printf("error of asm pop\n");
}

//------------------------------------------------------------------------------------------------------

void pow (CPU* proc, buff* buffer)
{
    assert(proc);
    assert(buffer);
    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    push_stack(proc->stack_data, pow(a, b));
}

//------------------------------------------------------------------------------------------------------

void fsqrt (CPU* proc, buff* buffer)
{
    assert(proc);
    assert(buffer);
    double a = 0;

    pop_stack(proc->stack_data, &a);

    push_stack(proc->stack_data, sqrt(a));
}

//------------------------------------------------------------------------------------------------------

void out (CPU* proc, buff* buffer)
{
    assert(proc);
    assert(buffer);
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
    assert(proc);
    assert(buffer);
    double a = 0;

    scanf("%lg", &a);

    push_stack(proc->stack_data, a);
}

//------------------------------------------------------------------------------------------------------

void jmp (CPU* proc, buff* buffer)
{
    assert(proc);
    assert(buffer);
    proc->rip = buffer->text[proc->rip];
}

//------------------------------------------------------------------------------------------------------

void jb (CPU* proc, buff* buffer)
{
    assert(proc);
    assert(buffer);
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
    assert(proc);
    assert(buffer);
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
    assert(proc);
    assert(buffer);
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
    assert(proc);
    assert(buffer);
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
    assert(proc);
    assert(buffer);
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
    assert(proc);
    assert(buffer);
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
    assert(proc);
    assert(buffer);
    push_stack(proc->stack_call, proc->rip + 1);
    proc->rip = buffer->text[proc->rip];
}

//------------------------------------------------------------------------------------------------------

void ret (CPU* proc, buff* buffer)
{
    assert(proc);
    assert(buffer);
    double a = 0;

    pop_stack(proc->stack_call, &a);
    proc->rip = (int) a;
}

//------------------------------------------------------------------------------------------------------

void free_buffer (buff* buffer)
{
    assert(buffer);
    free(buffer->text);
    buffer->text = nullptr;
}
