#include "lexer.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(FILE *stream, const char *program) {
    fprintf(stream, "Usage: %s [source-file] [token-output-file]\n", program);
}

int main(int argc, char **argv) {
    if (argc > 3) {
        print_usage(stderr, argv[0]);
        return EXIT_FAILURE;
    }

    const char *source_path = argc >= 2 ? argv[1] : "input.txt";
    const char *output_path = argc >= 3 ? argv[2] : "output.txt";

    TokenArray tokens;
    char lexer_error[256];
    if (!lexer_tokenize_file(source_path, &tokens,
                             lexer_error, sizeof(lexer_error))) {
        fprintf(stderr, "%s: lexical error: %s\n", source_path, lexer_error);
        return EXIT_FAILURE;
    }

    FILE *output = fopen(output_path, "w");
    if (output == NULL) {
        fprintf(stderr, "cannot open '%s' for writing: %s\n",
                output_path, strerror(errno));
        token_array_free(&tokens);
        return EXIT_FAILURE;
    }

    token_array_print(output, &tokens);
    if (fclose(output) != 0) {
        fprintf(stderr, "failed to finish writing '%s'\n", output_path);
        token_array_free(&tokens);
        return EXIT_FAILURE;
    }

    printf("Wrote %zu tokens to %s\n", tokens.count, output_path);
    token_array_free(&tokens);
    return EXIT_SUCCESS;
}
