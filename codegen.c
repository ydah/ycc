#include "ycc.h"

char* argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

int label_count = 0;
char* funcname;

void gen_addr(Node* node) {
    if (node->kind != NODE_VAR)
        error("Left side of assignment is not a variable");

    printf("  lea rax, [rbp-%d]\n", node->var->offset);
    printf("  push rax\n");
}

void load() {
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
}

void store() {
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
}

void gen(Node* node) {
    switch (node->kind) {
        case NODE_ASSIGN: {
            gen_addr(node->lhs);
            gen(node->rhs);
            store();
            return;
        }
        case NODE_BLOCK: {
            for (Node* n = node->body; n; n = n->next) gen(n);
            return;
        }
        case NODE_EXPR_STMT: {
            gen(node->lhs);
            printf("  add rsp, 8\n");
            return;
        }
        case NODE_FOR: {
            int c = label_count++;
            int e = label_count++;
            if (node->init) gen(node->init);
            printf(".Lbegin%d:\n", c);
            if (node->cond) {
                gen(node->cond);
                printf("  pop rax\n");
                printf("  cmp rax, 0\n");
                printf("  je .Lend%d\n", e);
            }
            gen(node->then);
            if (node->inc) gen(node->inc);
            printf("  jmp .Lbegin%d\n", c);
            printf(".Lend%d:\n", e);
            return;
        }
        case NODE_FUNCALL: {
            Node* args[6];
            int count = 0;
            for (Node* arg = node->args; arg && count < 6; arg = arg->next) {
                args[count++] = arg;
            }

            for (int i = count - 1; i >= 0; i--) {
                gen(args[i]);
            }

            static char* argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
            for (int i = 0; i < count && i < 6; i++) {
                printf("  pop %s\n", argreg[i]);
            }
            printf("  mov rax, 0\n");
            printf("  call %s\n", node->funcname);
            printf("  push rax\n");
            return;
        }
        case NODE_IF: {
            int c = label_count++;
            int e = label_count++;
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            if (node->els) {
                printf("  je .Lelse%d\n", e);
            } else {
                printf("  je .Lend%d\n", c);
            }
            gen(node->then);
            printf("  jmp .Lend%d\n", c);
            if (node->els) {
                printf(".Lelse%d:\n", e);
                gen(node->els);
            }
            printf(".Lend%d:\n", c);
            return;
        }
        case NODE_VAR: {
            gen_addr(node);
            load();
            return;
        }
        case NODE_NUM: {
            printf("  push %d\n", node->val);
            return;
        }
        case NODE_RETURN: {
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  jmp .Lreturn%s\n", funcname);
            return;
        }
        case NODE_WHILE: {
            int c = label_count++;
            int e = label_count++;
            printf(".Lbegin%d:\n", c);
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lend%d\n", e);
            gen(node->then);
            printf("  jmp .Lbegin%d\n", c);
            printf(".Lend%d:\n", e);
            return;
        }
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
        case NODE_ADD:
            printf("  add rax, rdi\n");
            break;
        case NODE_SUB:
            printf("  sub rax, rdi\n");
            break;
        case NODE_MUL:
            printf("  imul rax, rdi\n");
            break;
        case NODE_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
        case NODE_EQ:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case NODE_NE:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case NODE_LT:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case NODE_LE:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
        default:
            error("Invalid node");
            break;
    }

    printf("  push rax\n");
}

void codegen(Function* prog) {
    printf(".intel_syntax noprefix\n");

    for (Function* fn = prog; fn; fn = fn->next) {
        funcname = fn->name;
        printf(".global %s\n", funcname);
        printf("%s:\n", funcname);

        // Prologue
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", fn->stack_size);

        // Push arguments to stack
        int arg_offset = 0;
        for (VarList* vl = fn->params; vl; vl = vl->next) {
            Var* var = vl->var;
            printf("  mov [rbp-%d], %s\n", var->offset, argreg[arg_offset++]);
        }

        for (Node* node = fn->node; node; node = node->next) gen(node);

        // Epilogue
        printf(".Lreturn%s:\n", funcname);
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    }
}
