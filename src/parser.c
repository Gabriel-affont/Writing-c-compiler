#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

static void printf_indent(int level) {
    for (int i = 0; i < level; i++) printf("  ");
}

static void printf_exp_node(const ASTExp *exp, int indent) {
    if (!exp) return;
    printf_indent(indent);
    
    if (exp->type == EXP_INT_LITERAL) {
        printf("IntLiteral(%d)\n", exp->int_val);
        return;
    }
    if (exp->type == EXP_VARIABLE) {
        printf("Var(%s)\n", exp->var_name);
        return;
    }
    if (exp->type == EXP_ASSIGN) {
        printf("Assign('%s'):\n", exp->assign.name);
        printf_exp_node(exp->assign.exp, indent + 1);
        return;
    }
    if (exp->type == EXP_UNARY) {
        printf("UnaryOp\n");
        printf_exp_node(exp->unary.sub_exp, indent + 1);
        return;
    }
    if (exp->type == EXP_BINARY) {
        printf("BinaryOp\n");
        printf_exp_node(exp->binary.left, indent + 1);
        printf_exp_node(exp->binary.right, indent + 1);
        return;
    }
}

static void print_statement_node(const ASTStatement *stmt, int indent) {
    printf_indent(indent);
    if (stmt->type == STATEMENT_RETURN) {
        printf("Return:\n");
        printf_exp_node(stmt->exp, indent + 1);
    } else if (stmt->type == STATEMENT_DECLARE) {
        printf("Declare('%s'):\n", stmt->declare.name);
        if (stmt->declare.init_exp) printf_exp_node(stmt->declare.init_exp, indent + 1);
    } else if (stmt->type == STATEMENT_EXP) {
        printf("ExpStatement:\n");
        printf_exp_node(stmt->exp, indent + 1);
    }
}

static void print_function_node(const ASTFunction *func, int indent) {
    printf_indent(indent);
    printf("FunctionDeclaration(name: '%s'):\n", func->name);
    for (size_t i = 0; i < func->statement_count; i++) {
        print_statement_node(func->statements[i], indent + 1);
    }
}

void print_ast(const ASTProgram *program) {
    if (!program) return;
    print_function_node(program->function, 0);
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

static Token *expect(Parser *parser, TokenType type, const char *err_msg) {
    Token *token = advance(parser);
    if (!token || token->type != type) {
        fprintf(stderr, "Parse Error: %s\n", err_msg);
        exit(EXIT_FAILURE);
    }
    return token;
}

/* Declarations */
static ASTExp *parse_exp(Parser *parser);
static ASTExp *parse_logical_or_exp(Parser *parser);
static ASTExp *parse_logical_and_exp(Parser *parser);
static ASTExp *parse_equality_exp(Parser *parser);
static ASTExp *parse_relational_exp(Parser *parser);
static ASTExp *parse_additive_exp(Parser *parser);
static ASTExp *parse_term(Parser *parser);
static ASTExp *parse_factor(Parser *parser);

static ASTExp *parse_factor(Parser *parser) {
    Token *tok = peek(parser);
    if (!tok) exit(EXIT_FAILURE);

    if (tok->type == TOKEN_OPEN_PAREN) {
        advance(parser);
        ASTExp *exp = parse_exp(parser);
        expect(parser, TOKEN_CLOSE_PAREN, "Expected ')'");
        return exp;
    }

    if (tok->type == TOKEN_MINUS || tok->type == TOKEN_TILDE || tok->type == TOKEN_EXCLAMATION) {
        advance(parser);
        ASTExp *exp = malloc(sizeof(ASTExp));
        exp->type = EXP_UNARY;
        if (tok->type == TOKEN_MINUS) exp->unary.op = UNARY_NEGATE;
        if (tok->type == TOKEN_TILDE) exp->unary.op = UNARY_COMPLEMENT;
        if (tok->type == TOKEN_EXCLAMATION) exp->unary.op = UNARY_NOT;
        exp->unary.sub_exp = parse_factor(parser);
        return exp;
    }

    if (tok->type == TOKEN_INT_LITERAL) {
        advance(parser);
        ASTExp *exp = malloc(sizeof(ASTExp));
        exp->type = EXP_INT_LITERAL;
        exp->int_val = atoi(tok->lexeme);
        return exp;
    }

    if (tok->type == TOKEN_IDENTIFIER) {
        advance(parser);
        ASTExp *exp = malloc(sizeof(ASTExp));
        exp->type = EXP_VARIABLE;
        exp->var_name = strdup(tok->lexeme);
        return exp;
    }

    fprintf(stderr, "Parse Error: Invalid factor\n");
    exit(EXIT_FAILURE);
}

static ASTExp *parse_term(Parser *parser) {
    ASTExp *left = parse_factor(parser);
    Token *tok = peek(parser);
    while (tok && (tok->type == TOKEN_ASTERISK || tok->type == TOKEN_SLASH)) {
        advance(parser);
        ASTExp *b = malloc(sizeof(ASTExp));
        b->type = EXP_BINARY;
        b->binary.left = left;
        b->binary.op = (tok->type == TOKEN_ASTERISK) ? BINARY_MULTIPLY : BINARY_DIVIDE;
        b->binary.right = parse_factor(parser);
        left = b;
        tok = peek(parser);
    }
    return left;
}

static ASTExp *parse_additive_exp(Parser *parser) {
    ASTExp *left = parse_term(parser);
    Token *tok = peek(parser);
    while (tok && (tok->type == TOKEN_PLUS || tok->type == TOKEN_MINUS)) {
        advance(parser);
        ASTExp *b = malloc(sizeof(ASTExp));
        b->type = EXP_BINARY;
        b->binary.left = left;
        b->binary.op = (tok->type == TOKEN_PLUS) ? BINARY_ADD : BINARY_SUBTRACT;
        b->binary.right = parse_term(parser);
        left = b;
        tok = peek(parser);
    }
    return left;
}

static ASTExp *parse_relational_exp(Parser *parser) {
    ASTExp *left = parse_additive_exp(parser);
    Token *tok = peek(parser);
    while (tok && (tok->type == TOKEN_LESS_THAN || tok->type == TOKEN_LESS_EQUAL ||
                   tok->type == TOKEN_GREATER_THAN || tok->type == TOKEN_GREATER_EQUAL)) {
        advance(parser);
        ASTExp *b = malloc(sizeof(ASTExp));
        b->type = EXP_BINARY;
        b->binary.left = left;
        if (tok->type == TOKEN_LESS_THAN) b->binary.op = BINARY_LESS_THAN;
        if (tok->type == TOKEN_LESS_EQUAL) b->binary.op = BINARY_LESS_EQUAL;
        if (tok->type == TOKEN_GREATER_THAN) b->binary.op = BINARY_GREATER_THAN;
        if (tok->type == TOKEN_GREATER_EQUAL) b->binary.op = BINARY_GREATER_EQUAL;
        b->binary.right = parse_additive_exp(parser);
        left = b;
        tok = peek(parser);
    }
    return left;
}

static ASTExp *parse_equality_exp(Parser *parser) {
    ASTExp *left = parse_relational_exp(parser);
    Token *tok = peek(parser);
    while (tok && (tok->type == TOKEN_EQUAL || tok->type == TOKEN_NOT_EQUAL)) {
        advance(parser);
        ASTExp *b = malloc(sizeof(ASTExp));
        b->type = EXP_BINARY;
        b->binary.left = left;
        b->binary.op = (tok->type == TOKEN_EQUAL) ? BINARY_EQUAL : BINARY_NOT_EQUAL;
        b->binary.right = parse_relational_exp(parser);
        left = b;
        tok = peek(parser);
    }
    return left;
}

static ASTExp *parse_logical_and_exp(Parser *parser) {
    ASTExp *left = parse_equality_exp(parser);
    Token *tok = peek(parser);
    while (tok && tok->type == TOKEN_LOGICAL_AND) {
        advance(parser);
        ASTExp *b = malloc(sizeof(ASTExp));
        b->type = EXP_BINARY;
        b->binary.left = left;
        b->binary.op = BINARY_LOGICAL_AND;
        b->binary.right = parse_equality_exp(parser);
        left = b;
        tok = peek(parser);
    }
    return left;
}

static ASTExp *parse_logical_or_exp(Parser *parser) {
    ASTExp *left = parse_logical_and_exp(parser);
    Token *tok = peek(parser);
    while (tok && tok->type == TOKEN_LOGICAL_OR) {
        advance(parser);
        ASTExp *b = malloc(sizeof(ASTExp));
        b->type = EXP_BINARY;
        b->binary.left = left;
        b->binary.op = BINARY_LOGICAL_OR;
        b->binary.right = parse_logical_and_exp(parser);
        left = b;
        tok = peek(parser);
    }
    return left;
}

/* <exp> ::= <id> "=" <exp> | <logical-or-exp> */
static ASTExp *parse_exp(Parser *parser) {
    Token *tok = peek(parser);
    if (tok && tok->type == TOKEN_IDENTIFIER) {
        // Look ahead to see if next token is '='
        if (parser->current + 1 < parser->tokens->count &&
            parser->tokens->items[parser->current + 1].type == TOKEN_ASSIGN) {
            Token *id = advance(parser); // Consume identifier
            advance(parser);             // Consume '='
            ASTExp *assign_node = malloc(sizeof(ASTExp));
            assign_node->type = EXP_ASSIGN;
            assign_node->assign.name = strdup(id->lexeme);
            assign_node->assign.exp = parse_exp(parser);
            return assign_node;
        }
    }
    return parse_logical_or_exp(parser);
}

static ASTStatement *parse_statement(Parser *parser) {
    Token *tok = peek(parser);
    ASTStatement *stmt = malloc(sizeof(ASTStatement));

    if (tok && tok->type == TOKEN_INT_KEYWORD) {
        advance(parser); // Consume 'int'
        Token *id = expect(parser, TOKEN_IDENTIFIER, "Expected variable name");
        stmt->type = STATEMENT_DECLARE;
        stmt->declare.name = strdup(id->lexeme);
        stmt->declare.init_exp = NULL;

        tok = peek(parser);
        if (tok && tok->type == TOKEN_ASSIGN) {
            advance(parser); // Consume '='
            stmt->declare.init_exp = parse_exp(parser);
        }
        expect(parser, TOKEN_SEMICOLON, "Expected ';' after declaration");
        return stmt;
    }

    if (tok && tok->type == TOKEN_RETURN_KEYWORD) {
        advance(parser); // Consume 'return'
        stmt->type = STATEMENT_RETURN;
        stmt->exp = parse_exp(parser);
        expect(parser, TOKEN_SEMICOLON, "Expected ';' after return");
        return stmt;
    }

    // Otherwise, expression statement
    stmt->type = STATEMENT_EXP;
    stmt->exp = parse_exp(parser);
    expect(parser, TOKEN_SEMICOLON, "Expected ';' after expression");
    return stmt;
}

static ASTFunction *parse_function(Parser *parser) {
    expect(parser, TOKEN_INT_KEYWORD, "Expected 'int'");
    Token *id = expect(parser, TOKEN_IDENTIFIER, "Expected function name");
    ASTFunction *func = malloc(sizeof(ASTFunction));
    func->name = strdup(id->lexeme);
    func->statements = NULL;
    func->statement_count = 0;

    expect(parser, TOKEN_OPEN_PAREN, "Expected '('");
    expect(parser, TOKEN_CLOSE_PAREN, "Expected ')'");
    expect(parser, TOKEN_OPEN_BRACE, "Expected '{'");

    while (peek(parser) && peek(parser)->type != TOKEN_CLOSE_BRACE) {
        func->statements = realloc(func->statements, sizeof(ASTStatement*) * (func->statement_count + 1));
        func->statements[func->statement_count++] = parse_statement(parser);
    }

    expect(parser, TOKEN_CLOSE_BRACE, "Expected '}'");
    return func;
}

ASTProgram *parse(TokenList *tokens) {
    Parser parser = {.tokens = tokens, .current = 0};
    ASTProgram *program = malloc(sizeof(ASTProgram));
    program->function = parse_function(&parser);
    return program;
}

void free_exp(ASTExp *exp) {
    if (!exp) return;
    if (exp->type == EXP_VARIABLE) free(exp->var_name);
    else if (exp->type == EXP_ASSIGN) { free(exp->assign.name); free_exp(exp->assign.exp); }
    else if (exp->type == EXP_UNARY) free_exp(exp->unary.sub_exp);
    else if (exp->type == EXP_BINARY) { free_exp(exp->binary.left); free_exp(exp->binary.right); }
    free(exp);
}

void free_ast(ASTProgram *program) {
    if (!program) return;
    if (program->function) {
        free(program->function->name);
        for (size_t i = 0; i < program->function->statement_count; i++) {
            ASTStatement *stmt = program->function->statements[i];
            if (stmt->type == STATEMENT_DECLARE) {
                free(stmt->declare.name);
                free_exp(stmt->declare.init_exp);
            } else {
                free_exp(stmt->exp);
            }
            free(stmt);
        }
        free(program->function->statements);
        free(program->function);
    }
    free(program);
}