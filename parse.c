#include "ycc.h"

VarList* locals = NULL;  // Local variable list

Var* find_var(Token* tok) {
    for (VarList* vl = locals; vl; vl = vl->next) {
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

Var* push_var(char* name) {
    Var* var = calloc(1, sizeof(Var));
    var->name = name;
    VarList* vl = calloc(1, sizeof(VarList));
    vl->var = var;
    vl->next = locals;
    locals = vl;
    return var;
}

Function* function();
Node* stmt();
Node* expr();
Node* assign();
Node* equality();
Node* relational();
Node* add();
Node* mul();
Node* unary();
Node* primary();

Function* program() {
    Function head;
    head.next = NULL;
    Function* cur = &head;

    while (!at_eof()) {
        cur->next = function();
        cur = cur->next;
    }

    return head.next;
}

VarList* read_func_params() {
    if (consume(")")) {
        return NULL;
    }

    VarList* head = calloc(1, sizeof(VarList));
    head->var = push_var(expect_ident());
    VarList* cur = head;

    while (!consume(")")) {
        expect(",");
        cur->next = calloc(1, sizeof(VarList));
        cur = cur->next;
        cur->var = push_var(expect_ident());
    }

    return head;
}

Function* function() {
    locals = NULL;

    Function* fn = calloc(1, sizeof(Function));
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
    } else if (consume("int")) {
        Token* tok = consume_ident();
        if (!tok) error("expected an identifier after int");

        Var* var = find_var(tok);
        if (var) error_tok(tok, "redefinition of variable");

        var = push_var(strndup(tok->str, tok->len));
        expect(";");
        return new_var(var);
    }

    Node* node = new_node(NODE_EXPR_STMT);
    node->lhs = expr();
    expect(";");
    return node;
}

Node* expr() { return assign(); }

Node* assign() {
    Node* node = equality();

    if (consume("=")) {
        node = new_binary(NODE_ASSIGN, node, assign());
    }

    return node;
}

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

Node* unary() {
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

    return primary();
}

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
