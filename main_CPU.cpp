#include "CPU.h"

//------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    CPU proc = {};

    CPU_construct (&proc);

    buff buffer = {};

    if (argc < 2)
    {
        printf("Usage: <%s> file-to-open\n", argv[0]);
        return -1;
    }

    processor_doing_brrr(&proc, reading_file (&buffer, argv[1]));

    free_buffer(&buffer);
    CPU_destruct(&proc);

    return 0;
}
