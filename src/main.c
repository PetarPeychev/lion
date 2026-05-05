#include "lion.h"
#include <stdio.h>
#include <string.h>

void print_usage(char *program_name) {
    fprintf(stderr, "Usage: %s <command> [ARGUMENTS]\n", program_name);
    fprintf(stderr, "\n");
    fprintf(stderr, "Commands:\n");
    fprintf(stderr, "    run [FILE] - Run a lion program.\n");
}

char *read_file(char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long position = ftell(file);
    if (position < 0) {
        fprintf(stderr, "Could not read file '%s'\n", path);
        fclose(file);
        return NULL;
    }
    size_t length = (size_t)position;
    rewind(file);

    char *buffer = malloc(length + 1);
    if (!buffer) {
        fprintf(stderr, "Not enough memory to read '%s'\n", path);
        fclose(file);
        return NULL;
    }
    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);
    return buffer;
}

int run(char *path) {
    char *source = read_file(path);
    if (source == NULL) {
        fprintf(stderr, "Failed to read file '%s'.\n", path);
        return EXIT_FAILURE;
    }

    printf("%s\n", source);

    free(source);
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
