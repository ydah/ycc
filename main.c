#include "ycc.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Invalid number of arguments\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize();
    Program* prog = program();
    add_type(prog);

    for (Function* fn = prog->funcs; fn; fn = fn->next) {
        int offset = 0;
        for (VarList* vl = fn->locals; vl; vl = vl->next) {
            offset += size_of(vl->var->ty);
            vl->var->offset = offset;
        }
        fn->stack_size = offset;
    }

    codegen(prog);
    return 0;
}
