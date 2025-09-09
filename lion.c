#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <readline/readline.h>
#include <readline/history.h>

typedef enum
{
    VALUE_NUMBER,
    VALUE_SYMBOL,
    VALUE_STRING,
    VALUE_LIST
} ValueType;

typedef struct Value Value;
typedef struct List List;

typedef struct
{
    Value *data;
    size_t count;
    size_t capacity;
} ValueArray;

typedef struct
{
    char *data;
    size_t length;
} String;

typedef struct
{
    String *keys;
    Value *values;
    size_t count;
    size_t capacity;
} Environment;

typedef struct
{
    Environment *data;
    size_t count;
    size_t capacity;
} EnvironmentStack;

struct Value
{
    ValueType type;
    union
    {
        double number;
        String symbol;
        String string;
        ValueArray list;
    } as;
};

typedef struct
{
    ValueArray values;
    EnvironmentStack environments;
    ValueArray saved_values;
    EnvironmentStack saved_environments;
} Interpreter;

ValueArray value_array_create(void)
{
    ValueArray array = {0};
    return array;
}
void value_array_push(ValueArray *array, Value value)
{
    if (array->count >= array->capacity)
    {
        size_t new_capacity = array->capacity == 0 ? 8 : array->capacity * 2;
        array->data = realloc(array->data, new_capacity * sizeof(Value));
        array->capacity = new_capacity;
    }
    array->data[array->count++] = value;
}

Value value_array_pop(ValueArray *array)
{
    return array->data[--array->count];
}

void value_array_free(ValueArray *array)
{
    free(array->data);
    array->data = NULL;
    array->count = 0;
    array->capacity = 0;
}

String string_create(const char *str)
{
    String s;
    s.length = strlen(str);
    s.data = malloc(s.length + 1);
    strcpy(s.data, str);
    return s;
}

String string_copy(String src)
{
    String s;
    s.length = src.length;
    s.data = malloc(s.length + 1);
    memcpy(s.data, src.data, s.length);
    s.data[s.length] = '\0';
    return s;
}

String string_concat(String a, String b)
{
    String result;
    result.length = a.length + b.length;
    result.data = malloc(result.length + 1);
    memcpy(result.data, a.data, a.length);
    memcpy(result.data + a.length, b.data, b.length);
    result.data[result.length] = '\0';
    return result;
}

bool string_equal(String a, String b)
{
    return a.length == b.length && memcmp(a.data, b.data, a.length) == 0;
}

void string_free(String *str)
{
    free(str->data);
    str->data = NULL;
    str->length = 0;
}

Environment environment_create(void)
{
    Environment env = {0};
    return env;
}

void environment_set(Environment *env, String key, Value value)
{
    for (size_t i = 0; i < env->count; i++)
    {
        if (string_equal(env->keys[i], key))
        {
            env->values[i] = value;
            return;
        }
    }

    if (env->count >= env->capacity)
    {
        size_t new_capacity = env->capacity == 0 ? 8 : env->capacity * 2;
        env->keys = realloc(env->keys, new_capacity * sizeof(String));
        env->values = realloc(env->values, new_capacity * sizeof(Value));
        env->capacity = new_capacity;
    }

    env->keys[env->count] = string_copy(key);
    env->values[env->count] = value;
    env->count++;
}

bool environment_get(Environment *env, String key, Value *value)
{
    for (size_t i = 0; i < env->count; i++)
    {
        if (string_equal(env->keys[i], key))
        {
            *value = env->values[i];
            return true;
        }
    }
    return false;
}

void environment_free(Environment *env)
{
    for (size_t i = 0; i < env->count; i++)
    {
        string_free(&env->keys[i]);
    }
    free(env->keys);
    free(env->values);
    env->keys = NULL;
    env->values = NULL;
    env->count = 0;
    env->capacity = 0;
}

EnvironmentStack environment_stack_create(void)
{
    EnvironmentStack stack = {0};
    return stack;
}

void environment_stack_push(EnvironmentStack *stack, Environment env)
{
    if (stack->count >= stack->capacity)
    {
        size_t new_capacity = stack->capacity == 0 ? 8 : stack->capacity * 2;
        stack->data = realloc(stack->data, new_capacity * sizeof(Environment));
        stack->capacity = new_capacity;
    }
    stack->data[stack->count++] = env;
}

Environment environment_stack_pop(EnvironmentStack *stack)
{
    return stack->data[--stack->count];
}

void environment_stack_free(EnvironmentStack *stack)
{
    for (size_t i = 0; i < stack->count; i++)
    {
        environment_free(&stack->data[i]);
    }
    free(stack->data);
    stack->data = NULL;
    stack->count = 0;
    stack->capacity = 0;
}

Value value_number(double n)
{
    Value v;
    v.type = VALUE_NUMBER;
    v.as.number = n;
    return v;
}

Value value_symbol(const char *str)
{
    Value v;
    v.type = VALUE_SYMBOL;
    v.as.symbol = string_create(str);
    return v;
}

Value value_string(const char *str)
{
    Value v;
    v.type = VALUE_STRING;
    v.as.string = string_create(str);
    return v;
}

Value value_list(void)
{
    Value v;
    v.type = VALUE_LIST;
    v.as.list = value_array_create();
    return v;
}

void value_print(Value value)
{
    switch (value.type)
    {
    case VALUE_NUMBER:
    {
        double n = value.as.number;
        if (n == floor(n))
        {
            printf("%.0f", n);
        }
        else
        {
            printf("%g", n);
        }
        break;
    }
    case VALUE_SYMBOL:

        printf("%.*s", (int)value.as.symbol.length, value.as.symbol.data);
        break;
    case VALUE_STRING:
        printf("\"%.*s\"", (int)value.as.string.length, value.as.string.data);
        break;
    case VALUE_LIST:
        printf("[");
        for (size_t i = 0; i < value.as.list.count; i++)
        {
            if (i > 0)
                printf(" ");
            value_print(value.as.list.data[i]);
        }
        printf("]");
        break;
    }
}

void lion_error(const char *msg)
{
    printf("Error: %s\n", msg);
}

bool lookup_symbol(Interpreter *interpreter, String symbol, Value *result)
{
    for (int i = interpreter->environments.count - 1; i >= 0; i--)
    {
        if (environment_get(&interpreter->environments.data[i], symbol, result))
        {
            return true;
        }
    }
    return false;
}
void interpret_value(Interpreter *interpreter, Value value);
void builtin_load(Interpreter *interpreter);

void builtin_eval(Interpreter *interpreter)
{
    if (interpreter->values.count < 1)
    {
        lion_error("not enough arguments to 'eval'");
        return;
    }

    Value arg = value_array_pop(&interpreter->values);
    if (arg.type == VALUE_LIST)
    {
        Environment new_env = environment_create();
        environment_stack_push(&interpreter->environments, new_env);

        for (size_t i = 0; i < arg.as.list.count; i++)
        {
            interpret_value(interpreter, arg.as.list.data[i]);
        }

        environment_stack_pop(&interpreter->environments);
    }
    else
    {
        interpret_value(interpreter, arg);
    }
}

void builtin_def(Interpreter *interpreter)
{
    if (interpreter->values.count < 2)
    {
        lion_error("not enough arguments to 'def'");
        return;
    }

    Value value = value_array_pop(&interpreter->values);
    Value name_list = value_array_pop(&interpreter->values);

    if (name_list.type != VALUE_LIST || name_list.as.list.count != 1 ||
        name_list.as.list.data[0].type != VALUE_SYMBOL)
    {
        lion_error("first argument to 'def' must be a symbol wrapped in a list");
        return;
    }

    String name = name_list.as.list.data[0].as.symbol;
    Environment *current_env = &interpreter->environments.data[interpreter->environments.count - 1];
    environment_set(current_env, name, value);
}

void builtin_defm(Interpreter *interpreter)
{
    if (interpreter->values.count < 1)
    {
        lion_error("not enough arguments to 'defm'");
        return;
    }

    Value symbols = value_array_pop(&interpreter->values);
    if (symbols.type != VALUE_LIST)
    {
        lion_error("last argument to 'defm' must be a list");
        return;
    }

    for (size_t i = 0; i < symbols.as.list.count; i++)
    {
        if (symbols.as.list.data[i].type != VALUE_SYMBOL)
        {
            lion_error("last argument to 'defm' must be a list of symbols");
            return;
        }
    }

    if (interpreter->values.count < symbols.as.list.count)
    {
        lion_error("not enough arguments to 'defm'");
        return;
    }

    Environment *current_env = &interpreter->environments.data[interpreter->environments.count - 1];
    for (int i = symbols.as.list.count - 1; i >= 0; i--)
    {
        Value val = value_array_pop(&interpreter->values);
        String name = symbols.as.list.data[i].as.symbol;
        environment_set(current_env, name, val);
    }
}

void builtin_wrap(Interpreter *interpreter)
{
    if (interpreter->values.count < 1)
    {
        lion_error("not enough arguments to 'wrap'");
        return;
    }

    Value arg = value_array_pop(&interpreter->values);
    Value list = value_list();
    value_array_push(&list.as.list, arg);
    value_array_push(&interpreter->values, list);
}

void builtin_print(Interpreter *interpreter)
{
    if (interpreter->values.count < 1)
    {
        lion_error("not enough arguments to 'print'");
        return;
    }

    Value arg = value_array_pop(&interpreter->values);
    if (arg.type == VALUE_STRING)
    {
        printf("%.*s\n", (int)arg.as.string.length, arg.as.string.data);
    }
    else
    {
        value_print(arg);
        printf("\n");
    }
}

void builtin_print_values(Interpreter *interpreter)
{
    printf("|");
    for (size_t i = 0; i < interpreter->values.count; i++)
    {
        printf(" ");
        value_print(interpreter->values.data[i]);
        printf(" |");
    }
    printf("\n");
}

void builtin_print_environments(Interpreter *interpreter)
{
    for (size_t i = 0; i < interpreter->environments.count; i++)
    {
        Environment *env = &interpreter->environments.data[i];
        for (size_t j = 0; j < env->count; j++)
        {
            printf("%.*s = ", (int)env->keys[j].length, env->keys[j].data);
            value_print(env->values[j]);
            printf("\n");
        }
    }
}

void builtin_add(Interpreter *interpreter)
{
    if (interpreter->values.count < 2)
    {
        lion_error("not enough arguments to '+'");
        return;
    }

    Value b = value_array_pop(&interpreter->values);
    Value a = value_array_pop(&interpreter->values);

    if (a.type == VALUE_NUMBER && b.type == VALUE_NUMBER)
    {
        value_array_push(&interpreter->values, value_number(a.as.number + b.as.number));
    }
    else if (a.type == VALUE_SYMBOL && b.type == VALUE_SYMBOL)
    {
        String result = string_concat(a.as.symbol, b.as.symbol);
        Value v = value_symbol("");
        string_free(&v.as.symbol);
        v.as.symbol = result;
        value_array_push(&interpreter->values, v);
    }
    else if (a.type == VALUE_STRING && b.type == VALUE_STRING)
    {
        String result = string_concat(a.as.string, b.as.string);
        Value v = value_string("");
        string_free(&v.as.string);
        v.as.string = result;
        value_array_push(&interpreter->values, v);
    }
    else if (a.type == VALUE_LIST && b.type == VALUE_LIST)
    {
        Value result = value_list();
        for (size_t i = 0; i < a.as.list.count; i++)
        {
            value_array_push(&result.as.list, a.as.list.data[i]);
        }
        for (size_t i = 0; i < b.as.list.count; i++)
        {
            value_array_push(&result.as.list, b.as.list.data[i]);
        }
        value_array_push(&interpreter->values, result);
    }
    else
    {
        lion_error("cannot add values of different types");
    }
}

void builtin_sub(Interpreter *interpreter)
{
    if (interpreter->values.count < 2)
    {
        lion_error("not enough arguments to '-'");
        return;
    }

    Value b = value_array_pop(&interpreter->values);
    Value a = value_array_pop(&interpreter->values);

    if (a.type == VALUE_NUMBER && b.type == VALUE_NUMBER)
    {
        value_array_push(&interpreter->values, value_number(a.as.number - b.as.number));
    }
    else
    {
        lion_error("both arguments to '-' must be numbers");
    }
}

void builtin_mul(Interpreter *interpreter)
{
    if (interpreter->values.count < 2)
    {
        lion_error("not enough arguments to '*'");
        return;
    }

    Value b = value_array_pop(&interpreter->values);
    Value a = value_array_pop(&interpreter->values);

    if (a.type == VALUE_NUMBER && b.type == VALUE_NUMBER)
    {
        value_array_push(&interpreter->values, value_number(a.as.number * b.as.number));
    }
    else
    {
        lion_error("both arguments to '*' must be numbers");
    }
}

void builtin_div(Interpreter *interpreter)
{
    if (interpreter->values.count < 2)
    {
        lion_error("not enough arguments to '/'");
        return;
    }

    Value b = value_array_pop(&interpreter->values);
    Value a = value_array_pop(&interpreter->values);

    if (a.type == VALUE_NUMBER && b.type == VALUE_NUMBER)
    {
        value_array_push(&interpreter->values, value_number(a.as.number / b.as.number));
    }
    else
    {
        lion_error("both arguments to '/' must be numbers");
    }
}

void interpret_value(Interpreter *interpreter, Value value)
{
    switch (value.type)
    {
    case VALUE_NUMBER:
    case VALUE_STRING:
    case VALUE_LIST:
        value_array_push(&interpreter->values, value);
        break;
    case VALUE_SYMBOL:
    {
        Value result;
        if (lookup_symbol(interpreter, value.as.symbol, &result))
        {
            value_array_push(&interpreter->values, result);
            builtin_eval(interpreter);
            return;
        }

        String sym = value.as.symbol;
        if (string_equal(sym, string_create("eval")))
        {
            builtin_eval(interpreter);
        }
        else if (string_equal(sym, string_create("def")))
        {
            builtin_def(interpreter);
        }
        else if (string_equal(sym, string_create("defm")))
        {
            builtin_defm(interpreter);
        }
        else if (string_equal(sym, string_create("wrap")))
        {
            builtin_wrap(interpreter);
        }
        else if (string_equal(sym, string_create("print")))
        {
            builtin_print(interpreter);
        }
        else if (string_equal(sym, string_create("print_values")))
        {
            builtin_print_values(interpreter);
        }
        else if (string_equal(sym, string_create("print_environments")))
        {
            builtin_print_environments(interpreter);
        }
        else if (string_equal(sym, string_create("load")))
        {
            builtin_load(interpreter);
        }
        else if (string_equal(sym, string_create("exit")))
        {
            exit(0);
        }
        else if (string_equal(sym, string_create("+")))
        {
            builtin_add(interpreter);
        }
        else if (string_equal(sym, string_create("-")))
        {
            builtin_sub(interpreter);
        }
        else if (string_equal(sym, string_create("*")))
        {
            builtin_mul(interpreter);
        }
        else if (string_equal(sym, string_create("/")))
        {
            builtin_div(interpreter);
        }
        else
        {
            printf("Error: undefined symbol '%.*s'\n",
                   (int)sym.length, sym.data);
        }
        break;
    }
    }
}

typedef struct
{
    char **tokens;
    size_t count;
    size_t capacity;
} TokenArray;

TokenArray tokenize(const char *code)
{
    TokenArray tokens = {0};
    size_t i = 0;
    size_t len = strlen(code);

    while (i < len)
    {
        char ch = code[i];

        if (isspace(ch))
        {
            i++;
        }
        else if (ch == '[' || ch == ']')
        {
            if (tokens.count >= tokens.capacity)
            {
                size_t new_capacity = tokens.capacity == 0 ? 32 : tokens.capacity * 2;
                tokens.tokens = realloc(tokens.tokens, new_capacity * sizeof(char *));
                tokens.capacity = new_capacity;
            }
            tokens.tokens[tokens.count] = malloc(2);
            tokens.tokens[tokens.count][0] = ch;
            tokens.tokens[tokens.count][1] = '\0';
            tokens.count++;
            i++;
        }
        else if (ch == '"')
        {
            size_t start = i;
            i++;
            while (i < len)
            {
                if (code[i] == '"' && (i == 0 || code[i - 1] != '\\'))
                {
                    i++;
                    break;
                }
                i++;
            }

            if (tokens.count >= tokens.capacity)
            {
                size_t new_capacity = tokens.capacity == 0 ? 32 : tokens.capacity * 2;
                tokens.tokens = realloc(tokens.tokens, new_capacity * sizeof(char *));
                tokens.capacity = new_capacity;
            }

            size_t token_len = i - start;
            tokens.tokens[tokens.count] = malloc(token_len + 1);
            memcpy(tokens.tokens[tokens.count], code + start, token_len);
            tokens.tokens[tokens.count][token_len] = '\0';
            tokens.count++;
        }
        else if (ch == '(')
        {
            i++;
            while (i < len && code[i] != ')')
            {
                i++;
            }
            if (i < len)
                i++;
        }
        else
        {
            size_t start = i;
            while (i < len && !isspace(code[i]) && code[i] != '[' && code[i] != ']' && code[i] != '"')
            {
                i++;
            }

            if (tokens.count >= tokens.capacity)
            {
                size_t new_capacity = tokens.capacity == 0 ? 32 : tokens.capacity * 2;
                tokens.tokens = realloc(tokens.tokens, new_capacity * sizeof(char *));
                tokens.capacity = new_capacity;
            }

            size_t token_len = i - start;
            tokens.tokens[tokens.count] = malloc(token_len + 1);
            memcpy(tokens.tokens[tokens.count], code + start, token_len);
            tokens.tokens[tokens.count][token_len] = '\0';
            tokens.count++;
        }
    }

    return tokens;
}

bool is_number(const char *str)
{
    if (*str == '-')
        str++;
    if (!isdigit(*str))
        return false;
    while (isdigit(*str))
        str++;
    if (*str == '.')
    {
        str++;
        if (!isdigit(*str))
            return false;
        while (isdigit(*str))
            str++;
    }
    return *str == '\0';
}

Value parse_tokens(char **tokens, size_t *index, size_t count)
{
    if (*index >= count)
    {
        return value_list();
    }

    Value result = value_list();

    while (*index < count)
    {
        char *token = tokens[*index];

        if (strcmp(token, "[") == 0)
        {
            (*index)++;
            Value nested = parse_tokens(tokens, index, count);
            value_array_push(&result.as.list, nested);
        }
        else if (strcmp(token, "]") == 0)
        {
            (*index)++;
            return result;
        }
        else if (is_number(token))
        {
            double num = atof(token);
            value_array_push(&result.as.list, value_number(num));
            (*index)++;
        }
        else if (token[0] == '"' && token[strlen(token) - 1] == '"')
        {
            size_t len = strlen(token);
            char *content = malloc(len - 1);
            memcpy(content, token + 1, len - 2);
            content[len - 2] = '\0';

            char *unescaped = malloc(len - 1);
            size_t j = 0;
            for (size_t i = 0; i < len - 2; i++)
            {
                if (content[i] == '\\' && i + 1 < len - 2)
                {
                    if (content[i + 1] == '"')
                    {
                        unescaped[j++] = '"';
                        i++;
                    }
                    else if (content[i + 1] == '\\')
                    {
                        unescaped[j++] = '\\';
                        i++;
                    }
                    else
                    {
                        unescaped[j++] = content[i];
                    }
                }
                else
                {
                    unescaped[j++] = content[i];
                }
            }
            unescaped[j] = '\0';

            Value v = value_string(unescaped);
            value_array_push(&result.as.list, v);

            free(content);
            free(unescaped);
            (*index)++;
        }
        else
        {
            value_array_push(&result.as.list, value_symbol(token));
            (*index)++;
        }
    }

    return result;
}

void interpret_code(Interpreter *interpreter, const char *code)
{
    TokenArray tokens = tokenize(code);

    size_t index = 0;
    Value parsed = parse_tokens(tokens.tokens, &index, tokens.count);

    for (size_t i = 0; i < parsed.as.list.count; i++)
    {
        interpret_value(interpreter, parsed.as.list.data[i]);
    }

    for (size_t i = 0; i < tokens.count; i++)
    {
        free(tokens.tokens[i]);
    }
    free(tokens.tokens);
    value_array_free(&parsed.as.list);
}

void save_state(Interpreter *interpreter)
{
    value_array_free(&interpreter->saved_values);
    environment_stack_free(&interpreter->saved_environments);

    interpreter->saved_values = value_array_create();
    for (size_t i = 0; i < interpreter->values.count; i++)
    {
        value_array_push(&interpreter->saved_values, interpreter->values.data[i]);
    }

    interpreter->saved_environments = environment_stack_create();
    for (size_t i = 0; i < interpreter->environments.count; i++)
    {
        Environment env = environment_create();
        Environment *src = &interpreter->environments.data[i];
        for (size_t j = 0; j < src->count; j++)
        {
            environment_set(&env, src->keys[j], src->values[j]);
        }
        environment_stack_push(&interpreter->saved_environments, env);
    }
}

void restore_state(Interpreter *interpreter)
{
    value_array_free(&interpreter->values);
    environment_stack_free(&interpreter->environments);

    interpreter->values = value_array_create();
    for (size_t i = 0; i < interpreter->saved_values.count; i++)
    {
        value_array_push(&interpreter->values, interpreter->saved_values.data[i]);
    }

    interpreter->environments = environment_stack_create();
    for (size_t i = 0; i < interpreter->saved_environments.count; i++)
    {
        Environment env = environment_create();
        Environment *src = &interpreter->saved_environments.data[i];
        for (size_t j = 0; j < src->count; j++)
        {
            environment_set(&env, src->keys[j], src->values[j]);
        }
        environment_stack_push(&interpreter->environments, env);
    }
}

void load_file(Interpreter *interpreter, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Error: cannot open file '%s'\n", filename);
        return;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(length + 1);
    fread(content, 1, length, file);
    content[length] = '\0';
    fclose(file);

    interpret_code(interpreter, content);
    free(content);
}

void builtin_load(Interpreter *interpreter)
{
    if (interpreter->values.count < 1)
    {
        lion_error("not enough arguments to 'load'");
        return;
    }

    Value arg = value_array_pop(&interpreter->values);
    if (arg.type != VALUE_STRING)
    {
        lion_error("first argument to 'load' must be a string");
        return;
    }

    char *filename = malloc(arg.as.string.length + 1);
    memcpy(filename, arg.as.string.data, arg.as.string.length);
    filename[arg.as.string.length] = '\0';

    load_file(interpreter, filename);
    free(filename);
}

void repl(Interpreter *interpreter)
{
    char *history_file = strcat(strcpy(malloc(strlen(getenv("HOME")) + 20), getenv("HOME")), "/.lion_history");

    using_history();
    read_history(history_file);

    char *input;
    while ((input = readline(">>> ")) != NULL)
    {
        if (*input)
        {
            add_history(input);
            save_state(interpreter);

            interpret_code(interpreter, input);

            if (interpreter->values.count > 0)
            {
                for (size_t i = 0; i < interpreter->values.count; i++)
                {
                    if (i > 0)
                        printf(" ");
                    value_print(interpreter->values.data[i]);
                }
                printf("\n");
            }
        }
        free(input);
    }

    write_history(history_file);
    free(history_file);
}

int main(int argc, char **argv)
{
    Interpreter interpreter = {0};
    interpreter.values = value_array_create();
    interpreter.environments = environment_stack_create();
    interpreter.saved_values = value_array_create();
    interpreter.saved_environments = environment_stack_create();

    Environment base_env = environment_create();
    environment_set(&base_env, string_create("pi"), value_number(3.14));
    environment_stack_push(&interpreter.environments, base_env);

    if (argc == 1)
    {
        repl(&interpreter);
    }
    else if (argc == 2)
    {
        load_file(&interpreter, argv[1]);
    }
    else
    {
        printf("Usage: lion [file]\n");
        return 1;
    }

    return 0;
}
