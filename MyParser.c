#include "ast.h"
#include "lexer.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const TokenArray *tokens;
    size_t current;
    size_t errors;
    const char *source_path;
} Parser;

static AstNode *parse_statement(Parser *parser);
static AstNode *parse_expression(Parser *parser);

static const Token *peek(const Parser *parser) {
    return &parser->tokens->items[parser->current];
}

static const Token *previous(const Parser *parser) {
    return &parser->tokens->items[parser->current - 1];
}

static bool check(const Parser *parser, TokenType type) {
    return peek(parser)->type == type;
}

static bool at_end(const Parser *parser) {
    return check(parser, TOK_EOF);
}

static const Token *advance(Parser *parser) {
    if (!at_end(parser)) {
        parser->current++;
    }
    return previous(parser);
}

static bool match(Parser *parser, TokenType type) {
    if (!check(parser, type)) {
        return false;
    }
    (void)advance(parser);
    return true;
}

static void parser_error_at(Parser *parser, const Token *token,
                            const char *format, ...) {
    fprintf(stderr, "%s:%zu:%zu: parse error",
            parser->source_path, token->line, token->column);
    if (token->type == TOK_EOF) {
        fputs(" at end of file: ", stderr);
    } else {
        fprintf(stderr, " at '%s': ", token->lexeme);
    }

    va_list arguments;
    va_start(arguments, format);
    vfprintf(stderr, format, arguments);
    va_end(arguments);
    fputc('\n', stderr);
    parser->errors++;
}

static const Token *expect(Parser *parser, TokenType type,
                           const char *description) {
    if (check(parser, type)) {
        return advance(parser);
    }
    parser_error_at(parser, peek(parser), "expected %s, found %s",
                    description, token_type_name(peek(parser)->type));
    return NULL;
}

static AstNode *new_node_at(const char *kind, const char *value,
                            const Token *token) {
    return ast_node_new(kind, value, token->line, token->column);
}

static AstNode *parse_scalar_type(Parser *parser) {
    TokenType type = peek(parser)->type;
    if (type != TOK_INTEGER && type != TOK_STRING_TYPE &&
        type != TOK_BOOLEAN && type != TOK_CHAR) {
        parser_error_at(parser, peek(parser),
                        "expected INTEGER, STRING, BOOLEAN, or CHAR");
        return NULL;
    }
    const Token *token = advance(parser);
    return new_node_at("Type", token_type_name(token->type), token);
}

static AstNode *parse_type(Parser *parser) {
    if (!match(parser, TOK_ARRAY)) {
        return parse_scalar_type(parser);
    }

    const Token *array_token = previous(parser);
    if (expect(parser, TOK_LEFT_BRACKET, "'[' after ARRAY") == NULL) {
        return NULL;
    }
    const Token *lower = expect(parser, TOK_NUMBER, "an integer lower bound");
    if (lower == NULL || expect(parser, TOK_COLON, "':' between array bounds") == NULL) {
        return NULL;
    }
    const Token *upper = expect(parser, TOK_NUMBER, "an integer upper bound");
    if (upper == NULL || expect(parser, TOK_RIGHT_BRACKET, "']' after array bounds") == NULL ||
        expect(parser, TOK_OF, "OF after array bounds") == NULL) {
        return NULL;
    }
    AstNode *element_type = parse_scalar_type(parser);
    if (element_type == NULL) {
        return NULL;
    }

    AstNode *array = new_node_at("ArrayType", NULL, array_token);
    ast_node_add(array, new_node_at("LowerBound", lower->lexeme, lower));
    ast_node_add(array, new_node_at("UpperBound", upper->lexeme, upper));
    ast_node_add(array, element_type);
    return array;
}

static AstNode *parse_call(Parser *parser, const Token *call_token) {
    const Token *name = expect(parser, TOK_IDENTIFIER,
                               "a function name after CALL");
    if (name == NULL) {
        return NULL;
    }

    AstNode *call = new_node_at("Call", name->lexeme, call_token);
    if (!match(parser, TOK_LEFT_PAREN)) {
        return call;
    }

    if (!check(parser, TOK_RIGHT_PAREN)) {
        do {
            AstNode *argument = parse_expression(parser);
            if (argument == NULL) {
                ast_node_free(call);
                return NULL;
            }
            ast_node_add(call, argument);
        } while (match(parser, TOK_COMMA));
    }

    if (expect(parser, TOK_RIGHT_PAREN, "')' after function arguments") == NULL) {
        ast_node_free(call);
        return NULL;
    }
    return call;
}

static AstNode *parse_primary(Parser *parser) {
    if (match(parser, TOK_NUMBER)) {
        return new_node_at("Number", previous(parser)->lexeme, previous(parser));
    }
    if (match(parser, TOK_STRING)) {
        return new_node_at("String", previous(parser)->lexeme, previous(parser));
    }
    if (match(parser, TOK_TRUE) || match(parser, TOK_FALSE)) {
        return new_node_at("Boolean", token_type_name(previous(parser)->type),
                           previous(parser));
    }
    if (match(parser, TOK_CALL)) {
        return parse_call(parser, previous(parser));
    }
    if (match(parser, TOK_LEFT_PAREN)) {
        const Token *opening = previous(parser);
        AstNode *expression = parse_expression(parser);
        if (expression == NULL ||
            expect(parser, TOK_RIGHT_PAREN, "')' after expression") == NULL) {
            ast_node_free(expression);
            return NULL;
        }
        AstNode *group = new_node_at("Group", NULL, opening);
        ast_node_add(group, expression);
        return group;
    }
    if (match(parser, TOK_IDENTIFIER)) {
        const Token *name = previous(parser);
        AstNode *value = new_node_at("Identifier", name->lexeme, name);

        while (match(parser, TOK_LEFT_BRACKET)) {
            const Token *opening = previous(parser);
            AstNode *index = parse_expression(parser);
            if (index == NULL ||
                expect(parser, TOK_RIGHT_BRACKET, "']' after array index") == NULL) {
                ast_node_free(value);
                ast_node_free(index);
                return NULL;
            }
            AstNode *access = new_node_at("ArrayAccess", NULL, opening);
            ast_node_add(access, value);
            ast_node_add(access, index);
            value = access;
        }
        return value;
    }

    parser_error_at(parser, peek(parser), "expected an expression");
    if (!at_end(parser)) {
        (void)advance(parser);
    }
    return NULL;
}

static AstNode *parse_unary(Parser *parser) {
    if (match(parser, TOK_NOT) || match(parser, TOK_MINUS) ||
        match(parser, TOK_PLUS)) {
        const Token *operator_token = previous(parser);
        AstNode *operand = parse_unary(parser);
        if (operand == NULL) {
            return NULL;
        }
        AstNode *unary = new_node_at("Unary", operator_token->lexeme,
                                     operator_token);
        ast_node_add(unary, operand);
        return unary;
    }
    return parse_primary(parser);
}

static AstNode *binary_node(const Token *operator_token,
                            AstNode *left, AstNode *right) {
    AstNode *binary = new_node_at("Binary", operator_token->lexeme,
                                  operator_token);
    ast_node_add(binary, left);
    ast_node_add(binary, right);
    return binary;
}

static AstNode *parse_factor(Parser *parser) {
    AstNode *expression = parse_unary(parser);
    while (expression != NULL &&
           (match(parser, TOK_STAR) || match(parser, TOK_SLASH))) {
        const Token *operator_token = previous(parser);
        AstNode *right = parse_unary(parser);
        if (right == NULL) {
            ast_node_free(expression);
            return NULL;
        }
        expression = binary_node(operator_token, expression, right);
    }
    return expression;
}

static AstNode *parse_term(Parser *parser) {
    AstNode *expression = parse_factor(parser);
    while (expression != NULL &&
           (match(parser, TOK_PLUS) || match(parser, TOK_MINUS))) {
        const Token *operator_token = previous(parser);
        AstNode *right = parse_factor(parser);
        if (right == NULL) {
            ast_node_free(expression);
            return NULL;
        }
        expression = binary_node(operator_token, expression, right);
    }
    return expression;
}

static AstNode *parse_comparison(Parser *parser) {
    AstNode *expression = parse_term(parser);
    while (expression != NULL &&
           (match(parser, TOK_LESS) || match(parser, TOK_LESS_EQUAL) ||
            match(parser, TOK_GREATER) || match(parser, TOK_GREATER_EQUAL))) {
        const Token *operator_token = previous(parser);
        AstNode *right = parse_term(parser);
        if (right == NULL) {
            ast_node_free(expression);
            return NULL;
        }
        expression = binary_node(operator_token, expression, right);
    }
    return expression;
}

static AstNode *parse_equality(Parser *parser) {
    AstNode *expression = parse_comparison(parser);
    while (expression != NULL &&
           (match(parser, TOK_EQUAL) || match(parser, TOK_NOT_EQUAL))) {
        const Token *operator_token = previous(parser);
        AstNode *right = parse_comparison(parser);
        if (right == NULL) {
            ast_node_free(expression);
            return NULL;
        }
        expression = binary_node(operator_token, expression, right);
    }
    return expression;
}

static AstNode *parse_and(Parser *parser) {
    AstNode *expression = parse_equality(parser);
    while (expression != NULL && match(parser, TOK_AND)) {
        const Token *operator_token = previous(parser);
        AstNode *right = parse_equality(parser);
        if (right == NULL) {
            ast_node_free(expression);
            return NULL;
        }
        expression = binary_node(operator_token, expression, right);
    }
    return expression;
}

static AstNode *parse_expression(Parser *parser) {
    AstNode *expression = parse_and(parser);
    while (expression != NULL && match(parser, TOK_OR)) {
        const Token *operator_token = previous(parser);
        AstNode *right = parse_and(parser);
        if (right == NULL) {
            ast_node_free(expression);
            return NULL;
        }
        expression = binary_node(operator_token, expression, right);
    }
    return expression;
}

static AstNode *parse_lvalue(Parser *parser) {
    const Token *name = expect(parser, TOK_IDENTIFIER, "an identifier");
    if (name == NULL) {
        return NULL;
    }
    AstNode *value = new_node_at("Identifier", name->lexeme, name);

    while (match(parser, TOK_LEFT_BRACKET)) {
        const Token *opening = previous(parser);
        AstNode *index = parse_expression(parser);
        if (index == NULL ||
            expect(parser, TOK_RIGHT_BRACKET, "']' after array index") == NULL) {
            ast_node_free(value);
            ast_node_free(index);
            return NULL;
        }
        AstNode *access = new_node_at("ArrayAccess", NULL, opening);
        ast_node_add(access, value);
        ast_node_add(access, index);
        value = access;
    }
    return value;
}

static AstNode *parse_declaration(Parser *parser, const Token *keyword) {
    const Token *name = expect(parser, TOK_IDENTIFIER,
                               "an identifier after DECLARE");
    if (name == NULL || expect(parser, TOK_COLON, "':' after the identifier") == NULL) {
        return NULL;
    }
    AstNode *type = parse_type(parser);
    if (type == NULL) {
        return NULL;
    }

    AstNode *declaration = new_node_at("Declare", name->lexeme, keyword);
    ast_node_add(declaration, type);
    return declaration;
}

static AstNode *parse_input(Parser *parser, const Token *keyword) {
    AstNode *target = parse_lvalue(parser);
    if (target == NULL) {
        return NULL;
    }
    AstNode *input = new_node_at("Input", NULL, keyword);
    ast_node_add(input, target);
    return input;
}

static AstNode *parse_output(Parser *parser, const Token *keyword) {
    AstNode *value = parse_expression(parser);
    if (value == NULL) {
        return NULL;
    }
    AstNode *output = new_node_at("Output", NULL, keyword);
    ast_node_add(output, value);
    return output;
}

static bool is_block_end(TokenType type) {
    return type == TOK_ELSE || type == TOK_ENDIF || type == TOK_NEXT ||
           type == TOK_ENDWHILE || type == TOK_ENDFUNCTION;
}

static bool is_statement_start(TokenType type) {
    return type == TOK_DECLARE || type == TOK_INPUT || type == TOK_OUTPUT ||
           type == TOK_IF || type == TOK_FOR || type == TOK_WHILE ||
           type == TOK_FUNCTION || type == TOK_IDENTIFIER ||
           type == TOK_NUMBER || type == TOK_STRING || type == TOK_TRUE ||
           type == TOK_FALSE || type == TOK_CALL || type == TOK_LEFT_PAREN ||
           type == TOK_NOT || type == TOK_MINUS || type == TOK_PLUS;
}

static void synchronize(Parser *parser) {
    while (!at_end(parser)) {
        if (match(parser, TOK_SEMICOLON)) {
            return;
        }
        if (is_statement_start(peek(parser)->type) ||
            is_block_end(peek(parser)->type)) {
            return;
        }
        (void)advance(parser);
    }
}

static AstNode *parse_block(Parser *parser, TokenType first_stop,
                            TokenType second_stop, const Token *owner) {
    AstNode *body = new_node_at("Body", NULL, owner);
    while (!at_end(parser) && !check(parser, first_stop) &&
           !check(parser, second_stop)) {
        if (match(parser, TOK_SEMICOLON)) {
            continue;
        }
        size_t before = parser->current;
        AstNode *statement = parse_statement(parser);
        if (statement != NULL) {
            ast_node_add(body, statement);
        } else {
            synchronize(parser);
        }
        if (parser->current == before && !at_end(parser)) {
            (void)advance(parser);
        }
    }
    return body;
}

static AstNode *parse_if(Parser *parser, const Token *keyword) {
    AstNode *condition = parse_expression(parser);
    if (condition == NULL) {
        return NULL;
    }
    (void)match(parser, TOK_THEN);

    AstNode *then_body = parse_block(parser, TOK_ELSE, TOK_ENDIF, keyword);
    AstNode *otherwise = NULL;
    if (match(parser, TOK_ELSE)) {
        const Token *else_token = previous(parser);
        otherwise = new_node_at("Else", NULL, else_token);
        ast_node_add(otherwise,
                     parse_block(parser, TOK_ENDIF, TOK_ENDIF, else_token));
    }

    if (expect(parser, TOK_ENDIF, "ENDIF to close IF") == NULL) {
        ast_node_free(condition);
        ast_node_free(then_body);
        ast_node_free(otherwise);
        return NULL;
    }

    AstNode *statement = new_node_at("If", NULL, keyword);
    AstNode *then_node = new_node_at("Then", NULL, keyword);
    ast_node_add(then_node, then_body);
    ast_node_add(statement, condition);
    ast_node_add(statement, then_node);
    ast_node_add(statement, otherwise);
    return statement;
}

static AstNode *parse_for(Parser *parser, const Token *keyword) {
    const Token *variable = expect(parser, TOK_IDENTIFIER,
                                   "a loop variable after FOR");
    if (variable == NULL ||
        expect(parser, TOK_ASSIGN, "'<--' after the loop variable") == NULL) {
        return NULL;
    }
    AstNode *start = parse_expression(parser);
    if (start == NULL || expect(parser, TOK_TO, "TO in FOR loop") == NULL) {
        ast_node_free(start);
        return NULL;
    }
    AstNode *end = parse_expression(parser);
    if (end == NULL) {
        ast_node_free(start);
        return NULL;
    }

    AstNode *body = parse_block(parser, TOK_NEXT, TOK_NEXT, keyword);
    if (expect(parser, TOK_NEXT, "NEXT to close FOR") == NULL) {
        ast_node_free(start);
        ast_node_free(end);
        ast_node_free(body);
        return NULL;
    }

    AstNode *statement = new_node_at("For", variable->lexeme, keyword);
    AstNode *start_node = new_node_at("Start", NULL, keyword);
    AstNode *end_node = new_node_at("End", NULL, keyword);
    ast_node_add(start_node, start);
    ast_node_add(end_node, end);
    ast_node_add(statement, start_node);
    ast_node_add(statement, end_node);
    ast_node_add(statement, body);
    return statement;
}

static AstNode *parse_while(Parser *parser, const Token *keyword) {
    AstNode *condition = parse_expression(parser);
    if (condition == NULL) {
        return NULL;
    }
    (void)match(parser, TOK_DO);

    AstNode *body = parse_block(parser, TOK_ENDWHILE, TOK_ENDWHILE, keyword);
    if (expect(parser, TOK_ENDWHILE, "ENDWHILE to close WHILE") == NULL) {
        ast_node_free(condition);
        ast_node_free(body);
        return NULL;
    }

    AstNode *statement = new_node_at("While", NULL, keyword);
    ast_node_add(statement, condition);
    ast_node_add(statement, body);
    return statement;
}

static AstNode *parse_function(Parser *parser, const Token *keyword) {
    const Token *name = expect(parser, TOK_IDENTIFIER,
                               "a function name after FUNCTION");
    if (name == NULL ||
        expect(parser, TOK_LEFT_PAREN, "'(' after the function name") == NULL) {
        return NULL;
    }

    AstNode *parameters = new_node_at("Parameters", NULL, keyword);
    if (!check(parser, TOK_RIGHT_PAREN)) {
        do {
            const Token *parameter_name = expect(parser, TOK_IDENTIFIER,
                                                  "a parameter name");
            if (parameter_name == NULL) {
                ast_node_free(parameters);
                return NULL;
            }
            AstNode *parameter = new_node_at("Parameter", parameter_name->lexeme,
                                             parameter_name);
            if (match(parser, TOK_COLON)) {
                AstNode *type = parse_type(parser);
                if (type == NULL) {
                    ast_node_free(parameter);
                    ast_node_free(parameters);
                    return NULL;
                }
                ast_node_add(parameter, type);
            }
            ast_node_add(parameters, parameter);
        } while (match(parser, TOK_COMMA));
    }

    if (expect(parser, TOK_RIGHT_PAREN, "')' after parameters") == NULL) {
        ast_node_free(parameters);
        return NULL;
    }
    AstNode *body = parse_block(parser, TOK_ENDFUNCTION, TOK_ENDFUNCTION, keyword);
    if (expect(parser, TOK_ENDFUNCTION,
               "ENDFUNCTION to close FUNCTION") == NULL) {
        ast_node_free(parameters);
        ast_node_free(body);
        return NULL;
    }

    AstNode *function = new_node_at("Function", name->lexeme, keyword);
    ast_node_add(function, parameters);
    ast_node_add(function, body);
    return function;
}

static AstNode *parse_identifier_statement(Parser *parser) {
    size_t start = parser->current;
    AstNode *target = parse_lvalue(parser);
    if (target == NULL) {
        return NULL;
    }

    if (match(parser, TOK_ASSIGN)) {
        const Token *assignment = previous(parser);
        AstNode *value = parse_expression(parser);
        if (value == NULL) {
            ast_node_free(target);
            return NULL;
        }
        AstNode *statement = new_node_at("Assign", NULL, assignment);
        ast_node_add(statement, target);
        ast_node_add(statement, value);
        return statement;
    }

    ast_node_free(target);
    parser->current = start;
    AstNode *expression = parse_expression(parser);
    if (expression == NULL) {
        return NULL;
    }
    AstNode *statement = new_node_at("ExpressionStatement", NULL,
                                     &parser->tokens->items[start]);
    ast_node_add(statement, expression);
    return statement;
}

static AstNode *parse_expression_statement(Parser *parser) {
    const Token *start = peek(parser);
    AstNode *expression = parse_expression(parser);
    if (expression == NULL) {
        return NULL;
    }
    AstNode *statement = new_node_at("ExpressionStatement", NULL, start);
    ast_node_add(statement, expression);
    return statement;
}

static AstNode *parse_statement(Parser *parser) {
    AstNode *statement = NULL;
    if (match(parser, TOK_DECLARE)) {
        statement = parse_declaration(parser, previous(parser));
    } else if (match(parser, TOK_INPUT)) {
        statement = parse_input(parser, previous(parser));
    } else if (match(parser, TOK_OUTPUT)) {
        statement = parse_output(parser, previous(parser));
    } else if (match(parser, TOK_IF)) {
        statement = parse_if(parser, previous(parser));
    } else if (match(parser, TOK_FOR)) {
        statement = parse_for(parser, previous(parser));
    } else if (match(parser, TOK_WHILE)) {
        statement = parse_while(parser, previous(parser));
    } else if (match(parser, TOK_FUNCTION)) {
        statement = parse_function(parser, previous(parser));
    } else if (check(parser, TOK_IDENTIFIER)) {
        statement = parse_identifier_statement(parser);
    } else if (is_statement_start(peek(parser)->type)) {
        statement = parse_expression_statement(parser);
    } else {
        parser_error_at(parser, peek(parser), "expected a statement");
        if (!at_end(parser)) {
            (void)advance(parser);
        }
        return NULL;
    }

    (void)match(parser, TOK_SEMICOLON);
    return statement;
}

static AstNode *parse_program(Parser *parser) {
    const Token *first = peek(parser);
    AstNode *program = new_node_at("Program", NULL, first);

    while (!at_end(parser)) {
        if (match(parser, TOK_SEMICOLON)) {
            continue;
        }
        size_t before = parser->current;
        AstNode *statement = parse_statement(parser);
        if (statement != NULL) {
            ast_node_add(program, statement);
        } else {
            synchronize(parser);
        }
        if (parser->current == before && !at_end(parser)) {
            (void)advance(parser);
        }
    }
    return program;
}

static void print_usage(FILE *stream, const char *program) {
    fprintf(stream, "Usage: %s [--tokens] [source-file]\n", program);
}

int main(int argc, char **argv) {
    bool show_tokens = false;
    const char *source_path = "input.txt";
    bool source_was_set = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tokens") == 0) {
            show_tokens = true;
        } else if (!source_was_set) {
            source_path = argv[i];
            source_was_set = true;
        } else {
            print_usage(stderr, argv[0]);
            return EXIT_FAILURE;
        }
    }

    TokenArray tokens;
    char lexer_error[256];
    if (!lexer_tokenize_file(source_path, &tokens,
                             lexer_error, sizeof(lexer_error))) {
        fprintf(stderr, "%s: lexical error: %s\n", source_path, lexer_error);
        return EXIT_FAILURE;
    }

    if (show_tokens) {
        token_array_print(stdout, &tokens);
        fputc('\n', stdout);
    }

    Parser parser = {&tokens, 0, 0, source_path};
    AstNode *program = parse_program(&parser);
    int result = EXIT_SUCCESS;
    if (parser.errors == 0) {
        ast_node_print(stdout, program);
    } else {
        fprintf(stderr, "%s: parsing failed with %zu error%s\n",
                source_path, parser.errors, parser.errors == 1 ? "" : "s");
        result = EXIT_FAILURE;
    }

    ast_node_free(program);
    token_array_free(&tokens);
    return result;
}
