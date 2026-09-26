#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"

static void append_token(TokenList *list, TokenType type, const char *lexeme) {
    if (list->count >= list->capacity) {
        list->capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        list->items = realloc(list->items, list->capacity * sizeof(Token));
    }
    list->items[list->count].type = type;
    if (lexeme) {
        size_t len = strlen(lexeme);
        list->items[list->count].lexeme = malloc(len + 1);
        strcpy(list->items[list->count].lexeme, lexeme);
    } else {
        list->items[list->count].lexeme = NULL;
    }
    list->count++;
}

TokenList lex_file(const char *filename) {
    TokenList list = { .items = NULL, .count = 0, .capacity = 0 };
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return list;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *src = malloc(size + 1);
    fread(src, 1, size, file);
    src[size] = '\0';
    fclose(file);

    size_t i = 0;
    while (src[i] != '\0') {
        if (isspace(src[i])) {
            i++;
            continue;
        }

        // 2-character operators
        if (src[i] == '&' && src[i + 1] == '&') { append_token(&list, TOKEN_LOGICAL_AND, "&&"); i += 2; continue; }
        if (src[i] == '|' && src[i + 1] == '|') { append_token(&list, TOKEN_LOGICAL_OR, "||"); i += 2; continue; }
        if (src[i] == '=' && src[i + 1] == '=') { append_token(&list, TOKEN_EQUAL, "=="); i += 2; continue; }
        if (src[i] == '!' && src[i + 1] == '=') { append_token(&list, TOKEN_NOT_EQUAL, "!="); i += 2; continue; }
        if (src[i] == '<' && src[i + 1] == '=') { append_token(&list, TOKEN_LESS_EQUAL, "<="); i += 2; continue; }
        if (src[i] == '>' && src[i + 1] == '=') { append_token(&list, TOKEN_GREATER_EQUAL, ">="); i += 2; continue; }

        // Single-character tokens
        switch (src[i]) {
            case '{': append_token(&list, TOKEN_OPEN_BRACE, "{"); i++; continue;
            case '}': append_token(&list, TOKEN_CLOSE_BRACE, "}"); i++; continue;
            case '(': append_token(&list, TOKEN_OPEN_PAREN, "("); i++; continue;
            case ')': append_token(&list, TOKEN_CLOSE_PAREN, ")"); i++; continue;
            case ';': append_token(&list, TOKEN_SEMICOLON, ";"); i++; continue;
            case '~': append_token(&list, TOKEN_TILDE, "~"); i++; continue;
            case '-': append_token(&list, TOKEN_MINUS, "-"); i++; continue;
            case '!': append_token(&list, TOKEN_EXCLAMATION, "!"); i++; continue;
            case '+': append_token(&list, TOKEN_PLUS, "+"); i++; continue;
            case '*': append_token(&list, TOKEN_ASTERISK, "*"); i++; continue;
            case '/': append_token(&list, TOKEN_SLASH, "/"); i++; continue;
            case '<': append_token(&list, TOKEN_LESS_THAN, "<"); i++; continue;
            case '>': append_token(&list, TOKEN_GREATER_THAN, ">"); i++; continue;
            case '=': append_token(&list, TOKEN_ASSIGN, "="); i++; continue; // Added single '='
        }

        // Identifiers and Keywords
        if (isalpha(src[i]) || src[i] == '_') {
            size_t start = i;
            while (isalnum(src[i]) || src[i] == '_') {
                i++;
            }
            size_t len = i - start;
            char *buf = malloc(len + 1);
            strncpy(buf, &src[start], len);
            buf[len] = '\0';

            if (strcmp(buf, "int") == 0) {
                append_token(&list, TOKEN_INT_KEYWORD, buf);
            } else if (strcmp(buf, "return") == 0) {
                append_token(&list, TOKEN_RETURN_KEYWORD, buf);
            } else {
                append_token(&list, TOKEN_IDENTIFIER, buf);
            }
            free(buf);
            continue;
        }

        // Integer Literals
        if (isdigit(src[i])) {
            size_t start = i;
            while (isdigit(src[i])) {
                i++;
            }
            size_t len = i - start;
            char *buf = malloc(len + 1);
            strncpy(buf, &src[start], len);
            buf[len] = '\0';

            append_token(&list, TOKEN_INT_LITERAL, buf);
            free(buf);
            continue;
        }

        char err_str[2] = { src[i], '\0' };
        append_token(&list, TOKEN_ERROR, err_str);
        i++;
    }

    append_token(&list, TOKEN_EOF, NULL);
    free(src);
    return list;
}

void free_tokens(TokenList *list) {
    if (!list) return;
    for (size_t i = 0; i < list->count; i++) {
        free(list->items[i].lexeme);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}