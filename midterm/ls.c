#define _DEFAULT_SOURCE
#include "ls.h"

#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_errors = 0;

void options_init(options_t *opts)
{
    memset(opts, 0, sizeof(*opts));
    opts->time_mode = TIME_MTIME;
    opts->size_mode = SIZE_BYTES;
    if (isatty(STDOUT_FILENO))
        opts->quote = 1;
    else
        opts->raw = 1;
}

void usage(const char *prog)
{
    fprintf(stderr,
        "usage: %s [-AacdFfhiklnqRrSstuw] [file ...]\n", prog);
}

int parse_options(int argc, char **argv, options_t *opts, int *first_operand)
{
    int c;

    opterr = 0;
    while ((c = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1) {
        switch (c) {
        case 'A': opts->almost_all = 1; break;
        case 'a': opts->all = 1; break;
        case 'c': opts->time_mode = TIME_CTIME; break;
        case 'd': opts->directory = 1; break;
        case 'F': opts->classify = 1; break;
        case 'f': opts->no_sort = 1; break;
        case 'h': opts->human = 1; opts->kilobytes = 0; break;
        case 'i': opts->inode = 1; break;
        case 'k': opts->kilobytes = 1; opts->human = 0; break;
        case 'l': opts->long_format = 1; break;
        case 'n': opts->numeric = 1; opts->long_format = 1; break;
        case 'q': opts->quote = 1; opts->raw = 0; break;
        case 'R': opts->recursive = 1; break;
        case 'r': opts->reverse = 1; break;
        case 'S': opts->sort_size = 1; opts->sort_time = 0; break;
        case 's': opts->blocks = 1; break;
        case 't': opts->sort_time = 1; opts->sort_size = 0; break;
        case 'u': opts->time_mode = TIME_ATIME; break;
        case 'w': opts->raw = 1; opts->quote = 0; break;
        case '?':
            fprintf(stderr, "%s: illegal option -- %c\n", argv[0], optopt);
            usage(argv[0]);
            return -1;
        default:
            usage(argv[0]);
            return -1;
        }
    }

    if (opts->human)
        opts->size_mode = SIZE_HUMAN;
    else if (opts->kilobytes)
        opts->size_mode = SIZE_KILOBYTES;
    else
        opts->size_mode = SIZE_BYTES;

    *first_operand = optind;
    return 0;
}

void entry_list_init(entry_list_t *list)
{
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void entry_list_free(entry_list_t *list)
{
    size_t i;
    for (i = 0; i < list->count; i++) {
        free(list->items[i].name);
        free(list->items[i].path);
    }
    free(list->items);
    entry_list_init(list);
}

int entry_list_add(entry_list_t *list, const char *name, const char *path,
    const struct stat *st)
{
    entry_t *tmp;

    if (list->count == list->capacity) {
        size_t newcap = list->capacity ? list->capacity * 2 : 32;
        tmp = realloc(list->items, newcap * sizeof(*tmp));
        if (!tmp)
            return -1;
        list->items = tmp;
        list->capacity = newcap;
    }

    list->items[list->count].name = strdup(name);
    list->items[list->count].path = strdup(path);
    if (!list->items[list->count].name || !list->items[list->count].path) {
        free(list->items[list->count].name);
        free(list->items[list->count].path);
        return -1;
    }
    list->items[list->count].st = *st;
    list->count++;
    return 0;
}

void list_directory(const char *path, const options_t *opts, int show_header)
{
    DIR *dir;
    struct dirent *de;
    entry_list_t list;
    char child[PATH_MAX];
    int first = 1;
    size_t i;

    dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
        g_errors = 1;
        return;
    }

    entry_list_init(&list);
    while ((de = readdir(dir)) != NULL) {
        struct stat st;
        int n;

        if (!should_show_name(de->d_name, opts))
            continue;

        n = snprintf(child, sizeof(child), "%s/%s", path, de->d_name);
        if (n < 0 || (size_t)n >= sizeof(child)) {
            fprintf(stderr, "ls: path too long: %s/%s\n", path, de->d_name);
            g_errors = 1;
            continue;
        }

        if (lstat(child, &st) == -1) {
            fprintf(stderr, "ls: %s: %s\n", child, strerror(errno));
            g_errors = 1;
            continue;
        }

        if (entry_list_add(&list, de->d_name, child, &st) == -1) {
            fprintf(stderr, "ls: out of memory\n");
            g_errors = 1;
            closedir(dir);
            entry_list_free(&list);
            return;
        }
    }
    closedir(dir);

    sort_entries(&list, opts);

    if (show_header)
        printf("%s:\n", path);
    print_entries(list.items, list.count, opts, path);

    if (opts->recursive) {
        for (i = 0; i < list.count; i++) {
            struct stat st = list.items[i].st;
            if (!S_ISDIR(st.st_mode))
                continue;
            if (strcmp(list.items[i].name, ".") == 0 ||
                strcmp(list.items[i].name, "..") == 0)
                continue;

            if (!first || show_header)
                putchar('\n');
            first = 0;
            list_directory(list.items[i].path, opts, 1);
        }
    }

    entry_list_free(&list);
}

void list_operand(const char *path, const options_t *opts, int show_header)
{
    struct stat st;
    entry_t e;

    if ((opts->directory ? lstat(path, &st) : stat(path, &st)) == -1) {
        fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
        g_errors = 1;
        return;
    }

    if (S_ISDIR(st.st_mode) && !opts->directory) {
        list_directory(path, opts, show_header);
        return;
    }

    e.name = (char *)path;
    e.path = (char *)path;
    e.st = st;

    if (opts->long_format)
        print_one_long(&e, opts);
    else {
        if (opts->inode)
            printf("%llu ", (unsigned long long)e.st.st_ino);
        if (opts->blocks) {
            if (opts->human) {
                char bbuf[64];
                format_size(e.st.st_blocks * 512, SIZE_HUMAN,
                    bbuf, sizeof(bbuf));
                printf("%s ", bbuf);
            } else if (opts->kilobytes) {
                printf("%lld ", ((long long)e.st.st_blocks + 1) / 2);
            } else {
                printf("%lld ", (long long)e.st.st_blocks);
            }
        }
        print_name(&e, opts);
        putchar('\n');
    }
}

int main(int argc, char **argv)
{
    options_t opts;
    int first_operand;
    int i;
    int operands;
    entry_list_t files;
    entry_list_t dirs;

    options_init(&opts);
    if (parse_options(argc, argv, &opts, &first_operand) == -1)
        return 1;

    operands = argc - first_operand;
    if (operands == 0) {
        list_directory(".", &opts, 0);
        return g_errors ? 1 : 0;
    }

    entry_list_init(&files);
    entry_list_init(&dirs);

    /* The manual requires non-directories first, with the two classes
     * sorted separately and lexicographically. */
    for (i = first_operand; i < argc; i++) {
        struct stat st;
        if ((opts.directory ? lstat(argv[i], &st) : stat(argv[i], &st)) == -1) {
            fprintf(stderr, "ls: %s: %s\n", argv[i], strerror(errno));
            g_errors = 1;
            continue;
        }
        if (S_ISDIR(st.st_mode) && !opts.directory) {
            if (entry_list_add(&dirs, argv[i], argv[i], &st) == -1) {
                fprintf(stderr, "ls: out of memory\n");
                g_errors = 1;
            }
        } else {
            if (entry_list_add(&files, argv[i], argv[i], &st) == -1) {
                fprintf(stderr, "ls: out of memory\n");
                g_errors = 1;
            }
        }
    }

    sort_entries_lexical(&files, opts.reverse);
    sort_entries_lexical(&dirs, opts.reverse);

    for (i = 0; (size_t)i < files.count; i++)
        list_operand(files.items[i].path, &opts, operands > 1);

    for (i = 0; (size_t)i < dirs.count; i++)
        list_operand(dirs.items[i].path, &opts, operands > 1);

    entry_list_free(&files);
    entry_list_free(&dirs);
    return g_errors ? 1 : 0;
}
