#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

typedef struct {
    TokenList *tokens;
    size_t current;
} Parser;

static Token *peek(Parser *parser) {
    if (parser->currrent < parser->tokens->count) {
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
    Token *token = expect(parser, TOKEN_INT_LITERAL, "Expected integer Literal");
    ASTExp *exp = malloc(sizeof(ASTExp));
    exp->value = atoi(tok->lexeme);
    return exp;
}

static ASTStatement *parse_statement(Parser *parser) {
    expect(parser,TOKEN_RETURN_KEYWORD, "Expected 'return' keyword");
    ASTExp *stmt = malloc(sizeof(ASTStatement));
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
    func->statement = parse_statement(parser);
    return func;
}

ASTProgram *parse(TokenList *tokens) {
    Parser parser = {.tokens = tokens, .current = 0};//possible error
    ASTProgram *program = malloc(sizeof(ASTProgram));
    program->function = parse_function(&parser);
    return program;
}

void free_ast(ASTProgram *program) {
    if (!program) return;
    if (program->function) {
        free(program->function->name);
        if (program->function->statement) {
            free(program->function->statement->exp);
            free(program->function->statement);
        }
        free(program->function);
    }
    free(program);
}