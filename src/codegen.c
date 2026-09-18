#include <stdio.h>
#include "codegen.h"

static void emit_exp(const ASTExp *exp, FILE *out) {
    if (exp->type == EXP_INT_LITERAL) {
        fprintf(out, "  movl $%d, %%eax\n", exp->int_val);
        return;
    }
    if (exp->type == EXP_UNARY) {
        emit_exp(exp->unary.sub_exp, out);

        switch (exp->unary.op) {
            case UNARY_NEGATE:
            fprintf(out,"  negl  %%eax\n");
            break;
            case UNARY_COMPLEMENT:
            fprintf(out, " notl  %%eax\n");
            break;
            case UNARY_NOT:
            fprintf(out, "  cmpl  $0, %%eax\n");
            fprintf(out, "  movl  $0, %%eax\n");
            fprintf(out, "  sete  %%al\n");
            break;
        }

    }
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