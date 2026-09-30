#ifndef VAXIOM_AST_H
#define VAXIOM_AST_H

#include <stddef.h>
#include <stdio.h>

typedef struct AstNode {
    char *kind;
    char *value;
    size_t line;
    size_t column;
    struct AstNode **children;
    size_t child_count;
    size_t child_capacity;
} AstNode;

AstNode *ast_node_new(const char *kind, const char *value,
                      size_t line, size_t column);
void ast_node_add(AstNode *parent, AstNode *child);
void ast_node_print(FILE *stream, const AstNode *node);
void ast_node_free(AstNode *node);

#endif
