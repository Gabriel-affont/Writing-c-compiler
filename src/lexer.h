#ifndef LEXER_H
#define LEXER_H
#include <stddef.h>

typedef enum {
    TOKEN_OPEN_BRACE,
    TOKEN_CLOSE_BRACE,
    TOKEN_OPEN_PAREN,
    TOKEN_CLOSE_PAREN,
    TOKEN_SEMICOLON,
    TOKEN_INT_KEYWORD,
    TOKEN_RETURN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_TILDE,
    TOKEN_MINUS,
    TOKEN_EXCLAMATION,
    TOKEN_EOF,
    TOKEN_ERROR

} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t capacity;
} TokenList;

TokenList lex_file(const char *filename);
void free_tokens(TokenList *list);

#endif