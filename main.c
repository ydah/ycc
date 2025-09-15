#include "ycc.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Invalid number of arguments\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize();
    Function* prog = program();

    int offset = 0;
    for (Var* var = prog->locals; var; var = var->next) {
        offset += 8;
        var->offset = offset;
    }
    prog->stack_size = offset;

    codegen(prog);
    return 0;
}
