#include <stdio.h>
#include "codegen.h"

static void emit_exp(const ASTExp *exp, FILE *out) {
    fprintf(out, "  movl $%d, %%eax\n", exp->value);
}

static void emit_statement(const ASTStatement *stmt, FILE *out) {
    emit_exp(stmt->exp, out);
    fprintf(out, "  ret\n");
}

static void emit_function(const ASTFunction *func, FILE *out) {
    fprintf(out, "  .globl %s\n", func->name);
    fprintf(out, "%s:\n", func->name);
    emit_statement(func->statement, out);
}
void generate_code(const ASTProgram *program, FILE *out) {
    if (program && program->function) {
        emit_function(program->function, out);

    }
}