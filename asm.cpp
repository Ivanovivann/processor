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

void assembling (buff* buffer);

void one_argument_handler (double* commands, buff* buffer, int command_number, unsigned* address, label* labels, unsigned count_labels);

void no_arguments_handler (double* commands, buff* buffer, int command_number, unsigned* address, label* labels, unsigned count_labels);

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

    buffer->text = (char*) calloc (size, sizeof(char));
    buffer->size = size;
    assert(buffer->text);

    fread(buffer->text, sizeof(char), size, input);

    fclose(input);

    return buffer;
}

//------------------------------------------------------------------------------------------------------

void one_argument_handler (double* commands, buff* buffer, int command_number, unsigned* address, label* labels, unsigned count_labels)
{
    buffer->text += strlen(strtok(buffer->text, " ")) + 1;
    char* token = strtok(buffer->text, "\r");

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
        buffer->text += strlen(token) + 1;
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
        buffer->text += strlen(token) + 1;
        return;
    }

    commands[(*address)++] = command_number;
    commands[(*address)++] = atof(token);

    buffer->text += strlen(token) + 1;   
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
            buffer->text -= strlen(strtok(buffer->text, " ")) + 1;
            commands[(*address)++] = 60;
        }
        
        buffer->text += strlen(strtok(buffer->text, "\r")) + 1;
        return;
    }

    buffer->text += strlen(strtok(buffer->text, "\r")) + 1;
    commands[(*address)++] = command_number;
}

//------------------------------------------------------------------------------------------------------

void assembling (buff* buffer)
{
    char* token = buffer->text;
    char* begin_buf = buffer->text;

    unsigned current_address = 0;
    int current_label_number = 0;
    unsigned count_labels = 0;
    unsigned count_commands = 0;

    for (int i = 0; i < buffer->size; i++)
    {
        if (buffer->text[i] == ':')
            count_labels++;
        if (buffer->text[i] == '\r' || buffer->text[i] == ' ')
            count_commands++;
    }

    label* labels = (label*) calloc (count_labels, sizeof(label));
    double* commands = (double*) calloc(count_commands, sizeof(double));

    while (begin_buf + buffer->size - buffer->text > 0 && (token = strtok(buffer->text, "\r")) != NULL)
    {
        if (token[strlen(token) - 1] == ':')
        {
            labels[current_label_number].name = token;
            labels[current_label_number].address = current_address;
            current_label_number++;
            buffer->text += strlen(token) + 1;
        }
        else
        {
            for (int i = 0; i < strlen(token); i++)
            {
                if (token[i] == ' ')
                    current_address++;
            }
            current_address++;

            buffer->text += strlen(token) + 1;
        }
        buffer->text++;
    }
    buffer->text = begin_buf;

    FILE* log = fopen("log_asm.txt", "w");
    int current_line = 1;
    current_address = 0;
    current_label_number = 0;
    
    while (begin_buf + buffer->size - buffer->text > 0 && (token = strtok(buffer->text, "\r")) != NULL)
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
            buffer->text += strlen(token) + 1;
        }
        
        if (!flag)
        {
            printf("error on line %d\n", current_line);
            fprintf(log, "error on line %d\n", current_line);
        }
        current_line++;

        buffer->text++;
    }

    FILE* output = fopen("asm_out.txt", "w");
    for(int i = 0; i <= count_commands - count_labels; i++)
    {
        fprintf(output, "%lg\n", commands[i]);

    }

    fclose(output);
    fclose(log);

    buffer->text = begin_buf;
    free(labels);
    free(commands);

    return;
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

