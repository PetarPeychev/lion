#include "lion.h"

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
