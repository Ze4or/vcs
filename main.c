#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vcs.h"

void print_help(void) {
    printf("Упрощенная система контроля версий (VCS)\n\n");
    printf("Использование:\n");
    printf("  ./vcs <команда> [аргументы]\n\n");
    printf("Команды:\n");
    printf("  init                          Инициализировать новый репозиторий\n");
    printf("  add <имя_файла>               Добавить файл в следующий коммит\n");
    printf("  remove <имя_файла>            Пометить файл на удаление\n");
    printf("  commit \"<сообщение>\"          Создать новый коммит\n");
    printf("  status                        Показать состояние файлов на стейджинге\n");
    printf("  log [--n <число>] [хеш]       Показать историю коммитов\n");
    printf("  diff <хеш_коммита>            Сравнить коммит с текущим\n");
    printf("  checkout <хеш> <имя_файла>    Восстановить файл из коммита\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Ошибка: Не указана команда.\n");
        fprintf(stderr, "Запустите './vcs --help' для вызова справки.\n");
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "help") == 0) {
        print_help();
        return 0;
    }

    if (strcmp(argv[1], "init") == 0) {
        return cmd_init();
    } 
    else if (strcmp(argv[1], "add") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Ошибка: Команда 'add' требует имя файла.\nПример: ./vcs add main.c\n");
            return 1;
        }
        return cmd_add(argv[2]);
    } 
    else if (strcmp(argv[1], "remove") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Ошибка: Команда 'remove' требует имя файла.\nПример: ./vcs remove main.c\n");
            return 1;
        }
        return cmd_remove(argv[2]);
    } 
    else if (strcmp(argv[1], "commit") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Ошибка: Команда 'commit' требует сообщение.\nПример: ./vcs commit \"Fix bugs\"\n");
            return 1;
        }
        return cmd_commit(argv[2]);
    } 
    else if (strcmp(argv[1], "status") == 0) {
        return cmd_status();
    } 
    else if (strcmp(argv[1], "log") == 0) {
        int num = -1;
        const char *start_commit = NULL;
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--n") == 0 && i + 1 < argc) {
                num = atoi(argv[i + 1]);
                i++;
            } else {
                start_commit = argv[i];
            }
        }
        return cmd_log(start_commit, num);
    } 
    else if (strcmp(argv[1], "diff") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Ошибка: Команда 'diff' требует хэш коммита.\nПример: ./vcs diff 5381a4b2\n");
            return 1;
        }
        return cmd_diff(argv[2]);
    } 
    else if (strcmp(argv[1], "checkout") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Ошибка: Недостаточно аргументов.\nПример: ./vcs checkout <хеш> <имя_файла>\n");
            return 1;
        }
        return cmd_checkout(argv[2], argv[3]);
    } 
    else {
        fprintf(stderr, "Ошибка: Неизвестная команда '%s'.\n", argv[1]);
        fprintf(stderr, "Запустите './vcs --help' для вызова справки.\n");
        return 1;
    }

    return 0;
}