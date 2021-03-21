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
    proc->RAM = (double*) calloc(2000, sizeof(double));
    assert(proc->RAM);
}

//------------------------------------------------------------------------------------------------------

void CPU_destruct (CPU* proc)
{
    free(proc->registers);
    proc->registers = nullptr;
    free(proc->RAM);
    proc->RAM = nullptr;
    destruct_stack(proc->stack_data);
    destruct_stack(proc->stack_call);
    free(proc->stack_data);
    free(proc->stack_call);
    proc->stack_data = nullptr;
    proc->stack_call = nullptr;
}

//------------------------------------------------------------------------------------------------------

size_t size_of_file (FILE* text)
{
    assert(text);
    fseek(text, 0, SEEK_END); 

    size_t size_of_file = ftell(text);
    assert(size_of_file);
    
    rewind(text);

    return size_of_file + 1;
}

//------------------------------------------------------------------------------------------------------

buff* reading_file (buff* buffer, char* name_of_file)
{
    assert(buffer);
    assert(name_of_file);

    FILE* input = fopen(name_of_file, "rb");
    assert(input && "Problems with opening the file");

    buffer->size = size_of_file (input);
    
    buffer->text = (unsigned char*) calloc(buffer->size, sizeof(unsigned char));
    assert(buffer->text);

    fread(buffer->text, sizeof(char), buffer->size, input);

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
    ASSERT_OK;

    proc->rip = buffer->size;
}

//------------------------------------------------------------------------------------------------------

void add (CPU* proc, buff* buffer)
{
    ASSERT_OK;

    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    push_stack(proc->stack_data, a + b);
}

//------------------------------------------------------------------------------------------------------

void sub (CPU* proc, buff* buffer)
{
    ASSERT_OK;

    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    push_stack(proc->stack_data, b - a);
}

//------------------------------------------------------------------------------------------------------

void mul (CPU* proc, buff* buffer)
{
    ASSERT_OK;
    
    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    push_stack(proc->stack_data, a * b);
}

//------------------------------------------------------------------------------------------------------

void div (CPU* proc, buff* buffer)
{
    ASSERT_OK;

    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    push_stack(proc->stack_data, b / a);
}

//------------------------------------------------------------------------------------------------------

void push (CPU* proc, buff* buffer)
{
    ASSERT_OK;

    

    if (buffer->text[proc->rip] & (1 << 7))
    {
        switch ((buffer->text[proc->rip] & (7 << 4)) >> 4)
        {
            case 1:                                 // only register
                push_stack (proc->stack_data, proc->registers[(buffer->text[proc->rip] & (3 << 2)) >> 2]);
                break;
            case 2:                                 // only number
                push_stack (proc->stack_data, (*(double*)(buffer->text + proc->rip + 1)));
                proc->rip += sizeof(double);
                break;
            case 5:                                 // RAM and register
                push_stack (proc->stack_data, proc->RAM[(int)proc->registers[(buffer->text[proc->rip] & (3 << 2)) >> 2]]);
                break;
            case 6:                                 // RAM and number
                push_stack (proc->stack_data, proc->RAM[(int)(*(double*)(buffer->text + proc->rip + 1))]);
                proc->rip += sizeof(double);
                break;
            case 7:                                 // RAM, register and number
                if(buffer->text[proc->rip] & (1 << 1))
                {
                    push_stack (proc->stack_data, proc->RAM[(int)proc->registers[(buffer->text[proc->rip] & (3 << 2)) >> 2] + (int)(*(double*)(buffer->text + proc->rip + 1))]);
                }
                else
                {
                    push_stack (proc->stack_data, proc->RAM[(int)proc->registers[(buffer->text[proc->rip] & (3 << 2)) >> 2] - (int)(*(double*)(buffer->text + proc->rip + 1))]);
                }
                proc->rip += sizeof(double);
                break;
            default:
                printf("NO, NO, THAT'S NOT TRUE. IT'S IMPOSSIBLE PUSH!!!\n");
        }
    }
    else
    {
        printf("error: push without arguments\n");
    }
    proc->rip++;
}

//------------------------------------------------------------------------------------------------------

void pop (CPU* proc, buff* buffer)
{
    ASSERT_OK;

    double a = 0;
    int error = pop_stack (proc->stack_data, &a);
    assert(!error);

    if (buffer->text[proc->rip] & (1 << 7))
    {
        switch ((buffer->text[proc->rip] & (7 << 4)) >> 4)
        {
            case 1:                                 // only register
                proc->registers[(buffer->text[proc->rip] & (3 << 2)) >> 2] = a;
                break;
            case 5:                                 // RAM and register
                proc->RAM[(int)proc->registers[(buffer->text[proc->rip] & (3 << 2)) >> 2]] = a;
                break;
            case 6:                                 // RAM and number
                proc->RAM[(int)(*(double*)(buffer->text + proc->rip + 1))] = a;
                proc->rip += sizeof(double);
                break;
            case 7:                                 // RAM, register and number
                proc->RAM[(int)(*(double*)(buffer->text + proc->rip + 1)) + (int)proc->registers[(buffer->text[proc->rip] & (3 << 2)) >> 2]] = a;
                proc->rip += sizeof(double);
                break;
            default:
                printf("NO, NO, THAT'S NOT TRUE. IT'S IMPOSSIBLE POP!!!\n");
        }
    }
    
    proc->rip++;

}

//------------------------------------------------------------------------------------------------------

void pow (CPU* proc, buff* buffer)
{
    ASSERT_OK;

    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    push_stack(proc->stack_data, pow(a, b));
}

//------------------------------------------------------------------------------------------------------

void fsqrt (CPU* proc, buff* buffer)
{
    ASSERT_OK;

    double a = 0;

    pop_stack(proc->stack_data, &a);

    push_stack(proc->stack_data, sqrt(a));
}

//------------------------------------------------------------------------------------------------------

void out (CPU* proc, buff* buffer)
{
    ASSERT_OK;

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
    ASSERT_OK;

    double a = 0;

    scanf("%lg", &a);

    push_stack(proc->stack_data, a);
}

//------------------------------------------------------------------------------------------------------

void jmp (CPU* proc, buff* buffer)
{
    ASSERT_OK;

    proc->rip = *((double*)(buffer->text + proc->rip));
}

//------------------------------------------------------------------------------------------------------

void jb (CPU* proc, buff* buffer)
{
    ASSERT_OK;

    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    if(b < a)
    {
        proc->rip = *((double*)(buffer->text + proc->rip));
    }
    else
        proc->rip += sizeof(double);

}

//------------------------------------------------------------------------------------------------------

void jbe (CPU* proc, buff* buffer)
{
    ASSERT_OK;

    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    if(b <= a)
    {
        proc->rip = *((double*)(buffer->text + proc->rip));
    }
    else
        proc->rip += sizeof(double);

}

//------------------------------------------------------------------------------------------------------

void ja (CPU* proc, buff* buffer)
{
    ASSERT_OK;

    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    if(b > a)
    {
        proc->rip = *((double*)(buffer->text + proc->rip));
    }
    else
        proc->rip += sizeof(double);

}

//------------------------------------------------------------------------------------------------------

void jae (CPU* proc, buff* buffer)
{
    ASSERT_OK;

    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    if(b >= a)
    {
        proc->rip = *((double*)(buffer->text + proc->rip));
    }
    else
        proc->rip += sizeof(double);

}

//------------------------------------------------------------------------------------------------------

void je (CPU* proc, buff* buffer)
{
    ASSERT_OK;

    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    if(b == a)
    {
        proc->rip = *((double*)(buffer->text + proc->rip));
    }
    else
        proc->rip += sizeof(double);

}

//------------------------------------------------------------------------------------------------------

void jne (CPU* proc, buff* buffer)
{
    ASSERT_OK;

    double a = 0, b = 0;

    pop_stack(proc->stack_data, &a);
    pop_stack(proc->stack_data, &b);

    if(b != a)
    {
        proc->rip = *((double*)(buffer->text + proc->rip));
    }
    else
        proc->rip += sizeof(double);

}

//------------------------------------------------------------------------------------------------------

void call (CPU* proc, buff* buffer)
{
    ASSERT_OK;

    push_stack(proc->stack_call, (double)(proc->rip + sizeof(double)));
    proc->rip = (unsigned)*((double*)(buffer->text + proc->rip));
}

//------------------------------------------------------------------------------------------------------

void nop (CPU* proc, buff* buffer)
{
    ASSERT_OK;
    printf("WHY AM I EXIST???\n");
}

//------------------------------------------------------------------------------------------------------

void ret (CPU* proc, buff* buffer)
{
    ASSERT_OK;

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
