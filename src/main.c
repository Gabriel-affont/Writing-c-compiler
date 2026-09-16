#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"


int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr,"Usage: %s <source_file.c>\n", argv[0]);
        return 1;
    }
    TokenList tokens = lex_file(argv[1]);
    ASTProgram *ast = parse(&tokens);
    printf("Parsed function '%s' returning %d\n",
    ast->function->name,
    ast->function->statement->exp->value);
    free_ast(ast);
    free_tokens(&tokens);
    return EXIT_SUCCESS;
}
