#include <stdio.h>
#include "lexer.h"


int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr,"Usage: %s <source_file.c>\n", argv[0]);
        return 1;
    }
    TokenList tokens = lex_file(argv[1]);
    for (size_t i = 0; i< tokens.count; i++) {
        printf("Token %zu: Type %d, Lexeme: '%s'\n",
        i, tokens.items[i].type,tokens.items[i].lexeme);
    }
    free_tokens(&tokens);
    return 0;
}
