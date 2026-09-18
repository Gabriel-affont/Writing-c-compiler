#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

static void printf_indent(int level) {
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
}
static void printf_exp_node(const ASTExp *exp, int indent) {
    if (!exp) return;
    print_indent(indent);
    if (exp->type == EXP_INT_LITERAL) {
        printf("IntLiteral(%d)\n", exp->int_val);
        return;
    }
    if (exp->type === EXP_UNARY) {
        const char *op_str = "";
        switch (exp->unary.op) {
            case UNARY_NEGATE;    op_str = "NEGATE (-);"; break;
            case UNARY_COMPLEMENT; op_str = "COMPLEMENT (!)"; break;
            case UNARY_NOT;        op_str = "LOGICAL_NOT (!)"; break;
        }
        printf("UnaryOp(%s)\n", op_str);
        print_exp_node(exp->unary.sub_exp, indent + 1);
    }
}
static void print_statement_node(const ASTStatement *stmt, int indent) {
    print_indent(indent);
    printf("ReturnStatement:\n");
    print_exp_node(stmt->exp, indent + 1);
}

static void print_function_node(const ASTFunction *func, int indent) {
    print_indent(indent);
    printf("FunctionDeclaration(name: '%s'):\n", func->name);
    print_statement_node(func->statement, indent + 1);
}

void print_ast(const ASTProgram *program) {
    if (!program) {
        printf("AST is NULL\n");
        print_function_node(program->function, 1);
    }
}

typedef struct {
    TokenList *tokens;
    size_t current;
} Parser;

static Token *peek(Parser *parser) {
    if (parser->current < parser->tokens->count) {
        return &parser->tokens->items[parser->current];
    }
    return NULL;
}

static Token *advance(Parser *parser) {
    Token *token = peek(parser);
    if (token) parser->current++;
    return token;
}

static Token *expect(Parser *parser, TokenType type, const char *err_msg){
    Token *token = advance(parser);
    if (!token || token->type != type) {
        fprintf(stderr, "Parse Error: %s\n", err_msg);
        exit(EXIT_FAILURE);
    }
    return token;
}

static ASTExp *parse_exp(Parser *parser) {
    //Token *token = expect(parser, TOKEN_INT_LITERAL, "Expected integer Literal");
    Token *tok = peek(parser);
    
    if (!tok) {
        fprintf(stderr, "Parse Error: Unexpected end of input\n");
        exit(EXIT_FAILURE);
    }

    if (tok->type == TOKEN_MINUS || tok->type == TOKEN_TILDE || tok->type == TOKEN_EXCLAMATION) {
        advance(parser);
    
        ASTExp *exp = malloc(sizeof(ASTExp));
         exp->type= EXP_UNARY;

         if (tok->type == TOKEN_MINUS)  exp->unary.op = UNARY_NEGATE;
         if (tok->type == TOKEN_TILDE)  exp->unary.op = UNARY_COMPLEMENT;
         if (tok->type == TOKEN_EXCLAMATION) exp->unary.op = UNARY_NOT;

          exp->unary.sub_exp = parse_exp(parser);
    return exp;
    

}
if (tok->type == TOKEN_INT_LITERAL) {
    advance(parser);
    ASTExp *exp = malloc(sizeof(ASTExp));
    exp->type = EXP_INT_LITERAL;
    exp->int_val = atoi(tok->lexeme);
    return exp;
}
fprintf(stderr, "Parse Error: Invalid expression starting with '%s'\n", tok->lexeme);
exit(EXIT_FAILURE);
}


static ASTStatement *parse_statement(Parser *parser) {
    expect(parser,TOKEN_RETURN_KEYWORD, "Expected 'return' keyword");
    ASTStatement *stmt = malloc(sizeof(ASTStatement));
    stmt->exp = parse_exp(parser);
    expect(parser, TOKEN_SEMICOLON, "Expected ';' after return statement");
    return stmt;
}
static ASTFunction *parse_function(Parser *parser) {
    expect(parser, TOKEN_INT_KEYWORD, "Expected 'int' return type");
    Token *id =expect(parser, TOKEN_IDENTIFIER, "Expected function name");
    ASTFunction *func = malloc(sizeof(ASTFunction));
    func->name = strdup(id->lexeme);
    expect(parser, TOKEN_OPEN_PAREN, "Expected '(' after function name");
    expect(parser, TOKEN_CLOSE_PAREN, "Expected ')' after '('");
    expect(parser, TOKEN_OPEN_BRACE, "Expected '{' to start function body");
    expect(parser, TOKEN_CLOSE_BRACE,"Expected '}' to close function body");
    func->statement = parse_statement(parser);
    return func;
}

ASTProgram *parse(TokenList *tokens) {
    Parser parser = {.tokens = tokens, .current = 0};//possible error
    ASTProgram *program = malloc(sizeof(ASTProgram));
    program->function = parse_function(&parser);
    return program;
}

void free_exp(ASTExp *exp) {
    if (!exp) return;
    if (exp->type == EXP_UNARY) {
        free_exp(exp->unary.sub_exp);
    }
    free(exp);
    
}

void free_ast(ASTProgram *program) {
    if (!program) return;
    if (program->function) {
        free(program->function->name);
        if (program->function->statement) {
            free_exp(program->function->statement->exp);
            free(program->function->statement);
        }
        free(program->function);
    }
    free(program);
}