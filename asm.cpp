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
    
void filling_buffer_without_tabs_and_other_symbols (char* intermediate_buffer, buff* buffer);

void assembling (buff* buffer);

int filling_commands(char* begin_buf, buff* buffer, label* labels, unsigned char* commands, unsigned count_labels);

void no_arguments_handler (unsigned char* commands, buff* buffer, int command_number, unsigned* address, label* labels, unsigned count_labels, char** tmp_labels);

void push_or_pop (unsigned char* commands, buff* buffer, int command_number, unsigned* address, label* labels, unsigned count_labels, char** tmp_labels);

void jumps (unsigned char* commands, buff* buffer, int command_number, unsigned* address, label* labels, unsigned count_labels, char** tmp_labels);

void control_filling (label* labels, char** tmp_labels, unsigned char* commands, int size_commands, int count_labels);

void free_tmp_labels (char** tmp_labels);

void print_in_file (unsigned char* arr_of_commands, int size);

void free_buffer (buff* buffer);

//------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    buff buffer = {};
    
    if (argc < 2)
    {
        printf("Usage: <%s> file-to-open\n", argv[0]);
        return -1;
    }

    assembling (reading_file (&buffer, argv[1]));

    free_buffer(&buffer);

    return 0;
}

//------------------------------------------------FUNCTIONS---------------------------------------------

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
    unsigned char* commands = (unsigned char*) calloc(count_commands * 10, sizeof(char));         //10 потому что максимум push съест 10 байт в бинарнике

    int size = filling_commands(begin_buf, buffer, labels, commands, count_labels);

    print_in_file(commands, size);

    buffer->text = begin_buf;
    free(labels);
    free(commands);

    return;
}

//------------------------------------------------------------------------------------------------------

void no_arguments_handler (unsigned char* commands, buff* buffer, int command_number, unsigned* address, label* labels, unsigned count_labels, char** tmp_labels)
{
    commands[(*address)++] = command_number;

    buffer->text += strlen(strtok(buffer->text, "\n"));

}

//------------------------------------------------------------------------------------------------------

void push_or_pop (unsigned char* commands, buff* buffer, int command_number, unsigned* address, label* labels, unsigned count_labels, char** tmp_labels)
{
    commands[(*address)++] = command_number;

    char* token = strtok(buffer->text, "\n");

    int len = strlen(token) + 1;

    if (token[len] == '[' || isdigit(token[len]) || ((token[len] =='r') && (token[len + 2] =='x')))  // 5 is a length of push + 1
    {
        buffer->text += strlen(token) + 1;

        token = strtok(buffer->text, "\n");
    }
    
    for (int i = 0; i < strlen(token); i++)
    {
        if (token[i] == '[')
        {
            commands[*(address)] |= 1 << 6;    
        }

        if (isdigit(token[i]) && !(commands[*(address)] & (1 << 5)))
        {
            commands[*(address)] |= 1 << 5;
            *((double*)(commands + (*address) + 1)) = atof(&(token[i]));
        }

        if (token[i] == 'r' && token[i + 2] == 'x')
        {
            commands[*(address)] |= 1 << 4;
            if(token[i + 1] - 'a' < 4)
            {
                commands[*(address)] |= (token[i + 1] - 'a') << 2;    
            }
            else
            {
                printf("undefined register\n");
            }
        }

        if (token[i] == '+')
        {
            commands[*(address)] |= (1 << 1);    
        }
    }

    if (commands[*(address)] != 0)
    {
        commands[*(address)] |= 1 << 7;
    }

    (*address)++;

    if (commands[(*address) - 1] & (1 << 5))
    {
        (*address) += sizeof(double);
    }

    buffer->text += strlen(token);
    
    return;
}

//------------------------------------------------------------------------------------------------------

void jumps (unsigned char* commands, buff* buffer, int command_number, unsigned* address, label* labels, unsigned count_labels, char** tmp_labels)
{
    buffer->text += strlen(strtok(buffer->text, " ")) + 1;
    char* token = strtok(buffer->text, "\n");

    commands[(*address)++] = command_number;
    *((int*)(commands + (*address))) = -1;

    for (int i = 0; i < count_labels; i++)
    {
        if(!labels[i].address)
        {
            break;
        }
        labels[i].name[strlen(labels[i].name) - 1] = 0;
        if (labels[i].name && !strcmp(token, labels[i].name))
        {
            *((int*)(commands + (*address))) = labels[i].address;
        }
        labels[i].name[strlen(labels[i].name)] = ':';
    }

    if(*((int*)(commands + (*address))) == -1)
    {
        int num = 0;
        for (num = 0; tmp_labels[num]; num++)
            ;

        tmp_labels[num] = (char*) calloc((strlen(token) + 1), sizeof(char));
        
        for (int i = 0; i < strlen(token); i++)
        {
            tmp_labels[num][i] = token[i];
        }
    }

    (*address) += 4;
    buffer->text += strlen(token);
    return;
}

//------------------------------------------------------------------------------------------------------

int filling_commands(char* begin_buf, buff* buffer, label* labels, unsigned char* commands, unsigned count_labels)
{
    char* token = NULL;
    int current_line = 1;
    unsigned current_address = 0;
    int current_label_number = 0;
    char** tmp_labels = (char**) calloc (1000, sizeof(char*));

    while (begin_buf + buffer->size - buffer->text - 1 > 0 && (token = strtok(buffer->text, "\n")) != NULL)
    {
        int flag = 0;
        #define CPU(name_of_command, name_code_of_command, code_of_command, in_handler, out_handler, cpu_func)  \
        if (!strcmp(token, name_of_command) || !strcmp(strtok(token, " "), name_of_command))                    \
        {                                                                                                       \
            in_handler(commands, buffer, code_of_command, &current_address, labels, count_labels, tmp_labels);  \
            flag++;                                                                                             \
        }
        #include "commands.h"
        #undef CPU
        
        if (token[strlen(token) - 1] == ':')
        {
            flag++;
            labels[current_label_number].name = token;
            labels[current_label_number].address = current_address;
            current_label_number++;

            buffer->text += strlen(token);
        }
        
        if (!flag)
        {
            printf("error on line %d\n", current_line);
        }
        current_line++;

        buffer->text++;
    }

    control_filling (labels, tmp_labels, commands, current_address, count_labels);

    free_tmp_labels (tmp_labels);

    return current_address;
}

//------------------------------------------------------------------------------------------------------

void control_filling (label* labels, char** tmp_labels, unsigned char* commands, int size_commands, int count_labels)
{
    int counter = 0;

    for (int i = 0; (i <= size_commands) && tmp_labels[counter]; i++)
    {
        if (commands[i] == 5 || commands[i] == 6)
        {
            i++;
            if (commands[i + 1] & (1 << 5))
            {
                i += sizeof(double);
            }
        }
        if ((int)(commands[i] / 10) == 7 && *((int*)(commands + i + 1)) == -1)
        {
            for (int j = 0; j < count_labels; j++)
            {
                labels[j].name[strlen(labels[j].name) - 1] = 0;

                if (!strcmp(tmp_labels[counter], labels[j].name))
                {
                    counter++;
                    *((int*)(commands + i + 1)) = labels[j].address;
                    break;
                }
                labels[j].name[strlen(labels[j].name)] = ':';
            }
            i += sizeof(int);
        }
    }
}
//------------------------------------------------------------------------------------------------------

void free_tmp_labels (char** tmp_labels)
{
    for (int i = 0; tmp_labels[i]; i++)
    {
        free(tmp_labels[i]);
    }

    free(tmp_labels);
}

//------------------------------------------------------------------------------------------------------

void print_in_file (unsigned char* arr_of_commands, int size)
{
    assert(arr_of_commands);
    assert(size);

    FILE* output = fopen("asm_out.txt", "wb");
    assert(output);

    // for (int i = 0; i < size; i++)
    // {
    //     fprintf(output, "%d\n", arr_of_commands[i]);
    // }
    
    fwrite(arr_of_commands, size, sizeof(char), output);

    fclose(output);
}

//------------------------------------------------------------------------------------------------------

void free_buffer (buff* buffer)
{
    assert(buffer);
    free(buffer->text);
    buffer->text = nullptr;
}
