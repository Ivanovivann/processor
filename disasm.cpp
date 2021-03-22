#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

//------------------------------------------------------------------------------------------------------

typedef struct{
    int label_number;
    long long int label_address;
}label;

typedef struct buff{
    char* text;
    size_t size;
}buff;

//------------------------------------------------------------------------------------------------------

size_t size_of_file (FILE* text);

buff* reading_file (buff* text, char* name_of_file);

void disassembling (buff* buffer);

int search_labels(buff* buffer, label* labels);

int cmp_label (const void* p1, const void* p2);

void func_without_arguments (FILE* output, buff* buffer, const char* command_name, label* labels);

void push_or_pop (FILE* output, buff* buffer, const char* command_name, label* labels);

void jumps (FILE* output, buff* buffer, const char* command_name, label* labels);

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

    disassembling (reading_file (&buffer, argv[1]));

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
    assert(buffer);
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

    label* labels = (label*) calloc(buffer->size / 9 + 1, sizeof(label)); //because address is double and 1 byte for define command

    int count_of_labels = search_labels(buffer, labels);
    
    int cur_label_counter = 0;

    while (begin_buf + buffer->size - buffer->text - 1 > 0)
    {
        if (cur_label_counter < count_of_labels && labels[cur_label_counter].label_address == (buffer->text - begin_buf))
        {
            fprintf(output, "L%d:\n", labels[cur_label_counter].label_number);
            cur_label_counter++;
        }

        switch (*buffer->text)
        {
            #define CPU(name_of_command, name_code_of_command, code_of_command, in_handler, out_handler, cpu_func)  \
            case code_of_command :                                                                                  \
                out_handler(output, buffer, name_of_command, labels);                                                       \
                break;                                        
            #include "commands.h"
            #undef CPU
        }
        buffer->text++;
    }

    fclose(output);

    buffer->text = begin_buf;
    free(labels);

    return;
}

//------------------------------------------------------------------------------------------------------

int search_labels(buff* buffer, label* labels)
{
    int counter = 0;
    for (int i = 0; i < buffer->size; i++)
    {
        if (buffer->text[i] == 5 || buffer->text[i] == 6) // push or pop
        {
            i++;
            if (buffer->text[i] & (1 << 5))         // bit of number mod
            {
                i += sizeof(double);
            }
            continue;
        }
        if (buffer->text[i] / 10 == 7)
        {
            long long int tmp = (long long int)(*(double*)(buffer->text + i + 1));
            int j = 0;
            i += sizeof(double) - 1;

            for(; j < counter; j++)
            {
                if (labels[j].label_address == tmp)
                    break;
            }
            if(j != counter && labels[j].label_address == tmp)
                continue;

            labels[counter].label_address = tmp;
            labels[counter].label_number = counter;
            counter++;
        }
    }
    qsort(labels, counter, sizeof(label), cmp_label);

    return counter;
}

//------------------------------------------------------------------------------------------------------

int cmp_label (const void* p1, const void* p2)
{
    return ((label*)p1)->label_address - ((label*)p2)->label_address;
}

//------------------------------------------------------------------------------------------------------

void func_without_arguments (FILE* output, buff* buffer, const char* command_name, label* labels)
{
    fprintf(output, "   %s\n", command_name);
}

//------------------------------------------------------------------------------------------------------

void push_or_pop (FILE* output, buff* buffer, const char* command_name, label* labels)
{
    buffer->text++;

    if ((*buffer->text) & (1 << 7))
    {
        switch (((*buffer->text) & (7 << 4)) >> 4)
        {
            case 1:                                 // only register
                fprintf(output, "   %s r%cx\n", command_name, (((*buffer->text) & (3 << 2)) >> 2) + 'a');
                break;
            case 2:                                 // only number
                fprintf(output, "   %s %lg\n", command_name, *(double*)(buffer->text + 1));
                buffer->text += sizeof(double);
                break;
            case 5:                                 // RAM and register
                fprintf(output, "   %s [r%cx]\n", command_name, (((*buffer->text) & (3 << 2)) >> 2) + 'a');
                break;
            case 6:                                 // RAM and number
                fprintf(output, "   %s [%lg]\n", command_name, *(double*)(buffer->text + 1));
                buffer->text += sizeof(double);
                break;
            case 7:                                 // RAM, register and number
                if((*buffer->text) & (1 << 1))
                {
                    fprintf(output, "   %s [r%cx + %lg]\n", command_name, (((*buffer->text) & (3 << 2)) >> 2) + 'a', *(double*)(buffer->text + 1)); 
                }
                else
                {
                    fprintf(output, "   %s [r%cx - %lg]\n", command_name, (((*buffer->text) & (3 << 2)) >> 2) + 'a', *(double*)(buffer->text + 1)); 
                }
                buffer->text += sizeof(double);
                break;
            default:
                printf("NO, NO, THAT'S NOT TRUE. IT'S IMPOSSIBLE!!!\n");
        }
        return;
    }
    fprintf(output, "   %s\n", command_name);
}

//------------------------------------------------------------------------------------------------------

void jumps (FILE* output, buff* buffer, const char* command_name, label* labels)
{
    int index_label = 0;
    long long int tmp = (long long int)(*(double*)(buffer->text + 1));
    
    for (; tmp != labels[index_label].label_address; index_label++)
        ;

    fprintf(output, "   %s L%d\n", command_name, labels[index_label].label_number);
    buffer->text += sizeof(double);
}

//------------------------------------------------------------------------------------------------------

void free_buffer (buff* buffer)
{
    free(buffer->text);
}
