#include "utils.h"
#include "vcs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#endif

void hash_file(const char *filepath, char *output_hash) {
    FILE *f = fopen(filepath, "rb");
    unsigned long hash = 5381;
    int c;

    if (!f) {
        sprintf(output_hash, "00000000");
        return;
    }

    while ((c = fgetc(f)) != EOF) {
        hash = ((hash << 5) + hash) + c;
    }
    fclose(f);
    sprintf(output_hash, "%08lx", hash);
}

bool file_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
}

bool dir_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

int create_dir(const char *path) {
    return mkdir(path, 0777);
}

void create_parent_dirs(const char *file_path) {
    char temp[MAX_PATH];
    snprintf(temp, sizeof(temp), "%s", file_path);
    
    for (int i = 0; temp[i] != '\0'; i++) {
        if (temp[i] == '/') {
            temp[i] = '\0';
            if (strlen(temp) > 0 && !dir_exists(temp)) {
                create_dir(temp);
            }
            temp[i] = '/';
        }
    }
}

void copy_file(const char *src, const char *dst) {
    create_parent_dirs(dst);
    FILE *s = fopen(src, "rb");
    if (!s) return;
    FILE *d = fopen(dst, "wb");
    if (!d) {
        fclose(s);
        return;
    }
    char buf[BUFSIZ];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), s)) > 0) {
        fwrite(buf, 1, n, d);
    }
    fclose(s);
    fclose(d);
}

void get_head_commit(char *output_hash) {
    FILE *f = fopen(".vcs/HEAD", "r");
    if (!f) {
        strcpy(output_hash, "");
        return;
    }
    if (fscanf(f, "%31s", output_hash) != 1) {
        strcpy(output_hash, "");
    }
    fclose(f);
}

void set_head_commit(const char *hash) {
    FILE *f = fopen(".vcs/HEAD", "w");
    if (f) {
        fprintf(f, "%s", hash);
        fclose(f);
    }
}

const char* normalize_path(const char *path) {
    if (strncmp(path, "./", 2) == 0) {
        return path + 2;
    }
    return path;
}