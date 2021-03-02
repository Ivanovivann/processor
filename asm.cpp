#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

//------------------------------------------------------------------------------------------------------

typedef struct{
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
    
void filling_buffer_without_tabs_and_other_symbols(char* intermediate_buffer, buff* buffer);

void assembling (buff* buffer);

void filling_labels (char* begin_buf, buff* buffer, label* labels);

void filling_commands(char* begin_buf, buff* buffer, label* labels, double* commands, unsigned count_labels);

void one_argument_handler (double* commands, buff* buffer, int command_number, unsigned* address, label* labels, unsigned count_labels);

void no_arguments_handler (double* commands, buff* buffer, int command_number, unsigned* address, label* labels, unsigned count_labels);

void print_in_file (double* arr_of_commands, int size);

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

    assembling (buffer);

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

    char* intermediate_buffer = (char*) calloc (size, sizeof(char));
    buffer->text = (char*) calloc (size, sizeof(char));
    buffer->size = size;
    assert(buffer->text);
    assert(intermediate_buffer);

    fread(intermediate_buffer, sizeof(char), size, input);

    fclose(input);

    filling_buffer_without_tabs_and_other_symbols(intermediate_buffer, buffer);

    free(intermediate_buffer);

    return buffer;
}

//------------------------------------------------------------------------------------------------------

void filling_buffer_without_tabs_and_other_symbols(char* intermediate_buffer, buff* buffer)
{
    int current_size = 1;
    int i = 0;

    while (intermediate_buffer[i] == 9 || intermediate_buffer[i] == ' ')
        i++;

    for(; i < buffer->size; i++)
    {
        if (intermediate_buffer[i] == 9 || intermediate_buffer[i] == '\r') //TAB, \r 
        {
            continue;
        }
        if ((intermediate_buffer[i] == ' ' || intermediate_buffer[i] == '\n') && buffer->text[current_size - 1] == '\n')
        {
            continue;
        }
        buffer->text[current_size] = intermediate_buffer[i];
        current_size++;
    }
    buffer->text = (char*) realloc(buffer->text, (current_size + 1) * sizeof(char));
    buffer->size = current_size + 1;
    buffer->text[current_size] = '\n';
}

//------------------------------------------------------------------------------------------------------

void one_argument_handler (double* commands, buff* buffer, int command_number, unsigned* address, label* labels, unsigned count_labels)
{
    buffer->text += strlen(strtok(buffer->text, " ")) + 1;
    char* token = strtok(buffer->text, "\n");

    if (command_number == 5)
    {
        if (isalpha(token[0]))
        {
            commands[(*address)++] = 51;
            commands[(*address)++] = token[0] - 'a';
        }
        else
        {
            commands[(*address)++] = 50;
            commands[(*address)++] = atof(token);
        }
        buffer->text += strlen(token);
        return;
    }

    if ((int) (command_number / 10) == 7)
    {
        commands[(*address)++] = command_number;
        commands[(*address)++] = -1;
        for (int i = 0; i < count_labels; i++)
        {
            labels[i].name[strlen(labels[i].name) - 1] = 0;
            if (labels[i].name && !strcmp(token, labels[i].name))
            {
                commands[(*address) - 1] = labels[i].address;
            }
            labels[i].name[strlen(labels[i].name)] = ':';
        }
        buffer->text += strlen(token);
        return;
    }

    commands[(*address)++] = command_number;
    commands[(*address)++] = atof(token);

    buffer->text += strlen(token);   
}

//------------------------------------------------------------------------------------------------------

void no_arguments_handler (double* commands, buff* buffer, int command_number, unsigned* address, label* labels, unsigned count_labels)
{
    if (command_number == 6)
    {
        buffer->text += strlen(strtok(buffer->text, " ")) + 1;
        if (buffer->text[1] == 'x')
        {
            commands[(*address)++] = 61;
            commands[(*address)++] = buffer->text[0] - 'a';
        }
        else
        {
            buffer->text -= strlen(strtok(buffer->text, " "));
            commands[(*address)++] = 60;
        }
        
        buffer->text += strlen(strtok(buffer->text, "\n"));
        return;
    }

    buffer->text += strlen(strtok(buffer->text, "\n"));
    commands[(*address)++] = command_number;
}

//------------------------------------------------------------------------------------------------------

void assembling (buff* buffer)
{
    char* token = buffer->text;
    char* begin_buf = buffer->text;

    unsigned count_labels = 0;
    unsigned count_commands = 0;

    buffer->text++;

    for (int i = 0; i < buffer->size - 1; i++)
    {
        if (buffer->text[i] == ':')
            count_labels++;
        if (buffer->text[i] == '\n' || buffer->text[i] == ' ')
            count_commands++;
    }

    label* labels = (label*) calloc (count_labels, sizeof(label));
    double* commands = (double*) calloc(count_commands, sizeof(double));

    filling_labels(begin_buf, buffer, labels);

    filling_commands(begin_buf, buffer, labels, commands, count_labels);

    print_in_file(commands, count_commands - count_labels);

    buffer->text = begin_buf;
    free(labels);
    free(commands);

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
        if (token[strlen(token) - 1] == ':')
        {
            labels[current_label_number].name = token;
            labels[current_label_number].address = current_address;
            current_label_number++;
            buffer->text += strlen(token);
        }
        else
        {
            for (int i = 0; i < strlen(token); i++)
            {
                if (token[i] == ' ')
                    current_address++;
            }
            current_address++;

            buffer->text += strlen(token);
        }
        buffer->text++;
    }
    buffer->text = begin_buf + 1;
}

//------------------------------------------------------------------------------------------------------

void filling_commands(char* begin_buf, buff* buffer, label* labels, double* commands, unsigned count_labels)
{
    char* token = NULL;
    int current_line = 1;
    unsigned current_address = 0;
    int current_label_number = 0;
    FILE* log = fopen("log_asm.txt", "w");

    while (begin_buf + buffer->size - buffer->text - 1 > 0 && (token = strtok(buffer->text, "\n")) != NULL)
    {
        int flag = 0;
        #define CPU(name_of_command, name_code_of_command, code_of_command, in_handler, out_handler, cpu_func)  \
        if (!strcmp(token, name_of_command) || !strcmp(strtok(token, " "), name_of_command))                    \
        {                                                                                                       \
            in_handler(commands, buffer, code_of_command, &current_address, labels, count_labels);              \
            flag++;                                                                                             \
        }
        #include "commands.h"
        #undef CPU
        
        if (token[strlen(token) - 1] == ':')
        {
            flag++;
            buffer->text += strlen(token);
        }
        
        if (!flag)
        {
            printf("error on line %d\n", current_line);
            fprintf(log, "error on line %d\n", current_line);
        }
        current_line++;

        buffer->text++;
    }
    fclose(log);
}

//------------------------------------------------------------------------------------------------------

void print_in_file (double* arr_of_commands, int size)
{
    assert(arr_of_commands);
    FILE* output = fopen("asm_out.txt", "w");
    
    assert(output);

    for (int i = 0; i < size; i++)
    {
        fprintf(output, "%lg\n", arr_of_commands[i]);
    }

    fclose(output);
}

//------------------------------------------------------------------------------------------------------

void free_buffer (buff* buffer)
{
    free(buffer->text);
    free(buffer);
}
