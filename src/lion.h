#ifndef LION_H
#define LION_H

#include <stdbool.h>
#include <stdlib.h>

enum ValueType {
    VALUE_NUMBER,
    VALUE_STRING,
    VALUE_SYMBOL,
    VALUE_LIST
};

struct Value;

struct List {
    struct Value *items;
    size_t length;
    size_t capacity;
};

struct List list_init(size_t capacity);
void list_grow(struct List *list, size_t capacity);
void list_append(struct List *list, struct Value value);
void list_free(struct List list);

union ValueData {
    double number;
    char *symbol;
    char *string;
    struct List list;
};

struct Value {
    enum ValueType type;
    union ValueData data;
};

#define STACK_SIZE 256

struct Stack {
    struct Value items[STACK_SIZE];
    int top;
};

struct Stack stack_init(void);
bool stack_is_empty(struct Stack *stack);
bool stack_is_full(struct Stack *stack);
bool stack_push(struct Stack *stack, struct Value value);
bool stack_pop(struct Stack *stack, struct Value *out);

struct Slice {
    const char *data;
    size_t length;
};

struct Slice slice(const char *str);

char *read_file(char *path);
struct List parse(struct Slice code);
void apply(struct List list, struct Stack *stack);

#endif
