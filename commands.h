CPU("out", OUT, 0, no_arguments_handler)
CPU("add", ADD, 1, no_arguments_handler)
CPU("sub", SUB, 2, no_arguments_handler)
CPU("mul", MUL, 3, no_arguments_handler)
CPU("div", DIV, 4, no_arguments_handler)
CPU("push", PUSH, 5, one_argument_handler)
CPU("pop", POP, 6, no_arguments_handler)
CPU("pow", POW, 7, no_arguments_handler)
CPU("fsqrt", FSQRT, 8, no_arguments_handler)
CPU("print", PRINT, 9, no_arguments_handler)
CPU("in", IN, 10, no_arguments_handler)

//const char* commands_name[] = {"out", "add", "sub", "mul", "div", "push", "pop", "pow", "fsqrt", "print", "in"};
