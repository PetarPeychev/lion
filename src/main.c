#include "lion.h"
#include <stdio.h>
#include <string.h>

void print_usage(char *program_name) {
    fprintf(stderr, "Usage: %s <command> [ARGUMENTS]\n", program_name);
    fprintf(stderr, "\n");
    fprintf(stderr, "Commands:\n");
    fprintf(stderr, "    run [FILE] - Run a lion program.\n");
}

int main(int argc, char *argv[]) {
    char *program_name = argv[0];

    if (argc < 2) {
        print_usage(program_name);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "run") == 0) {
        if (argc < 3) {
            fprintf(stderr, "ERROR: Filename not provided.\n\n");
            print_usage(program_name);
            return EXIT_FAILURE;
        } else if (argc > 3) {
            fprintf(stderr, "ERROR: Too many arguments.\n\n");
            print_usage(program_name);
            return EXIT_FAILURE;
        }
        char *source = read_file(argv[2]);
        if (source == NULL) {
            fprintf(stderr, "ERROR: Failed to read file '%s'.\n", argv[2]);
            return EXIT_FAILURE;
        }

        struct List tokens = parse(slice(source));
        struct Stack stack = stack_init();
        apply(tokens, &stack);

        free(source);
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "ERROR: Unrecognized command '%s'.\n\n", argv[1]);
        print_usage(program_name);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
