#include "stack.h"

void fill_poison (Stack* stack, unsigned prev_capacity)
{
    for (unsigned i = prev_capacity + sizeof(CANARY1) / sizeof(type_of_stack); i < stack->capacity + sizeof(CANARY1) / sizeof(type_of_stack); i++)
    {
        ((type_of_stack*)(stack->data))[i] = NAN;
    }
    ((unsigned long long *)(stack->data))[1 + stack->capacity / (sizeof(CANARY1) / sizeof(type_of_stack))] = CANARY1;
}

void* poison_realloc (Stack* stack, unsigned prev_capacity, unsigned capacity)
{
    if (! prev_capacity) {
        stack->data = malloc(capacity * sizeof(type_of_stack) + 2 * sizeof(CANARY1));
        ((unsigned long long *)(stack->data))[0] = CANARY1;
    }
    else {
        stack->data = realloc ((&(((unsigned long long *)(stack->data))[-1])), capacity * sizeof(type_of_stack) + 2 * sizeof(CANARY1));
    }

    fill_poison (stack, prev_capacity);

    return (void *)(&(((char*)(stack->data))[sizeof(CANARY1)]));
}

enum Stack_error construct_stack (Stack* stack)
{
    stack->canary1 = CANARY;
    stack->canary2 = CANARY;
    stack->capacity = BEGIN_CAPACITY;
    stack->data = poison_realloc(stack, 0, stack->capacity);
    stack->size = 0;

    stack->hash = make_hash (stack);

    return verificator (stack);
}

enum Stack_error verificator (Stack* stack)
{
    if (stack == NULL) {
        return stack_nullptr;
    }
    if (stack->size >= stack->capacity /* || (stack->size > 0 && ((type_of_stack *)(stack->data))[stack->size - 1] != ((type_of_stack *)(stack->data))[stack->size - 1])*/) {
        return bad_size;
    }
    if (stack->data == NULL) {
        return no_free_memory;
    }
    if (stack->size < 0) {
        return stack_is_empty;
    }
    if (stack->canary1 != CANARY) {
        return stack_canary_left_in_danger;
    }
    if (stack->canary2 != CANARY) {
        return stack_canary_right_in_danger;
    }
    if (((unsigned long long *)(stack->data))[-1] != CANARY1) {
        return data_canary_left_in_danger;
    }
    if (((unsigned long long *)(stack->data))[stack->capacity / (sizeof(CANARY1) / sizeof(type_of_stack))] != CANARY1) {
        return data_canary_right_in_danger;
    }
    if (stack->hash != make_hash (stack)) {
        return hash_error;
    }

    return no_errors;
}

void destruct_stack (Stack* stack)
{
    int error = verificator(stack);
    if (error) {
        dump_stack (stack);
    }

    free ((char *) stack->data - sizeof (CANARY1));
    stack->data = NULL;
    stack->size = 0;
    stack->capacity = 0;
    stack->canary1 = 0;
    stack->canary2 = 0;
}

enum Stack_error push_stack (Stack* stack, type_of_stack value)
{
    int error = verificator(stack);
    if (error) {
        dump_stack (stack);
    }

    if (stack->size >= stack->capacity - 1){
        unsigned prev_capacity = stack->capacity;
        stack->capacity *= 2;
        stack->data = poison_realloc(stack, prev_capacity, stack->capacity);
    }

    ((type_of_stack *)(stack->data))[stack->size] = value;
    stack->size++;

    stack->hash = make_hash (stack);

    return verificator(stack);
}

enum Stack_error pop_stack (Stack* stack, type_of_stack* value)
{
    int error = verificator(stack);
    if (error) {
        dump_stack (stack);
    }

    --(stack->size);
    *value = ((type_of_stack *)(stack->data))[stack->size];

    ((type_of_stack *)(stack->data))[stack->size] = NAN;

    if (stack->size * 2 < stack->capacity && stack->capacity > BEGIN_CAPACITY) {
        unsigned prev_capacity = stack->capacity;
        stack->capacity /= 2;
        stack->data = poison_realloc(stack, prev_capacity, stack->capacity);
    }
    stack->hash = make_hash (stack);

    return verificator(stack);
}

void dump_stack (Stack* stack)
{
    int error = verificator(stack);
    if (error) {
        printf("Stack is BAD error - %d\n", error);
    }
    else{
        printf("Stack is good [%p] {\n", stack);
    }
    printf("\tstack left canary = %llX\n", stack->canary1);
    printf("\tcapacity = %u\n", stack->capacity);
    printf("\tsize = %u\n", stack->size);
    printf("\tstack right canary = %llX\n", stack->canary2);

    printf("\t\tdata left canary = %llX\n", ((unsigned long long *)(stack->data))[-1]);
    for (int i = 0; i < stack->capacity; i++)
    {
        if (((type_of_stack *)(stack->data))[i] != ((type_of_stack *)(stack->data))[i]) {
            printf("\t\tElement [%d] - %lG (POISON)\n", i, ((type_of_stack *)(stack->data))[i]);
        }
        else {
            printf("\t\tElement [%d] - %lg\n", i, ((type_of_stack *)(stack->data))[i]);
        }

    }
    printf("\t\tdata right canary = %llX\n", ((unsigned long long *)(stack->data))[stack->capacity / (sizeof(CANARY1) / sizeof(type_of_stack))]);
    printf("}\n");
}

unsigned long long make_hash (Stack* stack)
{
    char * bait_stack_eqvivalent = (char *) stack;

    unsigned long long old_hash = stack->hash;
    stack->hash = 0;

    unsigned long long new_hash = 0;

    for (int i = 0; i < sizeof (* stack); i++)
    {
        new_hash = ROR (new_hash) + bait_stack_eqvivalent[i];
    }

    char * inception_of_data = (char *) stack->data - sizeof (CANARY1);
    for (int i = 0; i < stack->capacity * sizeof (type_of_stack) + 2 * sizeof (CANARY1); i++)
    {
        new_hash = ROR (new_hash) + inception_of_data[i];
    }

    stack->hash = old_hash;

    //printf ("%llx %llx \n", old_hash, new_hash);
    return new_hash;
}

unsigned long long ROL (unsigned long long value)
{
    return (value << 1 | value >> 63);
}

unsigned long long ROR (unsigned long long value)
{
    return (value >> 1 | value << 63);
}

void test_stack (Stack * stack)
{
    for (int i = 0; i < 1230; i++)
    {
        if (push_stack (stack, i)) {
            printf ("push is bad\n");
            dump_stack(stack);
        }
    }

    for (int i = 0; i < 1220; i++)
    {
        type_of_stack value = 0;
        if (pop_stack (stack, &value)) {
            printf ("pop is bad\n");
            dump_stack(stack);
        }
    }
}
