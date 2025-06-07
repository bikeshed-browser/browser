#include <stdio.h>
#include <raylib.h>
#include <fileutils.h>
#include <string.h>
#include <darray.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>

#define W_RATIO 16
#define H_RATIO 9
#define SCALE 100
#define WIDTH  W_RATIO*SCALE
#define HEIGHT H_RATIO*SCALE
typedef struct HTMLAttribute HTMLAttribute;
typedef struct {
    HTMLAttribute** items;
    size_t len, cap;
} HTMLAttributes;
struct HTMLAttribute {
    char* key;
    size_t key_len;
    char* value; // value is NULL if it has no value
    size_t value_len;
};
typedef struct HTMLTag HTMLTag;
typedef struct {
    HTMLTag** items;
    size_t len, cap;
} HTMLTags;
struct HTMLTag {
    HTMLTag* parent;
    const char* name;
    size_t name_len;
    HTMLTags children;
    const char* str_content;
    size_t str_content_len;
    HTMLAttributes attributes;
    bool self_closing;
};
enum {
    HTMLERR_TODO=1,
    HTMLERR_EOF,
    HTMLERR_INVALID_TAG,
    HTMLERR_INVALID_ATTRIBUTE,
    HTMLERR_COUNT,
};
static_assert(HTMLERR_COUNT == 5, "Update htmlerr_strtab");
const char* htmlerr_strtab[] = {
    [HTMLERR_TODO] = "Unimplemented",
    [HTMLERR_EOF]  = "End of File",
    [HTMLERR_INVALID_TAG] = "Invalid tag format",
    [HTMLERR_INVALID_ATTRIBUTE]  = "Invalid attribute format",
};
const char* htmlerr_str(int err) {
    if(err >= 0) return "OK";
    err = -err;
    if(err >= HTMLERR_COUNT) return "Unknown error";
    return htmlerr_strtab[err];
}

int parse_attribute(const char* content, HTMLAttribute* att, const char** end) {
    att->key = (char*)content;
    while (isalnum(*content) || *content == '_' || *content == '-')
        content++;
    att->key_len = content - att->key;
    while(isspace(*content)) content++;
    if (*content != '=') {
        // it has no value
        *end = content;
        return 0;
    }
    content++;
    while (isspace(*content)) content++;
    if (*content != '"') return -HTMLERR_INVALID_ATTRIBUTE;
    content++;
    att->value = (char*)content;
    while(*content && *content != '"') content++;
    if(*content != '"') return -HTMLERR_EOF;
    att->value_len = content - att->value;
    content++;
    *end = content;
    return 0;
}

void dump_attributes(HTMLTag* tag) {
    if (!tag->attributes.len) return;
    printf("Tag has attributes, dump:\n");
    for (size_t i = 0; i < tag->attributes.len; i++) {
        HTMLAttribute *attr = tag->attributes.items[i];
        printf("Tag: key(%.*s)->value(%.*s)\n",
                (int)attr->key_len, attr->key,
                (int)attr->value_len, attr->value
        );
    }
}

#define STRINGIFY0(x) # x
#define STRINGIFY1(x) STRINGIFY0(x)
#define todof(...) (fprintf(stderr, "TODO " __FILE__ ":" STRINGIFY1(__LINE__) ":" __VA_ARGS__), abort())
int html_parse_next_tag(const char* content, HTMLTag* tag, char** end) {
    if(*content == '<') {
        content++;
        tag->name = content;
        while(isalnum(*content)) content++;
        tag->name_len = content - tag->name;
        while (*content && *content != '>' && *content != '/') {
            while(isspace(*content)) content++;
            int e;
            HTMLAttribute *att = (HTMLAttribute*) malloc(sizeof(HTMLAttribute));
            assert(att && "Just buy more RAM");
            memset(att, 0, sizeof(*att));
            if ((e=parse_attribute(content, att, &content)) < 0) return e;
            da_push(&tag->attributes, att);
        }
        if (*content == '/') {
            if (content[1] != '>') return -HTMLERR_INVALID_TAG;
            content += 2;
            *end = (char*) content;
            tag->self_closing = true;
            return 0;
        }
        tag->self_closing = false;
        dump_attributes(tag);
        content++;
        *end = (char*)content;
        return 0;
    }
    if(*content == '\0') return -HTMLERR_EOF;
    tag->str_content = content;
    while(*content != '<' && *content) content++;
    tag->str_content_len = content - tag->str_content;
    *end = (char*)content;
    return 0;
}

//TODO: Handle html comments `<!-- html comment!-->` because it cannot load title in motherfuckingwebsite.html
void dump_html_tag(HTMLTag* tag, size_t indent) {
    if(tag->name) {
        if(tag->name_len == 5 && strncmp(tag->name, "style", 5) == 0) return;
        printf("%*s<%.*s>\n", (int)indent, "", (int)tag->name_len, tag->name);
        for(size_t i = 0; i < tag->children.len; ++i) {
            dump_html_tag(tag->children.items[i], indent + 4);
        }
        printf("%*s</%.*s>\n", (int)indent, "", (int)tag->name_len, tag->name);
    } else {
        printf("%*s", (int)indent, "");
        for(size_t i = 0; i < tag->str_content_len; ++i) {
            char c = tag->str_content[i];
            if(isgraph(c) || c == ' ') printf("%c", c);
            else printf("\\x%02X", c);
        }
        printf("\n");
    }
}
void render_html_tag(HTMLTag* tag, float fontSize, float* rx, float* ry) {
    if(tag->name) {
        if(tag->name_len == 5 && strncmp(tag->name, "style", 5) == 0) return;
        if(tag->name_len == 5 && strncmp(tag->name, "title", 5) == 0) return;
        for(size_t i = 0; i < tag->children.len; ++i) {
            float childFontSize = fontSize;
            if(strncmp(tag->name, "h1", 2) == 0) childFontSize = 24.0;
            else if(strncmp(tag->name, "h2", 2) == 0) childFontSize = 20.0;
            else if(strncmp(tag->name, "h3", 2) == 0) childFontSize = 18.0;
            else if(strncmp(tag->name, "p", 1) == 0) childFontSize = 12.0;
            else if(strncmp(tag->name, "li", 2) == 0) childFontSize = 10.0;
            else {
                childFontSize = 11.0;
            }
            render_html_tag(tag->children.items[i], childFontSize, rx, ry);
        }
    } else {
        Font font = GetFontDefault();
        float x = *rx, y = *ry; 
        size_t width = GetScreenWidth();
        for(size_t i = 0; i < tag->str_content_len; ++i) {
            char c = tag->str_content[i];
            if(!isgraph(c) && c != ' ') c = '?';
            if(x + fontSize > width) {
                x = *rx;
                y += fontSize;
            }
            size_t index = GetGlyphIndex(font, c);
            Vector2 pos = {
                x, y
            };
            DrawTextCodepoint(font, c, pos, fontSize, BLACK);
            float scaleFactor = fontSize/font.baseSize;
            int defaultFontSize = 10;   // Default Font chars height in pixel
            if (fontSize < defaultFontSize) fontSize = defaultFontSize;
            int spacing = fontSize/defaultFontSize;
            if (font.glyphs[index].advanceX == 0) x += ((float)font.recs[index].width*scaleFactor + spacing);
            else x += ((float)font.glyphs[index].advanceX*scaleFactor + spacing);
        }
        *ry = y + fontSize;
    }
}
HTMLTag* find_child_html_tag(HTMLTag* tag, const char* name) {
    if(!tag) return NULL;
    size_t name_len = strlen(name);
    for(size_t i = 0; i < tag->children.len; ++i) {
        HTMLTag* child = tag->children.items[i];
        if(child->name_len == name_len && memcmp(child->name, name, name_len) == 0) return child;
    }
    return NULL;
}
char* shift_args(int* argc, char*** argv) {
    return (*argc) <= 0 ? NULL : ((*argc)--, *((*argv)++));
}
void help(FILE* sink, const char* exe) {
    fprintf(sink, "%s <input path>\n", exe);
}
int main(int argc, char** argv) {
    const char* exe = shift_args(&argc, &argv);
    const char* example_path = NULL;

    while(argc) {
        const char* arg = shift_args(&argc, &argv);
        if (strcmp(arg, "--help") == 0) {
            help(stdout, exe);
            return 0;
        }
        else if(!example_path) example_path = arg;
        else {
            fprintf(stderr, "ERROR Unexpected argument: `%s`\n", arg);
            help(stderr, exe);
            return 1;
        }
    }
    if(!example_path) {
        fprintf(stderr, "ERROR Missing input path!\n");
        help(stderr, exe);
        return 1;
    }

    size_t content_size;
    char* content_data = (char*)read_entire_file(example_path, &content_size);
    if(!content_data) return 1;
    char* content = content_data;
    bool quirks_mode = true;
    if(!memcmp(content, "<!DOCTYPE html>", 15)) {
        content += 15;
        quirks_mode = false;
    }
    (void) quirks_mode;
    HTMLTag root = { 0 };
    root.name = "\\root";
    root.name_len = strlen(root.name);
    HTMLTag* node = &root;
    for(;;) {
        while(isspace(*content)) content++;
        if(*content == '\0') break;
        if(content[0] == '<' && content[1] == '/') {
            while(*content != '>' && *content) content++;
            content++;
            node = node->parent;
            continue;
        }
        if(content[0] == '<' && content[1] == '!' && content[2] == '-' && content[3] == '-') {
            content += 4;
            while(*content && (content[0] != '-' || content[1] != '-' || content[2] != '>')) content++;
            if(*content) content += 3;
            continue;
        }
        HTMLTag* tag = malloc(sizeof(*tag));
        assert(tag && "Just buy more RAM");
        memset(tag, 0, sizeof(*tag));
        int e = html_parse_next_tag(content, tag, &content);
        if(e == -HTMLERR_EOF) break;
        tag->parent = node; 
        da_push(&node->children, tag);
        if (!tag->self_closing && tag->name) node = tag;
        if(e != 0) {
            fprintf(stderr, "Failed to parse tag: %s\n", htmlerr_str(e));
            return 1;
        }
    }
    if(node != &root) {
        fprintf(stderr, "WARN: Some unclosed tags.\n");
    }
    dump_html_tag(node, 0);

    HTMLTag* html = find_child_html_tag(&root, "html");
    HTMLTag* head = find_child_html_tag(html, "head");
    HTMLTag* title = find_child_html_tag(head, "title");
    const char* window_title = "Bikeshed";
    if(title && title->children.len && !title->children.items[0]->name) {
        HTMLTag* title_str = title->children.items[0];
        window_title = TextFormat("Bikeshed - %.*s", (int)title_str->str_content_len, title_str->str_content);
    }
    InitWindow(WIDTH, HEIGHT, window_title);
    SetTargetFPS(60);
    float scroll_y = 0;
    while(!WindowShouldClose()) {
        scroll_y += GetMouseWheelMove()*6.0;
        BeginDrawing();
        ClearBackground(RAYWHITE);
        float x = 0, y = scroll_y;
        render_html_tag(node, 24.0, &x, &y);
        EndDrawing();
    }
    CloseWindow();
}
