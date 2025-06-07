#include <stdio.h>
#include <raylib.h>
#include <fileutils.h>
#include <string.h>
#include <darray.h>
#include <assert.h>
#include <ctype.h>

#include "cssparser.h"

int cssmain(){
    const char* example_path = "examples/sample.css";
    size_t content_size;
    char* content_data = (char*)read_entire_file(example_path, &content_size);
    char* content = content_data;
    remove_carrige_return(content);

    CSSNodes cssNodes = {0};
    int result = parse_css_file(content,&cssNodes);
    if(result < 0) {
        printf("CSSERR: %s\n", csserr_str(result));
        return 1;
    }

    printf("Parsed: %zu", cssNodes.len);
    for(size_t i = 0; i < cssNodes.len; i++){
        CSSNode* node = &cssNodes.items[i];
        printf("\n----\"%.*s\"-----\n", (int)node->name_len, node->name_content);
        if(node->attrs.len > 0) printf("attrs:\n");
        for(size_t j = 0; j < node->attrs.len; j++){
            CSSAttribute* attr = &node->attrs.items[j];
            printf("name: %.*s, values: ", (int)attr->name_len, attr->name_content);
            for(size_t m = 0; m < attr->values.len; m++){
                CSSAttributeValue* attrVal = &attr->values.items[m];
                printf("%.*s ", (int)attrVal->value_len, attrVal->value_content);
            }
            printf("\n");
        }
    }

    return 0;
}
