#pragma once

#include "darray.h"
#include <assert.h>

typedef struct {
    char* value_content;
    size_t value_len;
} CSSAttrVal;

typedef struct{
    CSSAttrVal* items;
    size_t len,cap;
} CSSAttrVals;

typedef struct{
    const char* name_content;
    size_t name_len;
    CSSAttrVals values;
} CSSAttr;

typedef struct{
    CSSAttr* items;
    size_t len,cap;
} CSSAttrs;

typedef struct {
    const char* name_content;
    size_t name_len;
    CSSAttrs attrs;
} CSSNode;

typedef struct {
    CSSNode* items;
    size_t len,cap;
} CSSNodes;

int parse_css_node(const char* content, CSSNode* out, char** end);
int parse_css_file(const char* content, CSSNodes* outNodes);

enum {
    CSSERR_TODO=1,
    CSSERR_EOF,
    CSSERR_UNEXPECTED_LEXEM,
    CSSERR_ATTR,
    CSSERR_COUNT
};
static_assert(CSSERR_COUNT == 5, "Update csserr_strtab");

extern const char* csserr_strtab[CSSERR_COUNT];
const char* csserr_str(int err);