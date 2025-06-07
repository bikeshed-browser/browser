#include "fileutils.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define FS_MALLOC malloc
#define FS_DEALLOC(ptr, size) ((void)(size), free(ptr))
#define defer_return(x) do { result = (x); goto DEFER; } while(0)
const char* read_entire_file(const char* path, size_t* size) {
    char* result = NULL;
    char* head = NULL;
    char* end = NULL;
    size_t buf_size = 0;
    long at = 0;
    FILE *f = fopen(path, "rb");

    if(!f) {
        fprintf(stderr, "ERROR Could not open file %s: %s\n",path,strerror(errno));
        return NULL;
    }
    if(fseek(f, 0, SEEK_END) != 0) {
        fprintf(stderr, "ERROR Could not fseek on file %s: %s\n",path,strerror(errno));
        defer_return(NULL);
    }
    at = ftell(f);
    if(at == -1L) {
        fprintf(stderr, "ERROR Could not ftell on file %s: %s\n",path,strerror(errno));
        defer_return(NULL);
    }
    *size = at;
    buf_size = at+1;
    rewind(f); // Fancy :D
    result = FS_MALLOC(buf_size);
    assert(result && "Ran out of memory");
    head = result;
    end = result+buf_size-1;
    while(head != end) {
        head += fread(head, 1, end-head, f);
        if(ferror(f)) {
            fprintf(stderr, "ERROR Could not fread on file %s: %s\n",path,strerror(errno));
            FS_DEALLOC(result, buf_size);
            defer_return(NULL);
        }
        // TODO: Think about checking with feof
    }
    result[buf_size-1] = '\0';
DEFER:
    fclose(f);
    return result;
}

void remove_carrige_return(char* content){
    char* cur = content;
    while ((cur = strchr(cur, '\r'))) {
        memmove(cur, cur + 1, strlen(cur + 1) + 1);
    }
}