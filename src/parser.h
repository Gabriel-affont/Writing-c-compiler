#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    UNARY_NEGATE,
    UNARY_COMPLEMENT,
    UNARY_NOT
} UnaryOp;

typedef enum {
    BINARY_ADD,
    BINARY_SUBTRACT,
    BINARY_MULTIPLY,
    BINARY_DIVIDE,
    
    BINARY_EQUAL,
    BINARY_NOT_EQUAL,
    BINARY_LESS_THAN,
    BINARY_LESS_EQUAL,
    BINARY_GREATER_THAN,
    BINARY_GREATER_EQUAL,
    BINARY_LOGICAL_AND,
    BINARY_LOGICAL_OR
} BinaryOp;

typedef enum {
    EXP_INT_LITERAL,
    EXP_UNARY,
    EXP_BINARY
} ExpType;

typedef struct ASTExp ASTExp;

struct ASTExp {
    ExpType type;
    union {
        int int_val;
        struct {
            UnaryOp op;
            ASTExp *sub_exp;
        } unary;
        struct {
            BinaryOp op;
            ASTExp *left;
            ASTExp *right;
        } binary;
    };
};

typedef struct ASTStatement {
    ASTExp *exp;
} ASTStatement;

typedef struct ASTFunction {
    char *name;
    ASTStatement *statement;
} ASTFunction;

typedef struct ASTProgram {
    ASTFunction *function;
} ASTProgram;

ASTProgram *parse(TokenList *tokens);
void free_ast(ASTProgram *program);
void free_exp(ASTExp *exp);
void print_ast(const ASTProgram *program);

#endif