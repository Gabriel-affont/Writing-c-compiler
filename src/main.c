#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "codegen.h"


int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr,"Usage: %s <source_file.c>\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *input_file = argv[1];
    const char *output_file = (argc >= 3) ? argv[2] : "output.s";
    //lexing the input file
    TokenList tokens = lex_file(input_file);
    //parsing the tokens into an AST
    ASTProgram *ast = parse(&tokens);
    //generating assembly code from the AST
    print_ast(ast);
    FILE *out = fopen(output_file, "w");
    if (!out) {
        perror("Failed to open output file");
        free_ast(ast);
        free_tokens(&tokens);
        return EXIT_FAILURE;
    }
    generate_code(ast, out);
    fclose(out);
    printf("Assembly code written to %\n", output_file);
    free_ast(ast);
    free_tokens(&tokens);
    return EXIT_SUCCESS;
}
    
