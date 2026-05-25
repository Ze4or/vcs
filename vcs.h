#ifndef VCS_H
#define VCS_H

#define VCS_DIR ".vcs"
#define INDEX_FILE ".vcs/index"
#define COMMITS_DIR ".vcs/commits"
#define MAX_PATH 512
#define MAX_MSG 512
#define MAX_HASH 32

typedef enum {
    FILE_ADDED,
    FILE_MODIFIED,
    FILE_REMOVED
} FileStatus;

typedef struct {
    char path[MAX_PATH];
    char hash[MAX_HASH];
    FileStatus status;
} IndexEntry;

typedef struct {
    char hash[MAX_HASH];
    char parent_hash[MAX_HASH];
    char message[MAX_MSG];
    long timestamp;
    int file_count;
} CommitMeta;

int cmd_init(void);
int cmd_add(const char *filename);
int cmd_remove(const char *filename);
int cmd_commit(const char *message);
int cmd_log(const char *start_commit, int num);
int cmd_status(void);
int cmd_diff(const char *commit_hash);
int cmd_checkout(const char *commit_hash, const char *filename);

#endif