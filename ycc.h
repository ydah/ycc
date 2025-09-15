#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// parse.c

typedef struct Var Var;
struct Var {
    Var* next;   // Next variable or NULL
    char* name;  // Variable name
    int offset;  // Offset from RBP
};

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
    NODE_VAR,      // Variable
    NODE_RETURN,   // "return"
    NODE_IF,       // "if"
    NODE_WHILE,    // "while"
    NODE_FOR,      // "for"
    NODE_BLOCK,    // { ... }
    NODE_FUNCALL,  // Function call
    NODE_EXPR_STMT, // Expression statement
} NodeKind;

typedef struct Node Node;
struct Node {
    NodeKind kind;   // Node type
    Node* lhs;       // Left hand side
    Node* rhs;       // right hand side
    int val;         // Use only kind is NODE_NUM
    Var* var;        // Use only kind is NODE_VAR
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

typedef struct {
    Node* node;      // AST root
    Var* locals;     // Local variable list
    int stack_size;  // Total stack size needed for locals
} Program;

Program* program();

// tokenize.c

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

typedef struct Token Token;
struct Token {
    TokenKind kind;  // Token type
    Token* next;     // Next token
    int val;         // If kind is TOKEN_NUM, its value
    char* str;       // Token string
    int len;         // Token length
};

void error(char* fmt, ...);
void error_at(char* loc, char* fmt, ...);
bool consume(char* op);
char* strndup(char* p, int len);
Token* consume_ident();
void expect(char* op);
int expect_number();
bool at_eof();
Token* new_token(TokenKind kind, Token* cur, char* str, int len);
Token* tokenize();

extern Token* token;      // Current token
extern char* user_input;  // Input string

/// codegen.c

void codegen(Program* prog);
