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

void error_tok(Token* tok, char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int pos = tok->str - user_input;
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

Token* peek(char* s) {
    if (token->kind != TOKEN_RESERVED || strlen(s) != token->len ||
        memcmp(token->str, s, token->len))
        return NULL;
    return token;
}

bool consume(char* s) {
    if (!peek(s)) return false;
    token = token->next;
    return true;
}

Token* consume_ident() {
    if (token->kind != TOKEN_IDENT) return NULL;
    Token* tok = token;
    token = token->next;
    return tok;
}

void expect(char* s) {
    if (!peek(s)) error_tok(token, "Expected '%s'", s);
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

bool starts_with(char* p, char* q) { return strncmp(p, q, strlen(q)) == 0; }

char* starts_with_reserved(char* p) {
    static char* kw[] = {"return", "if",  "else",  "while",
                         "for",    "int", "sizeof"};
    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
        if (starts_with(p, kw[i]) && !is_alnum(p[strlen(kw[i])])) return kw[i];
    }

    static char* ops[] = {"==", "!=", "<=", ">="};
    for (int i = 0; i < sizeof(ops) / sizeof(*ops); i++) {
        if (starts_with(p, ops[i])) return ops[i];
    }

    return NULL;
}

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

        // Multi-character operators or keywords
        char* kw = starts_with_reserved(p);
        if (kw) {
            int len = strlen(kw);
            cur = new_token(TOKEN_RESERVED, cur, p, len);
            p += len;
            continue;
        }

        // Single-character operators
        if (strchr("+-*/&<>(){}[];=,", *p)) {
            cur = new_token(TOKEN_RESERVED, cur, p++, 1);
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
