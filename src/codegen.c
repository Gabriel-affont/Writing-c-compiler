#include <stdio.h>
#include <stdlib.h>
#include "codegen.h"

static int label_counter = 0;

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
        // --- Short-Circuit Evaluation for Logical OR (||) ---
        if (exp->binary.op == BINARY_LOGICAL_OR) {
            int id = label_counter++;
            emit_exp(exp->binary.left, out);
            fprintf(out, "    cmpl    $0, %%eax\n");
            fprintf(out, "    je      _clause2_%d\n", id);
            fprintf(out, "    movl    $1, %%eax\n");
            fprintf(out, "    jmp     _end_%d\n", id);
            fprintf(out, "_clause2_%d:\n", id);
            emit_exp(exp->binary.right, out);
            fprintf(out, "    cmpl    $0, %%eax\n");
            fprintf(out, "    movl    $0, %%eax\n");
            fprintf(out, "    setne   %%al\n");
            fprintf(out, "_end_%d:\n", id);
            return;
        }

        // --- Short-Circuit Evaluation for Logical AND (&&) ---
        if (exp->binary.op == BINARY_LOGICAL_AND) {
            int id = label_counter++;
            emit_exp(exp->binary.left, out);
            fprintf(out, "    cmpl    $0, %%eax\n");
            fprintf(out, "    jne     _clause2_%d\n", id);
            fprintf(out, "    jmp     _end_%d\n", id);
            fprintf(out, "_clause2_%d:\n", id);
            emit_exp(exp->binary.right, out);
            fprintf(out, "    cmpl    $0, %%eax\n");
            fprintf(out, "    movl    $0, %%eax\n");
            fprintf(out, "    setne   %%al\n");
            fprintf(out, "_end_%d:\n", id);
            return;
        }

        // --- Standard Binary Operations (Arithmetic & Relational) ---
        emit_exp(exp->binary.left, out);
        fprintf(out, "    pushl   %%eax\n");
        emit_exp(exp->binary.right, out);
        fprintf(out, "    popl    %%ecx\n");

        switch (exp->binary.op) {
            case BINARY_ADD:
                fprintf(out, "    addl    %%ecx, %%eax\n");
                break;

            case BINARY_SUBTRACT:
                fprintf(out, "    subl    %%eax, %%ecx\n");
                fprintf(out, "    movl    %%ecx, %%eax\n");
                break;

            case BINARY_MULTIPLY:
                fprintf(out, "    imul    %%ecx, %%eax\n");
                break;

            case BINARY_DIVIDE:
                fprintf(out, "    pushl   %%eax\n");        // Save divisor (e2) on stack
                fprintf(out, "    movl    %%ecx, %%eax\n"); // Move dividend (e1) into %eax
                fprintf(out, "    cdq\n");                  // Sign-extend %eax into %edx:%eax
                fprintf(out, "    popl    %%ecx\n");        // Restore divisor into %ecx
                fprintf(out, "    idivl   %%ecx\n");         // Divide %edx:%eax by %ecx
                break;

            // --- Relational Operators ---
            case BINARY_EQUAL:
            case BINARY_NOT_EQUAL:
            case BINARY_LESS_THAN:
            case BINARY_LESS_EQUAL:
            case BINARY_GREATER_THAN:
            case BINARY_GREATER_EQUAL:
                fprintf(out, "    cmpl    %%eax, %%ecx\n"); // Compare e1 (ecx) to e2 (eax)
                fprintf(out, "    movl    $0, %%eax\n");
                if (exp->binary.op == BINARY_EQUAL)         fprintf(out, "    sete    %%al\n");
                if (exp->binary.op == BINARY_NOT_EQUAL)     fprintf(out, "    setne   %%al\n");
                if (exp->binary.op == BINARY_LESS_THAN)     fprintf(out, "    setl    %%al\n");
                if (exp->binary.op == BINARY_LESS_EQUAL)    fprintf(out, "    setle   %%al\n");
                if (exp->binary.op == BINARY_GREATER_THAN)  fprintf(out, "    setg    %%al\n");
                if (exp->binary.op == BINARY_GREATER_EQUAL) fprintf(out, "    setge   %%al\n");
                break;

            default:
                break;
        }
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