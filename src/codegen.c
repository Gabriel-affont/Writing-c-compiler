#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

static int label_counter = 0;

typedef struct {
    char name[64];
    int offset;
} VarEntry;

typedef struct {
    VarEntry entries[128];
    int count;
    int current_offset;
} SymbolTable;

static SymbolTable sym_table;

static int find_var_offset(const char *name) {
    for (int i = 0; i < sym_table.count; i++) {
        if (strcmp(sym_table.entries[i].name, name) == 0) {
            return sym_table.entries[i].offset;
        }
    }
    fprintf(stderr, "Codegen Error: Variable '%s' referenced before declaration!\n", name);
    exit(EXIT_FAILURE);
}

static int add_var(const char *name) {
    for (int i = 0; i < sym_table.count; i++) {
        if (strcmp(sym_table.entries[i].name, name) == 0) {
            fprintf(stderr, "Codegen Error: Duplicate variable declaration '%s'\n", name);
            exit(EXIT_FAILURE);
        }
    }
    sym_table.current_offset -= 4; // Move 4 bytes down on stack frame
    strcpy(sym_table.entries[sym_table.count].name, name);
    sym_table.entries[sym_table.count].offset = sym_table.current_offset;
    sym_table.count++;
    return sym_table.current_offset;
}

static void emit_exp(const ASTExp *exp, FILE *out) {
    if (exp->type == EXP_INT_LITERAL) {
        fprintf(out, "    movl    $%d, %%eax\n", exp->int_val);
        return;
    }

    if (exp->type == EXP_VARIABLE) {
        int offset = find_var_offset(exp->var_name);
        fprintf(out, "    movl    %d(%%ebp), %%eax\n", offset);
        return;
    }

    if (exp->type == EXP_ASSIGN) {
        emit_exp(exp->assign.exp, out);
        int offset = find_var_offset(exp->assign.name);
        fprintf(out, "    movl    %%eax, %d(%%ebp)\n", offset);
        return;
    }

    if (exp->type == EXP_UNARY) {
        emit_exp(exp->unary.sub_exp, out);
        switch (exp->unary.op) {
            case UNARY_NEGATE:     fprintf(out, "    negl    %%eax\n"); break;
            case UNARY_COMPLEMENT: fprintf(out, "    notl    %%eax\n"); break;
            case UNARY_NOT:
                fprintf(out, "    cmpl    $0, %%eax\n");
                fprintf(out, "    movl    $0, %%eax\n");
                fprintf(out, "    sete    %%al\n");
                break;
        }
        return;
    }

    if (exp->type == EXP_BINARY) {
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

        emit_exp(exp->binary.left, out);
        fprintf(out, "    pushl   %%eax\n");
        emit_exp(exp->binary.right, out);
        fprintf(out, "    popl    %%ecx\n");

        switch (exp->binary.op) {
            case BINARY_ADD:      fprintf(out, "    addl    %%ecx, %%eax\n"); break;
            case BINARY_SUBTRACT: fprintf(out, "    subl    %%eax, %%ecx\n"); fprintf(out, "    movl    %%ecx, %%eax\n"); break;
            case BINARY_MULTIPLY: fprintf(out, "    imul    %%ecx, %%eax\n"); break;
            case BINARY_DIVIDE:
                fprintf(out, "    pushl   %%eax\n");
                fprintf(out, "    movl    %%ecx, %%eax\n");
                fprintf(out, "    cdq\n");
                fprintf(out, "    popl    %%ecx\n");
                fprintf(out, "    idivl   %%ecx\n");
                break;
            case BINARY_EQUAL:
            case BINARY_NOT_EQUAL:
            case BINARY_LESS_THAN:
            case BINARY_LESS_EQUAL:
            case BINARY_GREATER_THAN:
            case BINARY_GREATER_EQUAL:
                fprintf(out, "    cmpl    %%eax, %%ecx\n");
                fprintf(out, "    movl    $0, %%eax\n");
                if (exp->binary.op == BINARY_EQUAL)         fprintf(out, "    sete    %%al\n");
                if (exp->binary.op == BINARY_NOT_EQUAL)     fprintf(out, "    setne   %%al\n");
                if (exp->binary.op == BINARY_LESS_THAN)     fprintf(out, "    setl    %%al\n");
                if (exp->binary.op == BINARY_LESS_EQUAL)    fprintf(out, "    setle   %%al\n");
                if (exp->binary.op == BINARY_GREATER_THAN)  fprintf(out, "    setg    %%al\n");
                if (exp->binary.op == BINARY_GREATER_EQUAL) fprintf(out, "    setge   %%al\n");
                break;
            default: break;
        }
    }
}

static void emit_statement(const ASTStatement *stmt, FILE *out) {
    if (stmt->type == STATEMENT_DECLARE) {
        int offset = add_var(stmt->declare.name);
        if (stmt->declare.init_exp) {
            emit_exp(stmt->declare.init_exp, out);
            fprintf(out, "    movl    %%eax, %d(%%ebp)\n", offset);
        }
    } else if (stmt->type == STATEMENT_EXP) {
        emit_exp(stmt->exp, out);
    } else if (stmt->type == STATEMENT_RETURN) {
        emit_exp(stmt->exp, out);
        // Function Epilogue
        fprintf(out, "    movl    %%ebp, %%esp\n");
        fprintf(out, "    popl    %%ebp\n");
        fprintf(out, "    ret\n");
    }
}

static void emit_function(const ASTFunction *func, FILE *out) {
    sym_table.count = 0;
    sym_table.current_offset = 0;

    fprintf(out, "    .globl %s\n", func->name);
    fprintf(out, "%s:\n", func->name);

    // Function Prologue
    fprintf(out, "    pushl   %%ebp\n");
    fprintf(out, "    movl    %%esp, %%ebp\n");

    for (size_t i = 0; i < func->statement_count; i++) {
        emit_statement(func->statements[i], out);
    }
}

void generate_code(const ASTProgram *program, FILE *out) {
    if (program && program->function) {
        emit_function(program->function, out);
    }
}