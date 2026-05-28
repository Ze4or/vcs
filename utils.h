#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

void hash_file(const char *filepath, char *output_hash);
bool file_exists(const char *path);
bool dir_exists(const char *path);
int create_dir(const char *path);
void create_parent_dirs(const char *file_path);
void copy_file(const char *src, const char *dst);
void get_head_commit(char *output_hash);
void set_head_commit(const char *hash);
const char* normalize_path(const char *path);

#endif