#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void *checked_realloc(void *pointer, size_t size) {
    void *result = realloc(pointer, size);
    if (result == NULL) {
        fputs("vaxiom: out of memory\n", stderr);
        exit(EXIT_FAILURE);
    }
    return result;
}

static char *duplicate_text(const char *text) {
    if (text == NULL) {
        return NULL;
    }
    size_t length = strlen(text);
    char *copy = malloc(length + 1);
    if (copy == NULL) {
        fputs("vaxiom: out of memory\n", stderr);
        exit(EXIT_FAILURE);
    }
    memcpy(copy, text, length + 1);
    return copy;
}

AstNode *ast_node_new(const char *kind, const char *value,
                      size_t line, size_t column) {
    AstNode *node = calloc(1, sizeof(*node));
    if (node == NULL) {
        fputs("vaxiom: out of memory\n", stderr);
        exit(EXIT_FAILURE);
    }
    node->kind = duplicate_text(kind);
    node->value = duplicate_text(value);
    node->line = line;
    node->column = column;
    return node;
}

void ast_node_add(AstNode *parent, AstNode *child) {
    if (parent == NULL || child == NULL) {
        return;
    }
    if (parent->child_count == parent->child_capacity) {
        size_t new_capacity = parent->child_capacity == 0
                                  ? 4
                                  : parent->child_capacity * 2;
        parent->children = checked_realloc(
            parent->children, new_capacity * sizeof(*parent->children));
        parent->child_capacity = new_capacity;
    }
    parent->children[parent->child_count++] = child;
}

static void print_indent(FILE *stream, size_t depth) {
    for (size_t i = 0; i < depth; i++) {
        fputs("  ", stream);
    }
}

static void print_value(FILE *stream, const AstNode *node) {
    if (node->value == NULL) {
        return;
    }
    fputc(' ', stream);
    if (strcmp(node->kind, "String") == 0) {
        fputc('"', stream);
        for (const unsigned char *cursor = (const unsigned char *)node->value;
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
        fputc('"', stream);
    } else {
        fputs(node->value, stream);
    }
}

static void print_node(FILE *stream, const AstNode *node, size_t depth) {
    print_indent(stream, depth);
    fputs(node->kind, stream);
    print_value(stream, node);
    fprintf(stream, " [%zu:%zu]\n", node->line, node->column);
    for (size_t i = 0; i < node->child_count; i++) {
        print_node(stream, node->children[i], depth + 1);
    }
}

void ast_node_print(FILE *stream, const AstNode *node) {
    if (stream != NULL && node != NULL) {
        print_node(stream, node, 0);
    }
}

void ast_node_free(AstNode *node) {
    if (node == NULL) {
        return;
    }
    for (size_t i = 0; i < node->child_count; i++) {
        ast_node_free(node->children[i]);
    }
    free(node->children);
    free(node->kind);
    free(node->value);
    free(node);
}
