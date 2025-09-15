#include "ycc.h"

char* user_input;  // Input string
Token* token;      // Current token

void error(char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char* loc, char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");  // Print pos spaces.
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

char* strndup(char* p, int len) {
    char* buf = malloc(len + 1);
    strncpy(buf, p, len);
    buf[len] = '\0';
    return buf;
}

bool consume(char* op) {
    if ((token->kind == TOKEN_IDENT || token->kind == TOKEN_NUM) ||
        strlen(op) != token->len || memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

Token* consume_ident() {
    if (token->kind != TOKEN_IDENT) return NULL;
    Token* tok = token;
    token = token->next;
    return tok;
}

void expect(char* op) {
    if (token->kind != TOKEN_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "Expected '%s'", op);
    token = token->next;
}

char* expect_ident() {
    if (token->kind != TOKEN_IDENT)
        error_at(token->str, "Expected an identifier");
    char* s = strndup(token->str, token->len);
    token = token->next;
    return s;
}

int expect_number() {
    if (token->kind != TOKEN_NUM) error_at(token->str, "Expected a number");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() { return token->kind == TOKEN_EOF; }

Token* new_token(TokenKind kind, Token* cur, char* str, int len) {
    Token* tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool is_alpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

bool is_alnum(char c) { return is_alpha(c) || ('0' <= c && c <= '9'); }

Token* tokenize() {
    Token head;
    head.next = NULL;
    Token* cur = &head;
    char* p = user_input;

    while (*p) {
        // Skip whitespace characters
        if (isspace(*p)) {
            p++;
            continue;
        }

        // Multi-character operators
        if (strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0 ||
            strncmp(p, "<=", 2) == 0 || strncmp(p, ">=", 2) == 0) {
            cur = new_token(TOKEN_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        // Single-character operators
        if (strchr("+-*/&<>(){};=,", *p)) {
            cur = new_token(TOKEN_RESERVED, cur, p++, 1);
            continue;
        }

        // return
        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TOKEN_RETURN, cur, p, 6);
            p += 6;
            continue;
        }

        // if
        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            cur = new_token(TOKEN_IF, cur, p, 2);
            p += 2;
            continue;
        }

        // else
        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TOKEN_ELSE, cur, p, 4);
            p += 4;
            continue;
        }

        // while
        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            cur = new_token(TOKEN_WHILE, cur, p, 5);
            p += 5;
            continue;
        }

        // for
        if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
            cur = new_token(TOKEN_FOR, cur, p, 3);
            p += 3;
            continue;
        }

        // Number
        if (isdigit(*p)) {
            cur = new_token(TOKEN_NUM, cur, p, 0);
            char* start = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - start;
            continue;
        }

        // Identifier
        if (is_alpha(*p)) {
            cur = new_token(TOKEN_IDENT, cur, p, 0);
            char* start = p;
            while (is_alnum(*p)) p++;
            cur->len = p - start;
            continue;
        }

        error_at(p, "Invalid token");
    }

    new_token(TOKEN_EOF, cur, p, 0);
    return head.next;
}
