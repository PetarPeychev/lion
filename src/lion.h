#ifndef LION_H
#define LION_H

#include <stdbool.h>
#include <stdlib.h>

typedef enum {
    VALUE_NUMBER,
    VALUE_STRING,
    VALUE_SYMBOL,
    VALUE_LIST
} ValueType;

typedef struct Value Value;

typedef struct {
    Value *items;
    size_t count;
    size_t capacity;
} List;

typedef union {
    double number;
    char *symbol;
    char *string;
    List list;
} ValueData;

struct Value {
    ValueType type;
    ValueData data;
};

#define STACK_SIZE 256

typedef struct {
    Value items[STACK_SIZE];
    int top;
} Stack;

Stack stack_init(void);
bool stack_is_empty(Stack *stack);
bool stack_is_full(Stack *stack);
bool stack_push(Stack *stack, Value value);
bool stack_pop(Stack *stack, Value *out);

char *read_file(char *path);
List parse(char *code);
void apply(List list, Stack *stack);

#endif
