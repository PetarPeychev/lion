#include "lion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- LIST ---
List list_init(size_t capacity) {
    List list = {
        .items = malloc(sizeof(Value) * capacity),
        .length = 0,
        .capacity = capacity
    };
    if (list.items == NULL) {
        fprintf(
            stderr,
            "ERROR: Failed to allocate %zu bytes for list.\n",
            sizeof(Value) * capacity
        );
        exit(EXIT_FAILURE);
    }
    return list;
}

void list_grow(List *list, size_t capacity) {
    list->items = realloc(list->items, sizeof(Value) * capacity);
    list->capacity = capacity;
}

void list_append(List *list, Value value) {
    if (list->length >= list->capacity) {
        list_grow(list, list->capacity * 2);
    }
    list->items[list->length++] = value;
}

void list_free(List list) {
    free(list.items);
}

// --- STACK ---
Stack stack_init(void) {
    Stack stack = {.top = -1};
    return stack;
}

bool stack_is_empty(Stack *stack) {
    return stack->top == -1;
}

bool stack_is_full(Stack *stack) {
    return stack->top >= STACK_SIZE - 1;
}

bool stack_push(Stack *stack, Value value) {
    if (stack_is_full(stack)) {
        return false;
    }
    stack->items[++stack->top] = value;
    return true;
}

bool stack_pop(Stack *stack, Value *out) {
    if (stack_is_empty(stack)) {
        return false;
    }
    *out = stack->items[stack->top--];
    return true;
}

// --- SLICE ---
Slice slice(const char *str) {
    Slice slice = {.data = str, .length = strlen(str)};
    return slice;
}

// --- INTERPRETER ---
char *read_file(char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long position = ftell(file);
    if (position < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s'\n", path);
        fclose(file);
        return NULL;
    }
    size_t length = (size_t)position;
    rewind(file);

    char *buffer = malloc(length + 1);
    if (!buffer) {
        fprintf(stderr, "ERROR: Not enough memory to read '%s'\n", path);
        fclose(file);
        return NULL;
    }
    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);
    return buffer;
}

List parse(Slice code) {
    printf("%s\n", code.data);
    List tokens = {0};
    return tokens;
}

void apply(List list, Stack *stack) {
}
