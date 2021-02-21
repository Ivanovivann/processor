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

void disassembling (buff* buffer);

void func_without_arguments (FILE* output, buff* buffer, char* command_name);

void func_with_arguments (FILE* output, buff* buffer, char* command_name);

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
    
    FILE* output = fopen("disasm_out.txt", "w");
    
    while (begin_buf + buffer->size - buffer->text > 0 && (token = strtok(buffer->text, "\n")) != NULL)
    {
        buffer->text += strlen(token) + 1;
        switch (atoi(token)) 
        {
        #define CPU(name_of_command, name_code_of_command, code_of_command, in_handler, out_handler)    \
        case code_of_command :                                                                          \
            out_handler(output, buffer, name_of_command);                                               \
            break;                                        
        #include "commands.h"
        #undef CPU
        }
    }

    fclose(output);

    buffer->text = begin_buf;

    return;
}

//------------------------------------------------------------------------------------------------------

void func_without_arguments (FILE* output, buff* buffer, char* command_name)
{
    fprintf(output, "%s\n", command_name);
}

//------------------------------------------------------------------------------------------------------

void func_with_arguments (FILE* output, buff* buffer, char* command_name)
{ 
    fprintf(output, "%s %lg\n", command_name, atof(strtok(buffer->text, "\n")));
    buffer->text += strlen(strtok(buffer->text, "\n")) + 1;
}

//------------------------------------------------------------------------------------------------------

void free_buffer (buff* buffer)
{
    free(buffer->text);
    free(buffer);
}