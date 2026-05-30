#include "vcs.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int cmd_init(void) {
    if (dir_exists(VCS_DIR)) {
        printf("Репозиторий VCS уже инициализирован в текущей директории.\n");
        return 1;
    }

    if (create_dir(VCS_DIR) != 0 || create_dir(COMMITS_DIR) != 0) {
        fprintf(stderr, "Ошибка: Не удалось создать директории репозитория.\n");
        return 1;
    }

    FILE *f = fopen(INDEX_FILE, "wb");
    if (f) fclose(f);

    CommitMeta root;
    strcpy(root.hash, "00000000");
    strcpy(root.parent_hash, "-");
    strcpy(root.message, "Initial commit");
    root.timestamp = (long)time(NULL);
    root.file_count = 0;

    char commit_dir[MAX_PATH];
    sprintf(commit_dir, "%s/%s", COMMITS_DIR, root.hash);
    create_dir(commit_dir);

    char meta_path[MAX_PATH * 2];
    snprintf(meta_path, sizeof(meta_path), "%s/meta.vcs", commit_dir);
    f = fopen(meta_path, "wb");
    if (f) {
        fwrite(&root, sizeof(CommitMeta), 1, f);
        fclose(f);
    }

    set_head_commit(root.hash);
    printf("Инициализирован пустой репозиторий VCS. Создан начальный коммит 00000000.\n");
    return 0;
}

int cmd_add(const char *filename) {
    if (!dir_exists(VCS_DIR)) {
        fprintf(stderr, "Ошибка: Репозиторий не инициализирован. Выполните './vcs init'.\n");
        return 1;
    }

    if (filename[0] == '/' || strstr(filename, "../") == filename || strstr(filename, "..\\") == filename) {
        fprintf(stderr, "Ошибка: Нельзя добавить внешний файл '%s'. Файл должен находиться внутри репозитория.\n", filename);
        return 1;
    }

    if (!file_exists(filename)) {
        fprintf(stderr, "Ошибка: Файл '%s' не найден.\n", filename);
        return 1;
    }

    char current_hash[MAX_HASH];
    hash_file(filename, current_hash);

    FILE *f = fopen(INDEX_FILE, "r+b");
    if (!f) f = fopen(INDEX_FILE, "wb");
    
    IndexEntry entry;
    bool found = false;
    long pos = 0;

    while (fread(&entry, sizeof(IndexEntry), 1, f)) {
        if (strcmp(entry.path, filename) == 0) {
            found = true;
            pos = ftell(f) - sizeof(IndexEntry);
            break;
        }
    }

    strcpy(entry.path, filename);
    strcpy(entry.hash, current_hash);
    entry.status = FILE_ADDED;

    if (found) {
        fseek(f, pos, SEEK_SET);
    } else {
        fseek(f, 0, SEEK_END);
    }

    fwrite(&entry, sizeof(IndexEntry), 1, f);
    fclose(f);

    printf("Файл '%s' добавлен в индекс для следующего коммита.\n", filename);
    return 0;
}

int cmd_remove(const char *filename) {
    if (!dir_exists(VCS_DIR)) {
        fprintf(stderr, "Ошибка: Репозиторий не инициализирован.\n");
        return 1;
    }

    if (filename[0] == '/' || strstr(filename, "../") == filename || strstr(filename, "..\\") == filename) {
        fprintf(stderr, "Ошибка: Путь '%s' ведёт за пределы репозитория.\n", filename);
        return 1;
    }
    
    const char *clean_filename = normalize_path(filename);

    bool exists_on_disk = file_exists(filename);
    bool existed_in_head = false;
    char head_hash[MAX_HASH];
    get_head_commit(head_hash);

    if (strcmp(head_hash, "00000000") != 0 && strcmp(head_hash, "") != 0 && strcmp(head_hash, "-") != 0) {
        char parent_man[MAX_PATH];
        sprintf(parent_man, "%s/%s/manifest.vcs", COMMITS_DIR, head_hash);
        FILE *f_pman = fopen(parent_man, "rb");
        if (f_pman) {
            IndexEntry p_entry;
            while (fread(&p_entry, sizeof(IndexEntry), 1, f_pman)) {
                if (strcmp(p_entry.path, clean_filename) == 0) {
                    existed_in_head = true;
                    break;
                }
            }
            fclose(f_pman);
        }
    }

    if (!exists_on_disk && !existed_in_head) {
        fprintf(stderr, "Ошибка: Файл '%s' не найден на диске и не отслеживается репозиторием.\n", clean_filename);
        return 1;
    }

    FILE *f = fopen(INDEX_FILE, "rb");
    if (!f) f = fopen(INDEX_FILE, "wb");
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    int count = size / sizeof(IndexEntry);
    fseek(f, 0, SEEK_SET);

    IndexEntry *entries = malloc(size + sizeof(IndexEntry));
    if (count > 0 && entries) {
        fread(entries, sizeof(IndexEntry), count, f);
    }
    fclose(f);

    if (!existed_in_head) {
        FILE *f_new = fopen(INDEX_FILE, "wb");
        if (f_new) {
            for (int i = 0; i < count; i++) {
                if (strcmp(entries[i].path, clean_filename) != 0) {
                    fwrite(&entries[i], sizeof(IndexEntry), 1, f_new);
                }
            }
            fclose(f_new);
        }
        free(entries);
        printf("Файл '%s' убран из стейджинга.\n", clean_filename);
        return 0;
    }

    FILE *f_new = fopen(INDEX_FILE, "wb");
    if (f_new) {
        bool found = false;
        for (int i = 0; i < count; i++) {
            if (strcmp(entries[i].path, clean_filename) == 0) {
                strcpy(entries[i].hash, "00000000");
                entries[i].status = FILE_REMOVED;
                found = true;
            }
            fwrite(&entries[i], sizeof(IndexEntry), 1, f_new);
        }
        if (!found) {
            IndexEntry rem_entry;
            strcpy(rem_entry.path, clean_filename);
            strcpy(rem_entry.hash, "00000000");
            rem_entry.status = FILE_REMOVED;
            fwrite(&rem_entry, sizeof(IndexEntry), 1, f_new);
        }
        fclose(f_new);
    }

    free(entries);
    printf("Файл '%s' помечен на удаление в следующем коммите.\n", clean_filename);
    return 0;
}

int cmd_commit(const char *message) {
    if (!dir_exists(VCS_DIR)) return 1;

    FILE *f_idx = fopen(INDEX_FILE, "rb");
    if (!f_idx) return 1;

    fseek(f_idx, 0, SEEK_END);
    long size = ftell(f_idx);
    int index_count = size / sizeof(IndexEntry);
    if (index_count == 0) {
        printf("Ничего не изменено. Стейджинг пуст.\n");
        fclose(f_idx);
        return 1;
    }
    fseek(f_idx, 0, SEEK_SET);

    IndexEntry *index_entries = malloc(size);
    fread(index_entries, sizeof(IndexEntry), index_count, f_idx);
    fclose(f_idx);

    char head_hash[MAX_HASH];
    get_head_commit(head_hash);

    CommitMeta new_commit;
    strcpy(new_commit.parent_hash, head_hash);
    strcpy(new_commit.message, message);
    new_commit.timestamp = (long)time(NULL);
    
    unsigned long h = 5381;
    for(int i = 0; message[i]; i++) h = ((h << 5) + h) + message[i];
    h += new_commit.timestamp;
    sprintf(new_commit.hash, "%08lx", h);

    char commit_dir[MAX_PATH];
    sprintf(commit_dir, "%s/%s", COMMITS_DIR, new_commit.hash);
    create_dir(commit_dir);

    char manifest_path[MAX_PATH * 2];
    snprintf(manifest_path, sizeof(manifest_path), "%s/manifest.vcs", commit_dir);
    FILE *f_man = fopen(manifest_path, "wb");

    if (strcmp(head_hash, "00000000") != 0 && strcmp(head_hash, "-") != 0) {
        char parent_man[MAX_PATH];
        sprintf(parent_man, "%s/%s/manifest.vcs", COMMITS_DIR, head_hash);
        FILE *f_pman = fopen(parent_man, "rb");
        if (f_pman) {
            IndexEntry p_entry;
            while (fread(&p_entry, sizeof(IndexEntry), 1, f_pman)) {
                bool overridden = false;
                for (int i = 0; i < index_count; i++) {
                    if (strcmp(index_entries[i].path, p_entry.path) == 0) {
                        overridden = true;
                        break;
                    }
                }
                if (!overridden) {
                    fwrite(&p_entry, sizeof(IndexEntry), 1, f_man);
                }
            }
            fclose(f_pman);
        }
    }

    for (int i = 0; i < index_count; i++) {
        if (index_entries[i].status != FILE_REMOVED) {
            char store_path[MAX_PATH * 2];
            snprintf(store_path, sizeof(store_path), "%s/%s", commit_dir, index_entries[i].path);
            
            copy_file(index_entries[i].path, store_path);

            IndexEntry commit_file_rec;
            strcpy(commit_file_rec.path, index_entries[i].path);
            strcpy(commit_file_rec.hash, index_entries[i].hash);
            commit_file_rec.status = FILE_ADDED;
            fwrite(&commit_file_rec, sizeof(IndexEntry), 1, f_man);
        }
    }
    fclose(f_man);

    char meta_path[MAX_PATH * 2];
    snprintf(meta_path, sizeof(meta_path), "%s/meta.vcs", commit_dir);
    FILE *f_meta = fopen(meta_path, "wb");
    if (f_meta) {
        fwrite(&new_commit, sizeof(CommitMeta), 1, f_meta);
        fclose(f_meta);
    }

    FILE *f_clr = fopen(INDEX_FILE, "wb");
    if (f_clr) fclose(f_clr);

    set_head_commit(new_commit.hash);
    free(index_entries);

    printf("Коммит %s успешно создан.\nMessage: %s\n", new_commit.hash, message);
    return 0;
}

int cmd_log(const char *start_commit, int num) {
    if (!dir_exists(VCS_DIR)) return 1;

    char current_hash[MAX_HASH];
    if (start_commit && strlen(start_commit) > 0) {
        strcpy(current_hash, start_commit);
    } else {
        get_head_commit(current_hash);
    }

    int count = 0;
    while (strcmp(current_hash, "-") != 0 && strcmp(current_hash, "") != 0) {
        if (num > 0 && count >= num) break;

        char meta_path[MAX_PATH];
        sprintf(meta_path, "%s/%s/meta.vcs", COMMITS_DIR, current_hash);
        
        FILE *f = fopen(meta_path, "rb");
        if (!f) {
            fprintf(stderr, "Коммит %s не найден в истории.\n", current_hash);
            return 1;
        }

        CommitMeta meta;
        fread(&meta, sizeof(CommitMeta), 1, f);
        fclose(f);

        struct tm *tm_info = localtime(&meta.timestamp);
        char time_buf[26];
        strftime(time_buf, 26, "%Y-%m-%d %H:%M:%S", tm_info);

        printf("----------------------------------------\n");
        printf("Commit: %s\n", meta.hash);
        printf("Parent: %s\n", meta.parent_hash);
        printf("Date:   %s\n", time_buf);
        printf("Message: %s\n", meta.message);

        strcpy(current_hash, meta.parent_hash);
        count++;
    }
    printf("----------------------------------------\n");
    return 0;
}

int cmd_status(void) {
    if (!dir_exists(VCS_DIR)) {
        fprintf(stderr, "Ошибка: Репозиторий не инициализирован. Выполните './vcs init'.\n");
        return 1;
    }

    FILE *f = fopen(INDEX_FILE, "rb");
    if (!f) {
        printf("Изменения для коммита отсутствуют (стейджинг пуст).\n");
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    int count = size / sizeof(IndexEntry);
    fseek(f, 0, SEEK_SET);

    if (count == 0) {
        printf("Изменения для коммита отсутствуют (стейджинг пуст).\n");
        fclose(f);
        return 0;
    }

    char head_hash[MAX_HASH];
    get_head_commit(head_hash);

    printf("Изменения, которые будут включены в следующий коммит:\n");
    
    IndexEntry entry;
    while (fread(&entry, sizeof(IndexEntry), 1, f)) {
        printf("  ");
        if (entry.status == FILE_REMOVED) {
            printf("удален:           %s", entry.path);
        } else {
            bool existed_before = false;
            if (strcmp(head_hash, "00000000") != 0 && strcmp(head_hash, "") != 0 && strcmp(head_hash, "-") != 0) {
                char parent_man[MAX_PATH];
                snprintf(parent_man, sizeof(parent_man), "%s/%s/manifest.vcs", COMMITS_DIR, head_hash);
                FILE *f_pman = fopen(parent_man, "rb");
                if (f_pman) {
                    IndexEntry p_entry;
                    while (fread(&p_entry, sizeof(IndexEntry), 1, f_pman)) {
                        if (strcmp(p_entry.path, entry.path) == 0) {
                            existed_before = true;
                            break;
                        }
                    }
                    fclose(f_pman);
                }
            }

            if (existed_before) {
                printf("изменен:          %s", entry.path);
            } else {
                printf("создан:           %s", entry.path);
            }
        }
        printf("\n");
    }
    fclose(f);
    return 0;
}


int cmd_diff(const char *commit_hash) {
    if (!dir_exists(VCS_DIR)) return 1;

    char head_hash[MAX_HASH];
    get_head_commit(head_hash);

    char path_target[MAX_PATH];
    char path_head[MAX_PATH];
    sprintf(path_target, "%s/%s/manifest.vcs", COMMITS_DIR, commit_hash);
    sprintf(path_head, "%s/%s/manifest.vcs", COMMITS_DIR, head_hash);

    FILE *f_tar = fopen(path_target, "rb");
    if (!f_tar) {
        fprintf(stderr, "Ошибка: Указанный коммит '%s' не найден.\n", commit_hash);
        return 1;
    }

    printf("Сравнение коммита %s с текущим %s:\n", commit_hash, head_hash);
    printf("Файлы в целевом коммите (%s):\n", commit_hash);
    IndexEntry e;
    while(fread(&e, sizeof(IndexEntry), 1, f_tar)) {
        printf("  %s [Hash: %s]\n", e.path, e.hash);
    }
    fclose(f_tar);

    FILE *f_head = fopen(path_head, "rb");
    if (f_head) {
        printf("Файлы в текущем коммите (%s):\n", head_hash);
        while(fread(&e, sizeof(IndexEntry), 1, f_head)) {
            printf("  %s [Hash: %s]\n", e.path, e.hash);
        }
        fclose(f_head);
    }
    return 0;
}

int cmd_checkout(const char *commit_hash, const char *filename) {
    if (!dir_exists(VCS_DIR)) return 1;

    char manifest_path[MAX_PATH];
    sprintf(manifest_path, "%s/%s/manifest.vcs", COMMITS_DIR, commit_hash);
    FILE *f = fopen(manifest_path, "rb");
    if (!f) {
        fprintf(stderr, "Ошибка: Коммит %s или файл в нем не найден.\n", commit_hash);
        return 1;
    }

    IndexEntry entry;
    bool found = false;
    while (fread(&entry, sizeof(IndexEntry), 1, f)) {
        if (strcmp(entry.path, filename) == 0) {
            found = true;
            break;
        }
    }
    fclose(f);

    if (!found) {
        fprintf(stderr, "Ошибка: Файл '%s' не найден в коммите %s.\n", filename, commit_hash);
        return 1;
    }

    char source_file[MAX_PATH];
    sprintf(source_file, "%s/%s/%s", COMMITS_DIR, commit_hash, filename);

    create_parent_dirs(filename);
    copy_file(source_file, filename);
    printf("Файл '%s' успешно восстановлен из коммита %s.\n", filename, commit_hash);
    return 0;
}