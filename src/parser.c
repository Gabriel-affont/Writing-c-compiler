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
    printf_indent(indent);
    
    if (exp->type == EXP_INT_LITERAL) {
        printf("IntLiteral(%d)\n", exp->int_val);
        return;
    }
    
    if (exp->type == EXP_UNARY) {
        const char *op_str = "";
        switch (exp->unary.op) {
            case UNARY_NEGATE:     op_str = "NEGATE (-)"; break;
            case UNARY_COMPLEMENT: op_str = "COMPLEMENT (~)"; break;
            case UNARY_NOT:        op_str = "LOGICAL_NOT (!)"; break;
        }
        printf("UnaryOp(%s)\n", op_str);
        printf_exp_node(exp->unary.sub_exp, indent + 1);
        return;
    }

    if (exp->type == EXP_BINARY) {
        const char *op_str = "";
        switch (exp->binary.op) {
            case BINARY_ADD:           op_str = "ADD (+)"; break;
            case BINARY_SUBTRACT:      op_str = "SUBTRACT (-)"; break;
            case BINARY_MULTIPLY:      op_str = "MULTIPLY (*)"; break;
            case BINARY_DIVIDE:        op_str = "DIVIDE (/)"; break;
            case BINARY_EQUAL:         op_str = "EQUAL (==)"; break;
            case BINARY_NOT_EQUAL:     op_str = "NOT_EQUAL (!=)"; break;
            case BINARY_LESS_THAN:     op_str = "LESS_THAN (<)"; break;
            case BINARY_LESS_EQUAL:    op_str = "LESS_EQUAL (<=)"; break;
            case BINARY_GREATER_THAN:  op_str = "GREATER_THAN (>)"; break;
            case BINARY_GREATER_EQUAL: op_str = "GREATER_EQUAL (>=)"; break;
            case BINARY_LOGICAL_AND:   op_str = "LOGICAL_AND (&&)"; break;
            case BINARY_LOGICAL_OR:    op_str = "LOGICAL_OR (||)"; break;
        }
        printf("BinaryOp(%s)\n", op_str);
        printf_exp_node(exp->binary.left, indent + 1);
        printf_exp_node(exp->binary.right, indent + 1);
        return;
    }
}

static void print_statement_node(const ASTStatement *stmt, int indent) {
    printf_indent(indent);
    printf("ReturnStatement:\n");
    printf_exp_node(stmt->exp, indent + 1);
}

static void print_function_node(const ASTFunction *func, int indent) {
    printf_indent(indent);
    printf("FunctionDeclaration(name: '%s'):\n", func->name);
    print_statement_node(func->statement, indent + 1);
}

void print_ast(const ASTProgram *program) {
    if (!program) {
        printf("AST is NULL\n");
        return;
    }
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

/* Forward Declarations */
static ASTExp *parse_exp(Parser *parser);
static ASTExp *parse_logical_and_exp(Parser *parser);
static ASTExp *parse_equality_exp(Parser *parser);
static ASTExp *parse_relational_exp(Parser *parser);
static ASTExp *parse_additive_exp(Parser *parser);
static ASTExp *parse_term(Parser *parser);
static ASTExp *parse_factor(Parser *parser);

/* <factor> ::= "(" <exp> ")" | <unary_op> <factor> | <int> */
static ASTExp *parse_factor(Parser *parser) {
    Token *tok = peek(parser);
    if (!tok) {
        fprintf(stderr, "Parse Error: Unexpected end of input in factor\n");
        exit(EXIT_FAILURE);
    }

    if (tok->type == TOKEN_OPEN_PAREN) {
        advance(parser);
        ASTExp *exp = parse_exp(parser);
        expect(parser, TOKEN_CLOSE_PAREN, "Expected ')' after expression");
        return exp;
    }

    if (tok->type == TOKEN_MINUS || tok->type == TOKEN_TILDE || tok->type == TOKEN_EXCLAMATION) {
        advance(parser);
        ASTExp *exp = malloc(sizeof(ASTExp));
        exp->type = EXP_UNARY;

        if (tok->type == TOKEN_MINUS)       exp->unary.op = UNARY_NEGATE;
        if (tok->type == TOKEN_TILDE)       exp->unary.op = UNARY_COMPLEMENT;
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

    fprintf(stderr, "Parse Error: Invalid factor starting with '%s'\n", tok->lexeme);
    exit(EXIT_FAILURE);
}

/* <term> ::= <factor> { ("*" | "/") <factor> } */
static ASTExp *parse_term(Parser *parser) {
    ASTExp *left = parse_factor(parser);
    Token *tok = peek(parser);

    while (tok && (tok->type == TOKEN_ASTERISK || tok->type == TOKEN_SLASH)) {
        advance(parser);
        ASTExp *binary_exp = malloc(sizeof(ASTExp));
        binary_exp->type = EXP_BINARY;
        binary_exp->binary.left = left;
        binary_exp->binary.op = (tok->type == TOKEN_ASTERISK) ? BINARY_MULTIPLY : BINARY_DIVIDE;
        binary_exp->binary.right = parse_factor(parser);

        left = binary_exp;
        tok = peek(parser);
    }
    return left;
}

/* <additive-exp> ::= <term> { ("+" | "-") <term> } */
static ASTExp *parse_additive_exp(Parser *parser) {
    ASTExp *left = parse_term(parser);
    Token *tok = peek(parser);

    while (tok && (tok->type == TOKEN_PLUS || tok->type == TOKEN_MINUS)) {
        advance(parser);
        ASTExp *binary_exp = malloc(sizeof(ASTExp));
        binary_exp->type = EXP_BINARY;
        binary_exp->binary.left = left;
        binary_exp->binary.op = (tok->type == TOKEN_PLUS) ? BINARY_ADD : BINARY_SUBTRACT;
        binary_exp->binary.right = parse_term(parser);

        left = binary_exp;
        tok = peek(parser);
    }
    return left;
}

/* <relational-exp> ::= <additive-exp> { ("<" | ">" | "<=" | ">=") <additive-exp> } */
static ASTExp *parse_relational_exp(Parser *parser) {
    ASTExp *left = parse_additive_exp(parser);
    Token *tok = peek(parser);

    while (tok && (tok->type == TOKEN_LESS_THAN || tok->type == TOKEN_LESS_EQUAL ||
                   tok->type == TOKEN_GREATER_THAN || tok->type == TOKEN_GREATER_EQUAL)) {
        advance(parser);
        ASTExp *binary_exp = malloc(sizeof(ASTExp));
        binary_exp->type = EXP_BINARY;
        binary_exp->binary.left = left;

        if (tok->type == TOKEN_LESS_THAN)     binary_exp->binary.op = BINARY_LESS_THAN;
        if (tok->type == TOKEN_LESS_EQUAL)    binary_exp->binary.op = BINARY_LESS_EQUAL;
        if (tok->type == TOKEN_GREATER_THAN)  binary_exp->binary.op = BINARY_GREATER_THAN;
        if (tok->type == TOKEN_GREATER_EQUAL) binary_exp->binary.op = BINARY_GREATER_EQUAL;

        binary_exp->binary.right = parse_additive_exp(parser);
        left = binary_exp;
        tok = peek(parser);
    }
    return left;
}

/* <equality-exp> ::= <relational-exp> { ("==" | "!=") <relational-exp> } */
static ASTExp *parse_equality_exp(Parser *parser) {
    ASTExp *left = parse_relational_exp(parser);
    Token *tok = peek(parser);

    while (tok && (tok->type == TOKEN_EQUAL || tok->type == TOKEN_NOT_EQUAL)) {
        advance(parser);
        ASTExp *binary_exp = malloc(sizeof(ASTExp));
        binary_exp->type = EXP_BINARY;
        binary_exp->binary.left = left;
        binary_exp->binary.op = (tok->type == TOKEN_EQUAL) ? BINARY_EQUAL : BINARY_NOT_EQUAL;
        binary_exp->binary.right = parse_relational_exp(parser);

        left = binary_exp;
        tok = peek(parser);
    }
    return left;
}

/* <logical-and-exp> ::= <equality-exp> { "&&" <equality-exp> } */
static ASTExp *parse_logical_and_exp(Parser *parser) {
    ASTExp *left = parse_equality_exp(parser);
    Token *tok = peek(parser);

    while (tok && tok->type == TOKEN_LOGICAL_AND) {
        advance(parser);
        ASTExp *binary_exp = malloc(sizeof(ASTExp));
        binary_exp->type = EXP_BINARY;
        binary_exp->binary.left = left;
        binary_exp->binary.op = BINARY_LOGICAL_AND;
        binary_exp->binary.right = parse_equality_exp(parser);

        left = binary_exp;
        tok = peek(parser);
    }
    return left;
}

/* <exp> ::= <logical-and-exp> { "||" <logical-and-exp> } */
static ASTExp *parse_exp(Parser *parser) {
    ASTExp *left = parse_logical_and_exp(parser);
    Token *tok = peek(parser);

    while (tok && tok->type == TOKEN_LOGICAL_OR) {
        advance(parser);
        ASTExp *binary_exp = malloc(sizeof(ASTExp));
        binary_exp->type = EXP_BINARY;
        binary_exp->binary.left = left;
        binary_exp->binary.op = BINARY_LOGICAL_OR;
        binary_exp->binary.right = parse_logical_and_exp(parser);

        left = binary_exp;
        tok = peek(parser);
    }
    return left;
}

static ASTStatement *parse_statement(Parser *parser) {
    expect(parser, TOKEN_RETURN_KEYWORD, "Expected 'return' keyword");
    ASTStatement *stmt = malloc(sizeof(ASTStatement));
    stmt->exp = parse_exp(parser);
    expect(parser, TOKEN_SEMICOLON, "Expected ';' after return statement");
    return stmt;
}

static ASTFunction *parse_function(Parser *parser) {
    expect(parser, TOKEN_INT_KEYWORD, "Expected 'int' return type");
    Token *id = expect(parser, TOKEN_IDENTIFIER, "Expected function name");
    ASTFunction *func = malloc(sizeof(ASTFunction));
    func->name = strdup(id->lexeme);

    expect(parser, TOKEN_OPEN_PAREN, "Expected '(' after function name");
    expect(parser, TOKEN_CLOSE_PAREN, "Expected ')' after '('");
    expect(parser, TOKEN_OPEN_BRACE, "Expected '{' to start function body");
    func->statement = parse_statement(parser);
    expect(parser, TOKEN_CLOSE_BRACE, "Expected '}' to close function body");

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
    if (exp->type == EXP_UNARY) {
        free_exp(exp->unary.sub_exp);
    } else if (exp->type == EXP_BINARY) {
        free_exp(exp->binary.left);
        free_exp(exp->binary.right);
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