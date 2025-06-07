#pragma once

#include "darray.h"
#include <assert.h>

typedef struct {
    char* value_content;
    size_t value_len;
} CSSAttributeValue;

typedef struct{
    CSSAttributeValue* items;
    size_t len, cap;
} CSSAttributeValues;

typedef struct{
    const char* name_content;
    size_t name_len;
    CSSAttributeValues values;
} CSSAttribute;

typedef struct{
    CSSAttribute* items;
    size_t len, cap;
} CSSAttributes;

typedef struct {
    const char* name_content;
    size_t name_len;
    CSSAttributes attrs;
} CSSNode;

typedef struct {
    CSSNode* items;
    size_t len, cap;
} CSSNodes;

int parse_css_attribute(const char* content, CSSAttribute* out, char** end);
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
