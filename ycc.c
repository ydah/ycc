#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TOKEN_RESERVED, // Symbol
  TOKEN_NUM,      // Integer
  TOKEN_EOF,      // End of file
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind; // Token type
  Token *next;    // Next token
  int val;        // If kind is TOKEN_NUM, its value
  char *str;      // Token string
};

Token *token; // Current token

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool consume(char op) {
  if (token->kind != TOKEN_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

void expect(char op) {
  if (token->kind != TOKEN_RESERVED || token->str[0] != op)
    error("Expected '%c'", op);
  token = token->next;
}

int expect_number() {
  if (token->kind != TOKEN_NUM)
    error("Expected a number");
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

Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-') {
      cur = new_token(TOKEN_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TOKEN_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error("Invalid token: %s", p);
  }

  new_token(TOKEN_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Invalid number of arguments\n");
    return 1;
  }

  token = tokenize(argv[1]);

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  printf("  mov rax, %d\n", expect_number());

  while (!at_eof()) {
    if (consume('+')) {
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}
