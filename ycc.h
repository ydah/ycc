#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TOKEN_RESERVED, // Symbol
  TOKEN_IDENT,    // Identifier
  TOKEN_NUM,      // Integer
  TOKEN_EOF,      // End of file
} TokenKind;

typedef enum {
  NODE_ADD, // +
  NODE_SUB, // -
  NODE_MUL, // *
  NODE_DIV, // /
  NODE_NUM, // Number
  NODE_EQ,  // ==
  NODE_NE,  // !=
  NODE_LT,  // <
  NODE_LE,  // <=
  NODE_ASSIGN, // =
  NODE_LVAR,  // Local variable
} NodeKind;

typedef struct Token Token;
typedef struct Node Node;

struct Token {
  TokenKind kind; // Token type
  Token *next;    // Next token
  int val;        // If kind is TOKEN_NUM, its value
  char *str;      // Token string
  int len;        // Token length
};

struct Node {
  NodeKind kind; // Node type
  Node* lhs;     // Left hand side
  Node* rhs;     // right hand side
  int val;       // Use only kind is NODE_NUM
  int offset;    // Use only kind is NODE_LVAR
};

extern Token *token; // Current token
extern char *user_input; // Input string
extern Node* code[100]; // Abstract syntax tree

/// codegen.c

void gen(Node* node);

/// parse.c

Token* tokenize();
void program();
void error(char *fmt, ...);
