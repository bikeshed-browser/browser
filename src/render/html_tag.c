#include <render/html_tag.h> 
#include <string.h>
#include <ctype.h>
#include <html.h>
#include <atom.h>

void render_html_tag(HTMLTag* tag, Font font, float fontSize, float textFontSize, float spacing, float scroll_y) {
    if(tag->name) {
        if(tag->name->len == 5 && strncmp(tag->name->data, "style", 5) == 0) return;
        if(tag->name->len == 5 && strncmp(tag->name->data, "title", 5) == 0) return;
        if(strcmp(tag->name->data, "script") == 0) return;
        for(size_t i = 0; i < tag->children.len; ++i) {
            render_html_tag(tag->children.items[i], font, fontSize, tag->fontSize, spacing, scroll_y);
        }
    } else {
        float x = tag->x, y = tag->y; 
        for(size_t i = 0; i < tag->str_content_len; ++i) {
            char c = tag->str_content[i];
            if(isspace(c)) {
                while(i+1 < tag->str_content_len && isspace(tag->str_content[i+1])) i++;
                c = ' ';
            } else if (!isgraph(c)) c = '?';

            Vector2 size = MeasureCodepointEx(font, c, textFontSize, spacing);
            if(x + size.x > tag->x + tag->width) {
                //TODO: Unhardcode this?
                x = 0;// tag->x;
                y += textFontSize;
            }
            DrawTextCodepoint(font, c, (Vector2){x, y + scroll_y}, textFontSize, BLACK);
            x += size.x;
        }
    }
}
