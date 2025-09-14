#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TOKEN_RESERVED,  // Symbol
    TOKEN_IDENT,     // Identifier
    TOKEN_NUM,       // Integer
    TOKEN_RETURN,    // "return"
    TOKEN_IF,        // "if"
    TOKEN_ELSE,      // "else"
    TOKEN_WHILE,     // "while"
    TOKEN_FOR,       // "for"
    TOKEN_EOF,       // End of file
} TokenKind;

typedef enum {
    NODE_ADD,      // +
    NODE_SUB,      // -
    NODE_MUL,      // *
    NODE_DIV,      // /
    NODE_NUM,      // Number
    NODE_EQ,       // ==
    NODE_NE,       // !=
    NODE_LT,       // <
    NODE_LE,       // <=
    NODE_ASSIGN,   // =
    NODE_LVAR,     // Local variable
    NODE_RETURN,   // "return"
    NODE_IF,       // "if"
    NODE_WHILE,    // "while"
    NODE_FOR,      // "for"
    NODE_BLOCK,    // { ... }
    NODE_FUNCALL,  // Function call
} NodeKind;

typedef struct Token Token;
typedef struct Node Node;
typedef struct LVar LVar;

struct Token {
    TokenKind kind;  // Token type
    Token* next;     // Next token
    int val;         // If kind is TOKEN_NUM, its value
    char* str;       // Token string
    int len;         // Token length
};

struct Node {
    NodeKind kind;   // Node type
    Node* lhs;       // Left hand side
    Node* rhs;       // right hand side
    int val;         // Use only kind is NODE_NUM
    int offset;      // Use only kind is NODE_LVAR
    Node* cond;      // Condition (for if, while, for)
    Node* then;      // Then clause (for if, while, for)
    Node* els;       // Else clause (for if)
    Node* init;      // Initialization (for for)
    Node* inc;       // Increment (for for)
    Node* next;      // Next node (for block statements)
    Node* body;      // Body (for functions)
    Node* args;      // Arguments (for function calls)
    char* funcname;  // Function name (for function calls)
    int argnum;      // Number of arguments (for function calls)
};

struct LVar {
    LVar* next;  // Next variable or NULL
    char* name;  // Variable name
    int len;     // Variable name length
    int offset;  // Offset from RBP
};

extern Token* token;      // Current token
extern char* user_input;  // Input string
extern Node* code[100];   // Abstract syntax tree
extern LVar* locals;      // Local variable list

/// codegen.c

void gen(Node* node);

/// parse.c

Token* tokenize();
void program();
void error(char* fmt, ...);
