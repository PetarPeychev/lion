#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>

struct String
{
    char *data;
    size_t len;

    char &operator[](size_t index)
    {
        return data[index];
    }
};

String slice(String str, size_t start, size_t end)
{
    String slice = {NULL, 0};

    if (start >= str.len || end > str.len || start >= end)
    {
        return slice;
    }

    slice.data = str.data + start;
    slice.len = end - start;

    return slice;
}

void print(String str)
{
    if (str.data && str.len > 0)
    {
        printf("%.*s\n", (int)str.len, str.data);
    }
}

struct Value
{
    enum Type
    {
        Number,
        Symbol,
        Block
    };
    Type type;

    union
    {
        int i;
        String s;
        Block b;
    } data;
};

void interpret(String input)
{
    size_t token_length = 0;

    for (int i = 0; i <= input.len; i++)
    {
        if (isspace(input[i]) || i == input.len)
        {
            if (token_length > 0)
            {
                String token = slice(input, i - token_length, i);
            }
            token_length = 0;
        }
        else
        {
            token_length++;
        }
    }
}

int main()
{
    char *input;

    using_history();

    while (true)
    {
        input = readline("> ");

        if (input == NULL)
        {
            break;
        }

        if (*input)
        {
            add_history(input);
            interpret((String){.data = input, .len = strlen(input)});
        }

        free(input);
    }

    return 0;
}
