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
    size_t length;
    size_t capacity;
} List;

List list_init(size_t capacity);
void list_grow(List *list, size_t capacity);
void list_append(List *list, Value value);
void list_free(List list);

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

typedef struct {
    const char *data;
    size_t length;
} Slice;

Slice slice(const char *str);

char *read_file(char *path);
List parse(Slice code);
void apply(List list, Stack *stack);

#endif
