#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

//------------------------------------------------------------------------------------------------------

typedef struct buff{
    char* text;
    size_t size;
}buff;

typedef struct{
    char* name;
    int address;
}label;

//------------------------------------------------------------------------------------------------------

size_t size_of_file (FILE* text);

buff* reading_file (buff* text, char* name_of_file);

void disassembling (buff* buffer);

void filling_labels (char* begin_buf, buff* buffer, label* labels);

void func_without_arguments (FILE* output, buff* buffer, const char* command_name, int counter);

void func_with_one_argument (FILE* output, buff* buffer, const char* command_name, int counter);

void free_buffer (buff* buffer);

//------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    buff* buffer = (buff*) calloc(1, sizeof(buff));
    
    if (argc < 2)
    {
        printf("Usage: <%s> file-to-open\n", argv[0]);
        return -1;
    }

    buffer = reading_file (buffer, argv[1]);

    disassembling (buffer);

    free_buffer(buffer);

    return 0;
}

//------------------------------------------------FUNCTIONS---------------------------------------------

size_t size_of_file (FILE* text)
{
    fseek(text, 0, SEEK_END); 

    size_t size_of_file = ftell(text);
    assert(size_of_file);
    
    rewind(text);

    return size_of_file + 1;
}

//------------------------------------------------------------------------------------------------------

buff* reading_file (buff* buffer, char* name_of_file)
{
    assert(name_of_file);

    FILE* input = fopen(name_of_file, "r");
    assert(input && "Problems with opening the file");

    size_t size = size_of_file(input);

    buffer->text = (char*) calloc (size, sizeof(char));
    buffer->size = size;
    assert(buffer->text);

    fread(buffer->text, sizeof(char), size, input);

    fclose(input);

    return buffer;
}

//------------------------------------------------------------------------------------------------------

void disassembling (buff* buffer)
{
    char* token = buffer->text;
    char* begin_buf = buffer->text;
    int counter = 0;
    FILE* output = fopen("disasm_out.txt", "w");
    label* labels = (label*) calloc (1, sizeof(label));
    
    filling_labels (begin_buf, buffer, labels);
    
    while (begin_buf + buffer->size - buffer->text > 0 && (token = strtok(buffer->text, "\n")) != NULL)
    {
        buffer->text += strlen(token) + 1;
        switch (atoi(token)) 
        {
            #define CPU(name_of_command, name_code_of_command, code_of_command, in_handler, out_handler, cpu_func)  \
            case code_of_command :                                                                                  \
                out_handler(output, buffer, name_of_command, counter);                                                       \
                break;                                        
            #include "commands.h"
            #undef CPU
        }
    }

    fclose(output);

    buffer->text = begin_buf;
    free(labels);

    return;
}

//------------------------------------------------------------------------------------------------------

void filling_labels (char* begin_buf, buff* buffer, label* labels)
{
    char* token = NULL;
    unsigned current_address = 0;
    unsigned current_label_number = 0;
    
    while (begin_buf + buffer->size - buffer->text - 1 > 0 && (token = strtok(buffer->text, "\n")) != NULL)
    {
        buffer->text++;
    }
    buffer->text = begin_buf;
}

//------------------------------------------------------------------------------------------------------

void func_without_arguments (FILE* output, buff* buffer, const char* command_name, int counter);
{
    ++counter;
    if (!strcmp("classic_pop", command_name))
    {
        fprintf(output, "pop\n");
        return;
    }
    fprintf(output, "%s\n", command_name);
}

//------------------------------------------------------------------------------------------------------

void func_with_one_argument (FILE* output, buff* buffer, const char* command_name, int counter);
{
    ++++counter;
    char* token = strtok(buffer->text, "\n");
    buffer->text += strlen(token) + 1;

    if(!strcmp("reg_pop", command_name))
    {
        //printf("pop reg = %s", token);
        fprintf(output, "pop %cx\n", token[0] + 'a' - '0');
        return;
    }
    if(!strcmp("reg_push", command_name))
    {
        //printf("push reg = %s", token);
        fprintf(output, "push %cx\n", token[0] + 'a'- '0');
        return;
    }
    if(!strcmp("number_push", command_name))
    {
        fprintf(output, "push %s\n", token);
        return;
    }
    fprintf(output, "%s %s\n", command_name, token);
}

//------------------------------------------------------------------------------------------------------

void free_buffer (buff* buffer)
{
    free(buffer->text);
    free(buffer);
}
