#include "cssparser.h"
#include "darray.h"
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>

#define CSS_MALLOC malloc
#define CSS_FREE free

const char* csserr_strtab[] = {
    [CSSERR_TODO] = "Unimplemented",
    [CSSERR_EOF]  = "End of File",
    [CSSERR_UNEXPECTED_LEXEM]  = "Unexpected Lexem",
    [CSSERR_ATTR] = "Couldn't parse attribute"
};

const char* csserr_str(int err) {
    if(err >= 0) return "OK";
    err = -err;
    if(err >= CSSERR_COUNT) return "Unknown error";
    return csserr_strtab[err];
}

enum {
    CSSLEXEM_SPECIALCHAR,
    CSSLEXEM_TEXT
};

typedef struct{
    char* content;
    size_t len;
    size_t type;
} CSSLexem;

const char css_special_characters[] = {
    '{',
    '}',
    ';',
    ':',
};

//TODO: better way to trim comments
char* trim_left(const char* content) {
    char* cur = (char*)content;
    while (*cur != '\0') {
        if (isspace(*cur)) {
            cur++;
            continue;
        }
        
        if (cur[0] == '/' && cur[1] == '*') {
            cur += 2;
            
            while (*cur != '\0') {
                if (cur[0] == '*' && cur[1] == '/') {
                    cur += 2;
                    break;
                }
                cur++;
            }
            continue;
        }
        
        break;
    }
    return cur;
}

int is_special_char(char ch){
    for(size_t i = 0; i < sizeof(css_special_characters)/sizeof(css_special_characters[0]); i++){
        if(ch == css_special_characters[i]){
            return i;
        }
    }
    return -1;
}

int chop_css_lexem(const char* content, CSSLexem* out, char** end){
    char* cur = (char*)content;
    cur = trim_left(cur);
    if(*cur == 0) return -CSSERR_EOF;

    //checking if lexem is special char
    int isSpecialChar = is_special_char(*cur);
    if(isSpecialChar >= 0){
        out->type = CSSLEXEM_SPECIALCHAR;
        out->content=cur;
        out->len=1;
        *end = cur+1;
        return 0;
    }

    out->type = CSSLEXEM_TEXT;
    out->content = cur;

    while(!isspace(cur[0]) && cur[0] != '/' && is_special_char(cur[0]) == -1) cur++;

    out->len = cur - out->content;
    *end = cur;
    return 0;
}

int expect_special_lexem(const char* content, char ch, char** end){
    char* cur = (char*)content;
    CSSLexem lexem = {0};
    int result = chop_css_lexem(cur,&lexem, &cur);
    if(result < 0) return result;

    if(lexem.type != CSSLEXEM_SPECIALCHAR) return -CSSERR_UNEXPECTED_LEXEM;
    if(*lexem.content != ch) return -CSSERR_UNEXPECTED_LEXEM;

    *end = cur;
    return 0;
}

int parse_css_attribute(const char* content, CSSAttribute* out, char** end){
    char* cur = (char*)content;
    CSSLexem lexem = {0};
    int result = chop_css_lexem(cur,&lexem, &cur);
    if(result < 0) return result;
    if(lexem.type == CSSLEXEM_SPECIALCHAR && *lexem.content == '}') return 1;

    if(lexem.type != CSSLEXEM_TEXT) return -CSSERR_UNEXPECTED_LEXEM;
    out->name_content = lexem.content;
    out->name_len = lexem.len;

    result = expect_special_lexem(cur, ':', &cur);
    if(result < 0) return result;

    CSSAttributeValues vals = {0};
    
    while(true){
        result = chop_css_lexem(cur,&lexem, &cur);
        if(result < 0) {
            CSS_FREE(vals.items);
            return result;
        }

        if(lexem.type == CSSLEXEM_SPECIALCHAR && *lexem.content == ';'){
            cur++;
            break;
        }

        if(lexem.type != CSSLEXEM_TEXT) {
            CSS_FREE(vals.items);
            return -CSSERR_UNEXPECTED_LEXEM;
        }

        da_push(&vals, ((CSSAttributeValue){.value_content = lexem.content, .value_len = lexem.len}));
    }

    out->values = vals;

    *end = cur;
    return 0;
}

int parse_css_node(const char* content, CSSNode* out, char** end){
    char* cur = (char*)content;
    CSSLexem lexem = {0};
    int result = chop_css_lexem(cur,&lexem, &cur);
    if(result < 0) return result;

    if(lexem.type != CSSLEXEM_TEXT) return -CSSERR_UNEXPECTED_LEXEM;
    out->name_content = lexem.content;
    out->name_len = lexem.len;

    result = expect_special_lexem(cur, '{', &cur);
    if(result < 0) return result;

    CSSAttributes attrs = {0};
    CSSAttribute attr = {0};

    while(true){
        result = parse_css_attribute(cur, &attr, &cur);
        if(result == 1) break;
        if(result < 0) {
            CSS_FREE(attrs.items);
            return result;
        }
        da_push(&attrs, attr);
    }

    result = expect_special_lexem(cur, '}', &cur);
    if(result < 0) {
        CSS_FREE(attrs.items);
        return result;
    }

    out->attrs = attrs;
    *end = cur;
    return 0;
}

int parse_css_file(const char* content, CSSNodes* outNodes){
    char* cur = (char*)content;
    CSSNode node = {0};

    int result = 0;
    while(*cur != '\0'){
        result = parse_css_node(cur, &node, &cur);
        if(result < 0) return result;
        da_push(outNodes, node);
    }

    return 0;
}
