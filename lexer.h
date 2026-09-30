#ifndef VAXIOM_LEXER_H
#define VAXIOM_LEXER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef enum {
    TOK_EOF,
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_STRING,

    TOK_DECLARE,
    TOK_ARRAY,
    TOK_OF,
    TOK_INTEGER,
    TOK_STRING_TYPE,
    TOK_BOOLEAN,
    TOK_CHAR,
    TOK_IF,
    TOK_THEN,
    TOK_ELSE,
    TOK_ENDIF,
    TOK_FOR,
    TOK_TO,
    TOK_NEXT,
    TOK_WHILE,
    TOK_DO,
    TOK_ENDWHILE,
    TOK_FUNCTION,
    TOK_ENDFUNCTION,
    TOK_CALL,
    TOK_INPUT,
    TOK_OUTPUT,
    TOK_TRUE,
    TOK_FALSE,
    TOK_AND,
    TOK_OR,
    TOK_NOT,

    TOK_COLON,
    TOK_SEMICOLON,
    TOK_COMMA,
    TOK_LEFT_BRACKET,
    TOK_RIGHT_BRACKET,
    TOK_LEFT_PAREN,
    TOK_RIGHT_PAREN,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_ASSIGN,
    TOK_EQUAL,
    TOK_NOT_EQUAL,
    TOK_LESS,
    TOK_LESS_EQUAL,
    TOK_GREATER,
    TOK_GREATER_EQUAL
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
    size_t line;
    size_t column;
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t capacity;
} TokenArray;

bool lexer_tokenize_file(const char *path, TokenArray *tokens,
                         char *error, size_t error_size);
void token_array_free(TokenArray *tokens);
void token_array_print(FILE *stream, const TokenArray *tokens);
const char *token_type_name(TokenType type);

#endif
