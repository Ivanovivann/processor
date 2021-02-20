#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//------------------------------------------------------------------------------------------------------

typedef struct buff{
    char* text;
    size_t size;
}buff;

//------------------------------------------------------------------------------------------------------

size_t size_of_file (FILE* text);

buff* reading_file (buff* text, char* name_of_file);

void assembling (buff* buffer);

void one_argument_handler (FILE* output, int command_number, double argument);

void no_arguments_handler (FILE* output, int command_number);

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

void one_argument_handler (FILE* output, buff* buffer, int command_number)
{
    buffer->text += strlen(strtok(buffer->text, " ")) + 1;
    fprintf(output, "%d\n%lg\n", command_number, atof(strtok(buffer->text, "\r")));
    buffer->text += strlen(strtok(buffer->text, "\r")) + 1;   
}

//------------------------------------------------------------------------------------------------------

void no_arguments_handler (FILE* output, buff* buffer, int command_number)
{
    fprintf(output, "%d\n", command_number);
    buffer->text += strlen(strtok(buffer->text, "\r")) + 1; 
}

//------------------------------------------------------------------------------------------------------

void assembling (buff* buffer)
{
    char* token = buffer->text;
    char* begin_buf = buffer->text;
    
    FILE* output = fopen("asm_out.txt", "w");
    
    while (begin_buf + buffer->size - buffer->text > 0 && (token = strtok(buffer->text, "\r")) != NULL)
    {
        #define CPU(name_of_command, name_code_of_command, code_of_command, in_handler)         \
        if (!strcmp(token, name_of_command) || !strcmp(strtok(token, " "), name_of_command))    \
        {                                                                                       \
            in_handler(output, buffer, code_of_command);                                        \
        }
        #include "commands.h"
        #undef CPU
        
        buffer->text++;
    }

    fclose(output);

    buffer->text = begin_buf;

    return ;
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
