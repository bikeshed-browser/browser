#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct JSToken JSToken;
typedef struct {
    JSToken* items;
    size_t len, cap;
} JSTokens;

typedef enum {
    JS_TOK_INTEGER=256,
    JS_TOK_IDENT,
    JS_TOK_LET,
    JS_TOK_CONST,
    JS_TOK_STRLIT,
    JS_TOK_COUNT,
} JSTokType;

struct JSToken {
    JSTokType ttype;
    size_t val; // what this should be casted to depends on `ttype`
};

typedef enum {
    AST_NODE_BINOP,
    AST_NODE_UNARYOP,
    AST_NODE_UNSIGNED_INT,
    AST_NODE_IDENT,
    AST_NODE_STRLIT,
    AST_NODE_FUNC_CALL,
} ASTNodeType;

typedef struct ASTBranch ASTBranch;
struct ASTBranch {
    ASTNodeType type;
    union {
        struct {
            ASTBranch *val1;
            int op; // ascii for the symbol
            ASTBranch *val2;
        } BinOpNode;
        struct {
            int op;
            ASTBranch *val;
        } UnaryOp;
        struct {
            uint64_t val;
        } UnsignedInteger;
        struct {
            char* ident;
            size_t len, cap;  // <--|-- args
            ASTBranch* items; // <--|
        } FunctionCall;
    };
};

typedef struct {
    enum {
        JS_STATEMENT_DEFINE,
        JS_STATEMENT_EXPRESSION,
    } type;
    union {
        struct {
            bool is_const;
            ASTBranch *assign_expr;
        } define_statement;
        ASTBranch* expression_statement;
    };
} JSStatement;

int tokenise_js(JSTokens* toks, char* content);
int gen_ast(JSTokens toks, ASTBranch *ast);
void dump_tokens(JSTokens toks);
int run_js(char* content);
void dump_ast(ASTBranch *ast, int depth);
int js_parse_statement(JSTokens* toks);
int js_parse(JSTokens* toks);
void print_token(JSToken tok);
