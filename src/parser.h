#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    AST_INT_LITERAL,
    AST_RETURN_STATEMENT,
    AST_FUNCTION_DECLARATION,
    AST_PROGRAM
} ASTNodeType;

typedef struct ASTExp {
    int value;
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
ASTProgram *parse_tokens(TokenList *tokens);
void free_ast(ASTProgram *program);
#endif