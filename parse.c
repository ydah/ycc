#include "ycc.h"

VarList* locals = NULL;   // Local variable list
VarList* globals = NULL;  // Global variable list

Var* find_var(Token* tok) {
    for (VarList* vl = locals; vl; vl = vl->next) {
        Var* var = vl->var;
        if (strlen(var->name) == tok->len &&
            !memcmp(tok->str, var->name, tok->len)) {
            return var;
        }
    }

    for (VarList* vl = globals; vl; vl = vl->next) {
        Var* var = vl->var;
        if (strlen(var->name) == tok->len &&
            !memcmp(tok->str, var->name, tok->len)) {
            return var;
        }
    }
    return NULL;
}

Node* new_node(NodeKind kind) {
    Node* node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node* new_binary(NodeKind kind, Node* lhs, Node* rhs) {
    Node* node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node* new_unary(NodeKind kind, Node* expr) {
    Node* node = new_node(kind);
    node->lhs = expr;
    return node;
}

Node* new_num(int val) {
    Node* node = new_node(NODE_NUM);
    node->val = val;
    return node;
}

Node* new_var(Var* var) {
    Node* node = new_node(NODE_VAR);
    node->var = var;
    return node;
}

Var* push_var(char* name, Type* ty, bool is_local) {
    Var* var = calloc(1, sizeof(Var));
    var->name = name;
    var->ty = ty;
    var->is_local = is_local;
    VarList* vl = calloc(1, sizeof(VarList));
    vl->var = var;
    if (is_local) {
        vl->next = locals;
        locals = vl;
    } else {
        vl->next = globals;
        globals = vl;
    }
    return var;
}

Function* function();
Type* basetype();
void global_var();
Node* stmt();
Node* expr();
Node* assign();
Node* equality();
Node* relational();
Node* add();
Node* mul();
Node* postfix();
Node* unary();
Node* primary();

bool is_function() {
    Token *tok = token;
    basetype();
    bool is_func = consume_ident() && consume("(");
    token = tok;
    return is_func;
}

/*
 * program = (function | global_var)*
 */
Program* program() {
    Function head;
    head.next = NULL;
    Function* cur = &head;
    globals = NULL;

    while (!at_eof()) {
        if (is_function()) {
            cur->next = function();
            cur = cur->next;
        } else {
            global_var();
        }
    }

    Program* prog = calloc(1, sizeof(Program));
    prog->globals = globals;
    prog->funcs = head.next;
    return prog;
}

Type* basetype() {
    expect("int");
    Type* ty = int_type();
    while (consume("*")) ty = pointer_to(ty);
    return ty;
}

Type* read_type_suffix(Type* base) {
    if (!consume("[")) return base;
    int sz = expect_number();
    expect("]");
    base = read_type_suffix(base);
    return array_of(base, sz);
}

VarList* read_func_param() {
    Type* ty = basetype();
    char* name = expect_ident();
    ty = read_type_suffix(ty);

    VarList* vl = calloc(1, sizeof(VarList));
    vl->var = push_var(name, ty, true);
    return vl;
}

VarList* read_func_params() {
    if (consume(")")) return NULL;

    VarList* head = read_func_param();
    VarList* cur = head;

    while (!consume(")")) {
        expect(",");
        cur->next = read_func_param();
        cur = cur->next;
    }

    return head;
}

/*
 * function = basetype ident "(" params? ")" "{" stmt* "}"
 * params = param ("," param)*
 * param  = basetype ident
 */
Function* function() {
    locals = NULL;

    Function* fn = calloc(1, sizeof(Function));
    basetype();
    fn->name = expect_ident();
    expect("(");
    fn->params = read_func_params();
    expect("{");

    Node head;
    head.next = NULL;
    Node* cur = &head;

    while (!consume("}")) {
        cur->next = stmt();
        cur = cur->next;
    }

    fn->node = head.next;
    fn->locals = locals;
    return fn;
}

/*
 * global_var = basetype ident ("[" num "]")* ";"
 */
void global_var() {
    Type* ty = basetype();
    char* name = expect_ident();
    ty = read_type_suffix(ty);
    expect(";");
    push_var(name, ty, false);
}

/*
 * declaration = basetype ident ("[" num "]")* ("=" expr) ";"
 */
Node* declaration() {
    Token* tok = token;
    Type* ty = basetype();
    char* name = expect_ident();
    ty = read_type_suffix(ty);
    Var* var = push_var(name, ty, true);

    if (consume(";")) return new_node(NODE_NULL);

    expect("=");
    Node* lhs = new_var(var);
    Node* rhs = expr();
    expect(";");
    Node* node = new_binary(NODE_ASSIGN, lhs, rhs);
    return new_unary(NODE_EXPR_STMT, node);
}

/*
 * stmt = "return" expr ";"
 *      | "if" "(" expr ")" stmt ("else" stmt)?
 *      | "while" "(" expr ")" stmt
 *      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
 *      | "{" stmt* "}"
 *      | declaration
 *      | expr ";"
 */
Node* stmt() {
    if (consume("return")) {
        Node* node = new_node(NODE_RETURN);
        node->lhs = expr();
        expect(";");
        return node;
    } else if (consume("if")) {
        Node* node = new_node(NODE_IF);
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if (consume("else")) {
            node->els = stmt();
        }
        return node;
    } else if (consume("while")) {
        Node* node = new_node(NODE_WHILE);
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        return node;
    } else if (consume("for")) {
        Node* node = new_node(NODE_FOR);
        expect("(");
        if (!consume(";")) {
            node->init = expr();
            expect(";");
        }
        if (!consume(";")) {
            node->cond = expr();
            expect(";");
        }
        if (!consume(")")) {
            node->inc = expr();
            expect(")");
        }
        node->then = stmt();
        return node;
    } else if (consume("{")) {
        Node head;
        head.next = NULL;
        Node* cur = &head;

        while (!consume("}")) {
            cur->next = stmt();
            cur = cur->next;
        }

        Node* node = new_node(NODE_BLOCK);
        node->body = head.next;
        return node;
    }

    if (peek("int")) return declaration();

    Node* node = new_unary(NODE_EXPR_STMT, expr());
    expect(";");
    return node;
}

/*
 * expr = assign
 */
Node* expr() { return assign(); }

/*
 * assign = equality ("=" assign)?
 */
Node* assign() {
    Node* node = equality();

    if (consume("=")) {
        node = new_binary(NODE_ASSIGN, node, assign());
    }

    return node;
}

/*
 * equality = relational ("==" relational | "!=" relational)*
 */
Node* equality() {
    Node* node = relational();

    for (;;) {
        if (consume("==")) {
            node = new_binary(NODE_EQ, node, relational());
        } else if (consume("!=")) {
            node = new_binary(NODE_NE, node, relational());
        } else {
            return node;
        }
    }
}

/*
 * relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 */
Node* relational() {
    Node* node = add();

    for (;;) {
        if (consume("<")) {
            node = new_binary(NODE_LT, node, add());
        } else if (consume("<=")) {
            node = new_binary(NODE_LE, node, add());
        } else if (consume(">")) {
            node = new_binary(NODE_LT, add(), node);
        } else if (consume(">=")) {
            node = new_binary(NODE_LE, add(), node);
        } else {
            return node;
        }
    }
}

/*
 * add = mul ("+" mul | "-" mul)*
 */
Node* add() {
    Node* node = mul();

    for (;;) {
        if (consume("+")) {
            node = new_binary(NODE_ADD, node, mul());
        } else if (consume("-")) {
            node = new_binary(NODE_SUB, node, mul());
        } else {
            return node;
        }
    }
}

/*
 * mul = unary ("*" unary | "/" unary)*
 */
Node* mul() {
    Node* node = unary();

    for (;;) {
        if (consume("*")) {
            node = new_binary(NODE_MUL, node, unary());
        } else if (consume("/")) {
            node = new_binary(NODE_DIV, node, unary());
        } else {
            return node;
        }
    }
}

/*
 * unary = ("+" | "-" | "&" | "*") unary
 *       | postfix
 *       | "sizeof" unary
 */
Node* unary() {
    if (consume("sizeof")) {
        return new_unary(NODE_SIZEOF, unary());
    }

    if (consume("+")) {
        return unary();
    }

    if (consume("-")) {
        return new_binary(NODE_SUB, new_num(0), unary());
    }

    if (consume("&")) {
        return new_unary(NODE_ADDR, unary());
    }

    if (consume("*")) {
        return new_unary(NODE_DEREF, unary());
    }

    return postfix();
}

/*
 * postfix = primary ("[" expr "]")*
 */
Node* postfix() {
    Node* node = primary();

    while (consume("[")) {
        Node* exp = new_binary(NODE_ADD, node, expr());
        expect("]");
        node = new_unary(NODE_DEREF, exp);
    }
    return node;
}

/*
 * primary = "(" expr ")"
 *         | ident ("(" (assign ("," assign)*)? ")")?
 *         | num
 */
Node* primary() {
    if (consume("(")) {
        Node* node = expr();
        expect(")");
        return node;
    }

    Token* tok = consume_ident();
    if (tok) {
        if (consume("(")) {
            Node* node = new_node(NODE_FUNCALL);
            node->funcname = calloc(1, tok->len + 1);
            strncpy(node->funcname, tok->str, tok->len);
            node->argnum = 0;
            node->args = NULL;

            if (consume(")")) {
                return node;
            }

            node->args = expr();
            node->argnum++;
            Node* cur = node->args;

            while (consume(",")) {
                cur->next = expr();
                node->argnum++;
                cur = cur->next;
            }

            expect(")");
            return node;
        }

        Var* var = find_var(tok);
        if (!var) error_tok(tok, "undefined variable");

        return new_var(var);
    }

    return new_num(expect_number());
}
