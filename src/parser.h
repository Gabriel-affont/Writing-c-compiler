#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    AST_INT_LITERAL,
    AST_RETURN_STATEMENT,
    AST_FUNCTION_DECLARATION,
    AST_PROGRAM
} ASTNodeType;

typedef enum {
    UNARY_NEGATE,
    UNARY_COMPLEMENT,
    UNARY_NOT
} UnaryOp;

typedef enum {
    BIN_ADD,
    BIN_SUB,
    BIN_MUL,
    BIN_DIV
} BinaryOp;

typedef enum {
    EXP_INT_LITERAL,
    EXP_UNARY,
    EXP_BINARY

} EXPType;



typedef struct ASTExp {
    ExpType type;
    union {
        int int_val;
        struct {
            UnaryOp op;
            struct ASTExp *sub_exp;
        } unary;
        struct {
            BinaryOp op;
            struct ASTExp *left;
            struct ASTExp *right;
        } binary;
        
    };
} ASTExp;

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
void printf_ast(const ASTProgram *program);
#endif