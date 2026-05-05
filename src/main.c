#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage(char *program_name) {
    fprintf(stderr, "Usage: %s <command> [ARGUMENTS]\n", program_name);
    fprintf(stderr, "\n");
    fprintf(stderr, "Commands:\n");
    fprintf(stderr, "    run [FILE] - Run a lion program.\n");
}

int run(char *file_name) {
    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file.\n");
        return EXIT_FAILURE;
    }

    int c;
    while ((c = fgetc(file)) != EOF) {
        printf("%c", c);
    }

    fclose(file);
    return EXIT_SUCCESS;
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
        return run(argv[2]);
    } else {
        fprintf(stderr, "ERROR: Unrecognized command '%s'.\n\n", argv[1]);
        print_usage(program_name);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
