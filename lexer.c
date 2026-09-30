#include "lexer.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *source;
    size_t length;
    size_t start;
    size_t current;
    size_t line;
    size_t column;
    size_t token_line;
    size_t token_column;
    TokenArray *tokens;
    char *error;
    size_t error_size;
} Lexer;

typedef struct {
    const char *word;
    TokenType type;
} Keyword;

static const Keyword KEYWORDS[] = {
    {"DECLARE", TOK_DECLARE},       {"ARRAY", TOK_ARRAY},
    {"OF", TOK_OF},                 {"INTEGER", TOK_INTEGER},
    {"STRING", TOK_STRING_TYPE},   {"BOOLEAN", TOK_BOOLEAN},
    {"CHAR", TOK_CHAR},             {"IF", TOK_IF},
    {"THEN", TOK_THEN},             {"ELSE", TOK_ELSE},
    {"ENDIF", TOK_ENDIF},           {"FOR", TOK_FOR},
    {"TO", TOK_TO},                 {"NEXT", TOK_NEXT},
    {"WHILE", TOK_WHILE},           {"DO", TOK_DO},
    {"ENDWHILE", TOK_ENDWHILE},     {"FUNCTION", TOK_FUNCTION},
    {"ENDFUNCTION", TOK_ENDFUNCTION}, {"CALL", TOK_CALL},
    {"INPUT", TOK_INPUT},           {"OUTPUT", TOK_OUTPUT},
    {"TRUE", TOK_TRUE},             {"FALSE", TOK_FALSE},
    {"AND", TOK_AND},               {"OR", TOK_OR},
    {"NOT", TOK_NOT}
};

static char *copy_text(const char *text, size_t length) {
    char *copy = malloc(length + 1);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, text, length);
    copy[length] = '\0';
    return copy;
}

static void set_error(Lexer *lexer, const char *message) {
    if (lexer->error != NULL && lexer->error_size > 0) {
        (void)snprintf(lexer->error, lexer->error_size,
                       "%zu:%zu: %s", lexer->token_line,
                       lexer->token_column, message);
    }
}

static bool grow_tokens(TokenArray *tokens) {
    if (tokens->count < tokens->capacity) {
        return true;
    }

    size_t new_capacity = tokens->capacity == 0 ? 32 : tokens->capacity * 2;
    Token *new_items = realloc(tokens->items, new_capacity * sizeof(*new_items));
    if (new_items == NULL) {
        return false;
    }
    tokens->items = new_items;
    tokens->capacity = new_capacity;
    return true;
}

static bool add_owned_token(Lexer *lexer, TokenType type, char *lexeme) {
    if (!grow_tokens(lexer->tokens)) {
        free(lexeme);
        set_error(lexer, "out of memory while storing tokens");
        return false;
    }

    Token token = {type, lexeme, lexer->token_line, lexer->token_column};
    lexer->tokens->items[lexer->tokens->count++] = token;
    return true;
}

static bool add_token(Lexer *lexer, TokenType type) {
    char *lexeme = copy_text(lexer->source + lexer->start,
                             lexer->current - lexer->start);
    if (lexeme == NULL) {
        set_error(lexer, "out of memory while copying a token");
        return false;
    }
    return add_owned_token(lexer, type, lexeme);
}

static bool at_end(const Lexer *lexer) {
    return lexer->current >= lexer->length;
}

static char peek(const Lexer *lexer) {
    return at_end(lexer) ? '\0' : lexer->source[lexer->current];
}

static char advance(Lexer *lexer) {
    char value = lexer->source[lexer->current++];
    if (value == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    return value;
}

static bool match(Lexer *lexer, char expected) {
    if (at_end(lexer) || peek(lexer) != expected) {
        return false;
    }
    (void)advance(lexer);
    return true;
}

static bool text_equals_ignore_case(const char *left, const char *right) {
    while (*left != '\0' && *right != '\0') {
        unsigned char a = (unsigned char)*left++;
        unsigned char b = (unsigned char)*right++;
        if (toupper(a) != toupper(b)) {
            return false;
        }
    }
    return *left == '\0' && *right == '\0';
}

static TokenType identifier_type(const char *lexeme) {
    size_t keyword_count = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);
    for (size_t i = 0; i < keyword_count; i++) {
        if (text_equals_ignore_case(lexeme, KEYWORDS[i].word)) {
            return KEYWORDS[i].type;
        }
    }
    return TOK_IDENTIFIER;
}

static bool scan_identifier(Lexer *lexer) {
    while (isalnum((unsigned char)peek(lexer)) || peek(lexer) == '_') {
        (void)advance(lexer);
    }

    char *lexeme = copy_text(lexer->source + lexer->start,
                             lexer->current - lexer->start);
    if (lexeme == NULL) {
        set_error(lexer, "out of memory while copying an identifier");
        return false;
    }
    return add_owned_token(lexer, identifier_type(lexeme), lexeme);
}

static bool scan_number(Lexer *lexer) {
    while (isdigit((unsigned char)peek(lexer))) {
        (void)advance(lexer);
    }
    return add_token(lexer, TOK_NUMBER);
}

static bool append_character(char **buffer, size_t *length, size_t *capacity,
                             char value) {
    if (*length + 1 >= *capacity) {
        size_t new_capacity = *capacity == 0 ? 16 : *capacity * 2;
        char *new_buffer = realloc(*buffer, new_capacity);
        if (new_buffer == NULL) {
            return false;
        }
        *buffer = new_buffer;
        *capacity = new_capacity;
    }
    (*buffer)[(*length)++] = value;
    return true;
}

static bool scan_string(Lexer *lexer) {
    char *value = NULL;
    size_t length = 0;
    size_t capacity = 0;

    while (!at_end(lexer) && peek(lexer) != '"') {
        if (peek(lexer) == '\n' || peek(lexer) == '\r') {
            free(value);
            set_error(lexer, "unterminated string literal");
            return false;
        }

        char character = advance(lexer);
        if (character == '\\') {
            if (at_end(lexer)) {
                free(value);
                set_error(lexer, "unterminated escape sequence");
                return false;
            }
            char escaped = advance(lexer);
            switch (escaped) {
                case 'n': character = '\n'; break;
                case 'r': character = '\r'; break;
                case 't': character = '\t'; break;
                case '"': character = '"'; break;
                case '\\': character = '\\'; break;
                default: character = escaped; break;
            }
        }

        if (!append_character(&value, &length, &capacity, character)) {
            free(value);
            set_error(lexer, "out of memory while reading a string");
            return false;
        }
    }

    if (at_end(lexer)) {
        free(value);
        set_error(lexer, "unterminated string literal");
        return false;
    }
    (void)advance(lexer);

    if (!append_character(&value, &length, &capacity, '\0')) {
        free(value);
        set_error(lexer, "out of memory while finishing a string");
        return false;
    }
    return add_owned_token(lexer, TOK_STRING, value);
}

static bool scan_token(Lexer *lexer) {
    char character = advance(lexer);

    if (isalpha((unsigned char)character) || character == '_') {
        return scan_identifier(lexer);
    }
    if (isdigit((unsigned char)character)) {
        return scan_number(lexer);
    }

    switch (character) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            return true;
        case '#':
            while (!at_end(lexer) && peek(lexer) != '\n') {
                (void)advance(lexer);
            }
            return true;
        case '/':
            if (match(lexer, '/')) {
                while (!at_end(lexer) && peek(lexer) != '\n') {
                    (void)advance(lexer);
                }
                return true;
            }
            return add_token(lexer, TOK_SLASH);
        case ':': return add_token(lexer, TOK_COLON);
        case ';': return add_token(lexer, TOK_SEMICOLON);
        case ',': return add_token(lexer, TOK_COMMA);
        case '[': return add_token(lexer, TOK_LEFT_BRACKET);
        case ']': return add_token(lexer, TOK_RIGHT_BRACKET);
        case '(': return add_token(lexer, TOK_LEFT_PAREN);
        case ')': return add_token(lexer, TOK_RIGHT_PAREN);
        case '+': return add_token(lexer, TOK_PLUS);
        case '-': return add_token(lexer, TOK_MINUS);
        case '*': return add_token(lexer, TOK_STAR);
        case '"': return scan_string(lexer);
        case '=':
            (void)match(lexer, '=');
            return add_token(lexer, TOK_EQUAL);
        case '<':
            if (match(lexer, '-')) {
                if (!match(lexer, '-')) {
                    set_error(lexer, "expected a second '-' in assignment operator '<--'");
                    return false;
                }
                return add_token(lexer, TOK_ASSIGN);
            }
            if (match(lexer, '>')) return add_token(lexer, TOK_NOT_EQUAL);
            if (match(lexer, '=')) return add_token(lexer, TOK_LESS_EQUAL);
            return add_token(lexer, TOK_LESS);
        case '>':
            if (match(lexer, '=')) return add_token(lexer, TOK_GREATER_EQUAL);
            return add_token(lexer, TOK_GREATER);
        default: {
            char message[80];
            if (isprint((unsigned char)character)) {
                (void)snprintf(message, sizeof(message),
                               "unexpected character '%c'", character);
            } else {
                (void)snprintf(message, sizeof(message),
                               "unexpected byte 0x%02X",
                               (unsigned int)(unsigned char)character);
            }
            set_error(lexer, message);
            return false;
        }
    }
}

static bool read_file(const char *path, char **source, size_t *length,
                      char *error, size_t error_size) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        if (error != NULL && error_size > 0) {
            (void)snprintf(error, error_size, "cannot open '%s': %s",
                           path, strerror(errno));
        }
        return false;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        if (error != NULL && error_size > 0) {
            (void)snprintf(error, error_size, "cannot seek '%s'", path);
        }
        (void)fclose(file);
        return false;
    }
    long file_size = ftell(file);
    if (file_size < 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (error != NULL && error_size > 0) {
            (void)snprintf(error, error_size, "cannot determine the size of '%s'", path);
        }
        (void)fclose(file);
        return false;
    }

    char *buffer = malloc((size_t)file_size + 1);
    if (buffer == NULL) {
        if (error != NULL && error_size > 0) {
            (void)snprintf(error, error_size, "out of memory while reading '%s'", path);
        }
        (void)fclose(file);
        return false;
    }

    size_t bytes_read = fread(buffer, 1, (size_t)file_size, file);
    if (bytes_read != (size_t)file_size && ferror(file)) {
        if (error != NULL && error_size > 0) {
            (void)snprintf(error, error_size, "cannot read '%s'", path);
        }
        free(buffer);
        (void)fclose(file);
        return false;
    }

    buffer[bytes_read] = '\0';
    (void)fclose(file);
    *source = buffer;
    *length = bytes_read;
    return true;
}

bool lexer_tokenize_file(const char *path, TokenArray *tokens,
                         char *error, size_t error_size) {
    if (tokens == NULL) {
        return false;
    }
    tokens->items = NULL;
    tokens->count = 0;
    tokens->capacity = 0;
    if (error != NULL && error_size > 0) {
        error[0] = '\0';
    }

    char *source = NULL;
    size_t length = 0;
    if (!read_file(path, &source, &length, error, error_size)) {
        return false;
    }

    Lexer lexer = {source, length, 0, 0, 1, 1, 1, 1,
                   tokens, error, error_size};
    bool success = true;
    while (!at_end(&lexer)) {
        lexer.start = lexer.current;
        lexer.token_line = lexer.line;
        lexer.token_column = lexer.column;
        if (!scan_token(&lexer)) {
            success = false;
            break;
        }
    }

    if (success) {
        lexer.start = lexer.current;
        lexer.token_line = lexer.line;
        lexer.token_column = lexer.column;
        char *empty = copy_text("", 0);
        success = empty != NULL && add_owned_token(&lexer, TOK_EOF, empty);
        if (!success && error != NULL && error_size > 0 && error[0] == '\0') {
            (void)snprintf(error, error_size, "out of memory while finishing tokenization");
        }
    }

    free(source);
    if (!success) {
        token_array_free(tokens);
    }
    return success;
}

void token_array_free(TokenArray *tokens) {
    if (tokens == NULL) {
        return;
    }
    for (size_t i = 0; i < tokens->count; i++) {
        free(tokens->items[i].lexeme);
    }
    free(tokens->items);
    tokens->items = NULL;
    tokens->count = 0;
    tokens->capacity = 0;
}

const char *token_type_name(TokenType type) {
    static const char *const names[] = {
        "EOF", "IDENTIFIER", "NUMBER", "STRING_LITERAL",
        "DECLARE", "ARRAY", "OF", "INTEGER", "STRING", "BOOLEAN", "CHAR",
        "IF", "THEN", "ELSE", "ENDIF", "FOR", "TO", "NEXT", "WHILE",
        "DO", "ENDWHILE", "FUNCTION", "ENDFUNCTION", "CALL", "INPUT",
        "OUTPUT", "TRUE", "FALSE", "AND", "OR", "NOT", "COLON",
        "SEMICOLON", "COMMA", "LEFT_BRACKET", "RIGHT_BRACKET", "LEFT_PAREN",
        "RIGHT_PAREN", "PLUS", "MINUS", "STAR", "SLASH", "ASSIGN",
        "EQUAL", "NOT_EQUAL", "LESS", "LESS_EQUAL", "GREATER", "GREATER_EQUAL"
    };
    size_t count = sizeof(names) / sizeof(names[0]);
    return (size_t)type < count ? names[type] : "UNKNOWN";
}

static void print_escaped(FILE *stream, const char *text) {
    for (const unsigned char *cursor = (const unsigned char *)text;
         *cursor != '\0'; cursor++) {
        switch (*cursor) {
            case '\n': fputs("\\n", stream); break;
            case '\r': fputs("\\r", stream); break;
            case '\t': fputs("\\t", stream); break;
            case '\\': fputs("\\\\", stream); break;
            case '"': fputs("\\\"", stream); break;
            default: fputc((int)*cursor, stream); break;
        }
    }
}

void token_array_print(FILE *stream, const TokenArray *tokens) {
    if (stream == NULL || tokens == NULL) {
        return;
    }
    for (size_t i = 0; i < tokens->count; i++) {
        const Token *token = &tokens->items[i];
        if (token->type == TOK_EOF) {
            fprintf(stream, "%zu:%zu %s\n", token->line, token->column,
                    token_type_name(token->type));
            continue;
        }
        fprintf(stream, "%zu:%zu %-15s ", token->line, token->column,
                token_type_name(token->type));
        if (token->type == TOK_STRING) {
            fputc('"', stream);
            print_escaped(stream, token->lexeme);
            fputc('"', stream);
        } else {
            fputs(token->lexeme, stream);
        }
        fputc('\n', stream);
    }
}
