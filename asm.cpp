#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

//------------------------------------------------------------------------------------------------------

typedef struct{
    char* text;
    size_t size;
    char* begin_text;
}buff;

typedef struct{
    char* name;
    int address;
}label;

typedef struct{
    label* arr_labels;
    size_t count_labels;
    char** tmp_labels;
}all_about_labels;

//------------------------------------------------------------------------------------------------------

#define ASSERT_IN_FUNC  assert(commands);               \
                        assert(buffer);                 \
                        assert(labels);                 \
                        assert(labels->tmp_labels);     \
                        assert(labels->arr_labels);

//------------------------------------------------------------------------------------------------------

size_t size_of_file (FILE* text);

buff* reading_file (buff* text, char* name_of_file);
    
void filling_buffer_without_tabs_and_other_symbols (char* intermediate_buffer, buff* buffer);

void assembling (buff* buffer);

int filling_commands(buff* buffer, all_about_labels* labels, unsigned char* commands);

void no_arguments_handler (unsigned char* commands, buff* buffer, int command_number, unsigned* address, all_about_labels* labels);

void push_or_pop (unsigned char* commands, buff* buffer, int command_number, unsigned* address, all_about_labels* labels);

void jumps (unsigned char* commands, buff* buffer, int command_number, unsigned* address, all_about_labels* labels);

void control_filling (unsigned char* commands, int size_commands, all_about_labels* labels);

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

    while (intermediate_buffer[i] == 9 || intermediate_buffer[i] == ' ' || intermediate_buffer[i] == '\n')
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
    buffer->begin_text = buffer->text;
    buffer->size = current_size + 1;
    buffer->text[current_size] = '\n';
}

//------------------------------------------------------------------------------------------------------

void assembling (buff* buffer)
{
    assert(buffer);

    all_about_labels labels = {};
    char* token = buffer->text;

    unsigned count_commands = 0;

    buffer->text++;

    for (int i = 0; i < buffer->size - 1; i++)
    {
        if (buffer->text[i] == ':')
            (labels.count_labels)++;
        if (buffer->text[i] == '\n')
            count_commands++;
    }

    labels.arr_labels = (label*) calloc (labels.count_labels, sizeof(label));
    labels.tmp_labels = (char**) calloc (count_commands, sizeof(char*));
    unsigned char* commands = (unsigned char*) calloc(count_commands * 10, sizeof(char));         //10 потому что максимум push съест 10 байт в бинарнике

    int size = filling_commands(buffer, &labels, commands);

    print_in_file(commands, size);

    buffer->text = buffer->begin_text;
    free(labels.arr_labels);
    free(commands);

    return;
}

//------------------------------------------------------------------------------------------------------

void no_arguments_handler (unsigned char* commands, buff* buffer, int command_number, unsigned* address, all_about_labels* labels)
{
    ASSERT_IN_FUNC;

    commands[(*address)++] = command_number;

    buffer->text += strlen(strtok(buffer->text, "\n"));

}

//------------------------------------------------------------------------------------------------------

void push_or_pop (unsigned char* commands, buff* buffer, int command_number, unsigned* address, all_about_labels* labels)
{
    ASSERT_IN_FUNC;

    commands[(*address)++] = command_number;

    char* token = strtok(buffer->text, "\n");

    int len = strlen(token) + 1;

    if (token[len] == '[' || isdigit(token[len]) || token[len] == '-' || ((token[len] =='r') && (token[len + 2] =='x')))  // 5 is a length of push + 1
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

        if ((isdigit(token[i]) || (token[i] == '-' && isdigit(token[i + 1]))) && !(commands[*(address)] & (1 << 5)))
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

void jumps (unsigned char* commands, buff* buffer, int command_number, unsigned* address, all_about_labels* labels)
{
    ASSERT_IN_FUNC;

    buffer->text += strlen(strtok(buffer->text, " ")) + 1;
    char* token = strtok(buffer->text, "\n");

    commands[(*address)++] = command_number;
    *((double*)(commands + (*address))) = -1;

    for (int i = 0; i < labels->count_labels; i++)
    {
        if(!(labels->arr_labels)[i].address)
        {
            break;
        }
        (labels->arr_labels)[i].name[strlen((labels->arr_labels)[i].name) - 1] = 0;

        if ((labels->arr_labels)[i].name && !strcmp(token, (labels->arr_labels)[i].name))
        {
            *((double*)(commands + (*address))) = (labels->arr_labels)[i].address;
        }
        (labels->arr_labels)[i].name[strlen((labels->arr_labels)[i].name)] = ':';
    }

    if(*((double*)(commands + (*address))) == -1)
    {
        int num = 0;
        for (num = 0; labels->tmp_labels[num]; num++)
            ;

        labels->tmp_labels[num] = (char*) calloc((strlen(token) + 1), sizeof(char));
        
        for (int i = 0; i < strlen(token) && !isspace(token[i]); i++)
        {
            labels->tmp_labels[num][i] = token[i];
        }
    }

    (*address) += sizeof(double);
    buffer->text += strlen(token);
    return;
}

//------------------------------------------------------------------------------------------------------

int filling_commands(buff* buffer, all_about_labels* labels, unsigned char* commands)
{
    assert(buffer);
    assert(labels);
    assert(commands);

    char* token = NULL;
    int current_line = 1;
    unsigned current_address = 0;
    int current_label_number = 0;

    while (buffer->begin_text + buffer->size - buffer->text - 1 > 0 && (token = strtok(buffer->text, "\n")) != NULL)
    {
        int flag = 0;
        #define CPU(name_of_command, name_code_of_command, code_of_command, in_handler, out_handler, cpu_func)  \
        if (!strcmp(token, name_of_command) || !strcmp(strtok(token, " "), name_of_command))                    \
        {                                                                                                       \
            in_handler(commands, buffer, code_of_command, &current_address, labels);                            \
            flag++;                                                                                             \
        }
        #include "commands.h"
        #undef CPU
        
        if (token[strlen(token) - 1] == ':')
        {
            flag++;
            labels->arr_labels[current_label_number].name = token;
            labels->arr_labels[current_label_number].address = current_address;
            current_label_number++;

            buffer->text += strlen(token);
        }
        
        if (!flag)
        {
            printf("error on line %d || you write |%s|\n", current_line, token);
        }
        current_line++;

        buffer->text++;
    }

    control_filling (commands, current_address, labels);

    free_tmp_labels (labels->tmp_labels);

    return current_address;
}

//------------------------------------------------------------------------------------------------------

void control_filling (unsigned char* commands, int size_commands, all_about_labels* labels)
{
    assert(labels);
    assert(labels->arr_labels);
    assert(labels->tmp_labels);
    assert(commands);

    int counter = 0;

    for (int i = 0; (i <= size_commands) && labels->tmp_labels[counter]; i++)
    {
        if (commands[i] == 5 || commands[i] == 6) // push or pop
        {
            i++;
            if (commands[i] & (1 << 5))         // bit of number mod
            {
                i += sizeof(double);
            }
            continue;
        }
        if ((int)(commands[i] / 10) == 7 && *((double*)(commands + i + 1)) == -1)
        {
            int presence_of_label = 0;
            for (int j = 0; j < labels->count_labels; j++)
            {
                (labels->arr_labels)[j].name[strlen((labels->arr_labels)[j].name) - 1] = 0;
                
                if (!strcmp(labels->tmp_labels[counter], (labels->arr_labels)[j].name))
                {
                    presence_of_label++;
                    counter++;
                    *((double*)(commands + i + 1)) = (labels->arr_labels)[j].address;
                    (labels->arr_labels)[j].name[strlen((labels->arr_labels)[j].name)] = ':';
                    break;
                }
                (labels->arr_labels)[j].name[strlen((labels->arr_labels)[j].name)] = ':';
            }
            i += sizeof(double);
            
            if(presence_of_label == 0)
            {
                // printf("%s\n", labels->tmp_labels[counter]);
                printf("label wasn't found |%s|\n", labels->tmp_labels[counter]);
                counter++;
            }
        }
    }
}
//------------------------------------------------------------------------------------------------------

void free_tmp_labels (char** tmp_labels)
{
    assert(tmp_labels);
    for (int i = 0; tmp_labels[i]; i++)
    {
        free(tmp_labels[i]);
    }

    free(tmp_labels);
    tmp_labels = nullptr;
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
    //     printf("%d: %d\n", i, arr_of_commands[i]);
    // }
    
    fwrite(arr_of_commands, size, sizeof(char), output);

    fclose(output);
}

//------------------------------------------------------------------------------------------------------

void free_buffer (buff* buffer)
{
    assert(buffer);
    assert(buffer->text);
    free(buffer->text);
    buffer->text = nullptr;
}
