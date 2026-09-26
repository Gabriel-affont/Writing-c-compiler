#include <stdio.h>
#include "codegen.h"

static void emit_exp(const ASTExp *exp, FILE *out) {
    if (exp->type == EXP_INT_LITERAL) {
        fprintf(out, "    movl    $%d, %%eax\n", exp->int_val);
        return;
    }

    if (exp->type == EXP_UNARY) {
        emit_exp(exp->unary.sub_exp, out);

        switch (exp->unary.op) {
            case UNARY_NEGATE:
                fprintf(out, "    negl    %%eax\n");
                break;
            case UNARY_COMPLEMENT:
                fprintf(out, "    notl    %%eax\n");
                break;
            case UNARY_NOT:
                fprintf(out, "    cmpl    $0, %%eax\n");
                fprintf(out, "    movl    $0, %%eax\n");
                fprintf(out, "    sete    %%al\n");
                break;
        }
        return;
    }

    if (exp->type == EXP_BINARY) {
        // 1. Evaluate left child (e1) -> result lands in %eax
        emit_exp(exp->binary.left, out);

        // 2. Save e1 on the stack
        fprintf(out, "    pushl   %%eax\n");

        // 3. Evaluate right child (e2) -> result lands in %eax
        emit_exp(exp->binary.right, out);

        // 4. Pop saved e1 into %ecx
        fprintf(out, "    popl    %%ecx\n");

        // 5. Perform the binary operation
        switch (exp->binary.op) {
            case BINARY_ADD:
                // %eax = %eax (e2) + %ecx (e1)
                fprintf(out, "    addl    %%ecx, %%eax\n");
                break;

            case BINARY_SUBTRACT:
                // %ecx holds e1, %eax holds e2 -> %ecx = e1 - e2
                fprintf(out, "    subl    %%eax, %%ecx\n");
                fprintf(out, "    movl    %%ecx, %%eax\n");
                break;

            case BINARY_MULTIPLY:
                // %eax = %eax (e2) * %ecx (e1)
                fprintf(out, "    imul    %%ecx, %%eax\n");
                break;

            case BINARY_DIVIDE:
                // %eax holds e2 (divisor), %ecx holds e1 (dividend)
                    fprintf(out, "    pushl   %%eax\n");        // Save divisor (e2) on stack
                    fprintf(out, "    movl    %%ecx, %%eax\n"); // Move dividend (e1) into %eax
                    fprintf(out, "    cdq\n");                  // Sign-extend %eax into %edx:%eax
                    fprintf(out, "    popl    %%ecx\n");        // Restore divisor into %ecx
                    fprintf(out, "    idivl   %%ecx\n");         // divide %edx:%eax by %ecx
                break;
        }
        return;
    }
}

static void emit_statement(const ASTStatement *stmt, FILE *out) {
    emit_exp(stmt->exp, out);
    fprintf(out, "    ret\n");
}

static void emit_function(const ASTFunction *func, FILE *out) {
    fprintf(out, "    .globl %s\n", func->name);
    fprintf(out, "%s:\n", func->name);
    emit_statement(func->statement, out);
}

void generate_code(const ASTProgram *program, FILE *out) {
    if (program && program->function) {
        emit_function(program->function, out);
    }
}