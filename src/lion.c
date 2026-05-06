#include "lion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- LIST ---
struct List list_init(size_t capacity) {
    struct List list = {
        .items = malloc(sizeof(struct Value) * capacity),
        .length = 0,
        .capacity = capacity
    };
    if (list.items == NULL) {
        fprintf(
            stderr,
            "ERROR: Failed to allocate %zu bytes for list.\n",
            sizeof(struct Value) * capacity
        );
        exit(EXIT_FAILURE);
    }
    return list;
}

void list_grow(struct List *list, size_t capacity) {
    list->items = realloc(list->items, sizeof(struct Value) * capacity);
    list->capacity = capacity;
}

void list_append(struct List *list, struct Value value) {
    if (list->length >= list->capacity) {
        list_grow(list, list->capacity * 2);
    }
    list->items[list->length++] = value;
}

void list_free(struct List list) {
    free(list.items);
}

// --- STACK ---
struct Stack stack_init(void) {
    struct Stack stack = {.top = -1};
    return stack;
}

bool stack_is_empty(struct Stack *stack) {
    return stack->top == -1;
}

bool stack_is_full(struct Stack *stack) {
    return stack->top >= STACK_SIZE - 1;
}

bool stack_push(struct Stack *stack, struct Value value) {
    if (stack_is_full(stack)) {
        return false;
    }
    stack->items[++stack->top] = value;
    return true;
}

bool stack_pop(struct Stack *stack, struct Value *out) {
    if (stack_is_empty(stack)) {
        return false;
    }
    *out = stack->items[stack->top--];
    return true;
}

// --- SLICE ---
struct Slice slice(const char *str) {
    struct Slice slice = {.data = str, .length = strlen(str)};
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
    size_t read = fread(buffer, 1, length, file);
    if (read != length) {
        fprintf(stderr, "ERROR: Could not read file '%s'\n", path);
        fclose(file);
        return NULL;
    }
    buffer[length] = '\0';

    fclose(file);
    return buffer;
}

struct List parse(struct Slice code) {
    printf("%.*s\n", (int)code.length, code.data);
    struct List tokens = {0};
    return tokens;
}

void apply(struct List list, struct Stack *stack) {
}
