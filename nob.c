#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#ifdef _WIN32
#include <io.h>
#define F_OK 0
#define X_OK 0
#define access _access
#define PATH_SEPARATOR ";"
#else
#include <unistd.h>
#define PATH_SEPARATOR ":"
#endif

bool find_program(
    const char* name,
    char* out_path,
    size_t out_path_size
) {
    const char* path_env = getenv("PATH");
    if (!path_env) return false;

    size_t temp = nob_temp_save();

    char* paths = temp_strdup(path_env);
    if (!paths) return false;

    char* dir = strtok(paths, PATH_SEPARATOR);
    while(dir) {
#ifdef _WIN32
        snprintf(out_path, out_path_size, "%s\\%s.exe", dir, name);
        if (access(out_path, X_OK) == 0) {
            nob_temp_rewind(temp);
            return true;
        }
#else
        snprintf(out_path, out_path_size, "%s/%s", dir, name);
        if (access(out_path, X_OK) == 0) {
            nob_temp_rewind(temp);
            return true;
        }
#endif
        dir = strtok(NULL, PATH_SEPARATOR);
    }
    nob_temp_rewind(temp);
    return false;
}

static bool walk_directory(
    File_Paths* dirs,
    File_Paths* c_sources,
    const char* path
) {
    DIR *dir = opendir(path);
    if(!dir) {
        nob_log(NOB_ERROR, "Could not open directory %s: %s", path, strerror(errno));
        return false;
    }
    errno = 0;
    struct dirent *ent;
    while((ent = readdir(dir))) {
        if(strcmp(ent->d_name, "..") == 0 || strcmp(ent->d_name, ".") == 0) continue;
        const char* fext = nob_get_ext(ent->d_name);
        size_t temp = nob_temp_save();
        const char* p = nob_temp_sprintf("%s/%s", path, ent->d_name);
        Nob_File_Type type = nob_get_file_type(p);
        if(type == NOB_FILE_DIRECTORY) {
            da_append(dirs, p);
            if(!walk_directory(dirs, c_sources, p)) {
                closedir(dir);
                return false;
            }
            continue;
        }
        if(strcmp(fext, "c") == 0) {
            nob_da_append(c_sources, p);
            continue;
        }
        nob_temp_rewind(temp);
    }
    closedir(dir);
    return true;
}
int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    char* cc = getenv("CC");
    if(!cc) {
#ifdef _WIN32
        char compiler_path[512];
        if(find_program("clang", compiler_path, sizeof(compiler_path))) {
            cc = "clang";
        } else {
            nob_log(ERROR, "Clang was not found on your system and CC was not set. Please install clang or set your CC");
            return 2;
        }
#else
        cc = "cc";
#endif
        setenv("CC", cc, 0);
    }
    char* bindir = getenv("BINDIR");
    if(!bindir) bindir = "bin";
    setenv("BINDIR", bindir, 0);
    if(!mkdir_if_not_exists(bindir)) return 1;
    if(!mkdir_if_not_exists(temp_sprintf("%s/bikeshed", bindir))) return 1;


    File_Paths dirs = { 0 }, c_sources = { 0 };
    const char* src_dir = "src";
    size_t src_prefix_len = strlen(src_dir)+1;
    if(!walk_directory(&dirs, &c_sources, src_dir)) return 1;
    for(size_t i = 0; i < dirs.count; ++i) {
        if(!mkdir_if_not_exists(temp_sprintf("%s/bikeshed/%s", bindir, dirs.items[i]))) return 1;
    }
    File_Paths objs = { 0 };
    String_Builder stb = { 0 };
    File_Paths pathb = { 0 };
    Cmd cmd = { 0 };
    for(size_t i = 0; i < c_sources.count; ++i) {
        const char* src = c_sources.items[i];
        const char* out = temp_sprintf("%s/bikeshed/%.*s.o", bindir, (int)(strlen(src + src_prefix_len)-2), src + src_prefix_len);
        da_append(&objs, out);
        if(!nob_c_needs_rebuild1(&stb, &pathb, out, src)) continue;
        // C compiler
        cmd_append(&cmd, cc);
        // Warnings
        cmd_append(&cmd,
            "-Wall",
            "-Wextra",
        #if 1
            "-Werror",
        #endif
        );
        // Include directories
        cmd_append(&cmd,
            "-I", "vendor/raylib/raylib-5.5_linux_amd64/include",
            "-I", "include",
        );
        // Actual compilation
        cmd_append(&cmd,
            "-MD", "-O1", "-g", "-c",
            src,
            "-o", out,
        );
        if(!cmd_run_sync_and_reset(&cmd)) return 1;
    }
    const char* exe = "bikeshed";
    if(needs_rebuild(exe, objs.items, objs.count)) {
        cmd_append(&cmd, cc, "-o", exe);
        da_append_many(&cmd, objs.items, objs.count);
        // Vendor libraries we link with
        cmd_append(&cmd,
            "-Lvendor/raylib/raylib-5.5_linux_amd64/lib",
            "-l:libraylib.a"
        );
        cmd_append(&cmd, "-lm");
        if(!cmd_run_sync_and_reset(&cmd)) return 1;
    }
}