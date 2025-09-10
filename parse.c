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

LVar* find_lvar(Token* tok) {
  for (LVar* var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

bool consume(char* op) {
  if (!(token->kind == TOKEN_RESERVED ||
       token->kind == TOKEN_RETURN) ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

Token* consume_ident() {
  if (token->kind != TOKEN_IDENT)
    return NULL;
  Token *tok = token;
  token = token->next;
  return tok;
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

bool is_alpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

bool is_alnum(char c) {
  return is_alpha(c) || ('0' <= c && c <= '9');
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
    if (strchr("+-*/<>();=", *p)) {
      cur = new_token(TOKEN_RESERVED, cur, p++);
      cur->len = 1;
      continue;
    }

    // return
    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TOKEN_RETURN, cur, p);
      cur->len = 6;
      p += 6;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TOKEN_NUM, cur, p);
      char* start = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - start;
      continue;
    }

    if (is_alpha(*p)) {
      cur = new_token(TOKEN_IDENT, cur, p);
      char* start = p;
      while (is_alnum(*p))
        p++;
      cur->len = p - start;
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

Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void program() {
  int i = 0;
  while (!at_eof()) {
    code[i++] = stmt();
  }
  code[i] = NULL;
}

Node* stmt() {
  Node *node;
  if (consume("return")) {
    node = calloc(1, sizeof(Node));
    node->kind = NODE_RETURN;
    node->lhs = expr();
  } else {
    node = expr();
  }

  if (!consume(";"))
    error_at(token->str, "Expected ';'");
  return node;
}

Node* expr() {
  return assign();
}

Node* assign() {
  Node* node = equality();

  if (consume("=")) {
    node = new_binary(NODE_ASSIGN, node, assign());
  }

  return node;
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

  Token *tok = consume_ident();
  if (tok) {
    Node* node = calloc(1, sizeof(Node));
    node->kind = NODE_LVAR;

    LVar* lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals ? locals->offset + 8 : 8;
      node->offset = lvar->offset;
      locals = lvar;
    }

    return node;
  }

  return new_node_num(expect_number());
}
