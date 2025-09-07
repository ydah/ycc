#include "ycc.h"

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // Print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool consume(char* op) {
  if (token->kind != TOKEN_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

void expect(char* op) {
  if (token->kind != TOKEN_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "Expected '%c'", op);
  token = token->next;
}

int expect_number() {
  if (token->kind != TOKEN_NUM)
    error_at(token->str, "Expected a number");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TOKEN_EOF; }

Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

Token *tokenize() {
  Token head;
  head.next = NULL;
  Token *cur = &head;
  char *p = user_input;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    // Multi-character operators
    if (strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0 ||
        strncmp(p, "<=", 2) == 0 || strncmp(p, ">=", 2) == 0) {
      cur = new_token(TOKEN_RESERVED, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }

    // Single-character operators
    if (strchr("+-*/<>()", *p)) {
      cur = new_token(TOKEN_RESERVED, cur, p++);
      cur->len = 1;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TOKEN_NUM, cur, p);
      char* end = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - end;
      continue;
    }

    error_at(p, "Invalid token");
  }

  new_token(TOKEN_EOF, cur, p);
  return head.next;
}

Node* new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node* new_binary(NodeKind kind, Node* lhs, Node* rhs) {
  Node* node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node* new_node_num(int val) {
  Node* node = new_node(NODE_NUM);
  node->val = val;
  return node;
}

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Node* expr() {
  return equality();
}

Node* equality() {
  Node* node = relational();

  for(;;) {
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

  for(;;) {
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

  for(;;) {
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

  for(;;) {
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
    return new_binary(NODE_SUB, new_node_num(0), unary());
  }

  return primary();
}

Node* primary() {
  if (consume("(")) {
    Node* node = expr();
    expect(")");
    return node;
  }

  return new_node_num(expect_number());
}
