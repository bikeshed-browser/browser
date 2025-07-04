#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <darray.h>
#include <jsengine.h>

#ifdef _WIN32
char* strndup(const char* s, size_t n) {
    size_t len = strnlen(s, n);
    char* result = (char*) malloc(len + 1);
    if (!result) return NULL;
    memcpy(result, s, len);
    result[len] = '\0';
    return result;
}
#endif

const char* tok_str_map[] = {
    [JS_TOK_INTEGER] = "Integer",
    [JS_TOK_IDENT]   = "Identifier",
    [JS_TOK_LET]     = "LetKeyword",
    [JS_TOK_CONST]   = "ConstKeyword",
    [JS_TOK_STRLIT]  = "StringLiteral",
};
void print_token(JSToken tok) {
    if (tok.ttype < 256) {
        if (tok.ttype == '\n') printf("(Newline)");
        else printf("(%c)", tok.ttype);
    } else if (tok.ttype >= JS_TOK_COUNT || !tok.ttype) {
        printf("(InvalidToken)");
    } else {
        printf("(%s: ", tok_str_map[tok.ttype]);
        if (tok.ttype == JS_TOK_IDENT || tok.ttype == JS_TOK_STRLIT)
            printf("`%s`)", (char*) tok.val);
        else
            printf("%zu)", tok.val);
    }
}

void dump_tokens(JSTokens toks) {
    for (size_t i = 0; i < toks.len; i++) {
        print_token(toks.items[i]);
        if (i != toks.len - 1) printf(", ");
    }
    printf("\n");
}

int tokenise_js(JSTokens* toks, char* content) {
    for (; *content; content++) {
        if (isspace(*content) && *content != '\n') continue;
        else if (strchr("+-*/\n()!=,;", *content)) {
            da_push(toks, ((JSToken) { .ttype=*content, .val=0 }));
        } else if (isdigit(*content)) {
            char* start = content;
            for (; isdigit(*content); content++);
            char* end = content;
            char* numstr = strndup(start, (int) (end - start));
            int64_t num = strtoll(numstr, NULL, 10);
            free(numstr);
            da_push(toks, ((JSToken) { .ttype=JS_TOK_INTEGER, .val=(size_t)num }));
            content--;
        } else if (isalpha(*content)) {
            char* start = content;
            for (; isalnum(*content) || *content == '_'; content++);
            char* end = content;
            char* s = strndup(start, (int) (end - start)); // must be freed by caller at some point
            if (!strcmp(s, "let")) da_push(toks, ((JSToken) { .ttype=JS_TOK_LET, .val=0 }));
            else if (!strcmp(s, "const")) da_push(toks, ((JSToken) { .ttype=JS_TOK_CONST, .val=0 }));
            else da_push(toks, ((JSToken) { .ttype=JS_TOK_IDENT, .val=(size_t)s }));
            content--;
        } else if (*content == '"' || *content == '\'') {
            char opening_quote = *content; // single or double quote?
            content++;
            char* start = content;
            while (*content && (*content != opening_quote)) content++; // TODO escape support
            char* end = content;
            char* s = strndup(start, (int) (end - start)); // must be freed by caller at some point
            da_push(toks, ((JSToken) { .ttype=JS_TOK_STRLIT, .val=(size_t)s }));
        } else {
            fprintf(stderr, "invalid JS token: %d\n", *content);
            return -1;
        }
    }
    return 0;
}
