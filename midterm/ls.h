#ifndef LS_H
#define _DEFAULT_SOURCE
#define LS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <stdio.h>

#define LS_NAME_MAX 4096

typedef enum { TIME_MTIME = 0, TIME_CTIME, TIME_ATIME } time_mode_t;
typedef enum { SIZE_BYTES = 0, SIZE_KILOBYTES, SIZE_HUMAN } size_mode_t;

typedef struct {
    int all, almost_all, classify, directory, no_sort, human, inode, kilobytes;
    int long_format, numeric, quote, recursive, reverse, sort_size, blocks;
    int sort_time, raw;
    time_mode_t time_mode;
    size_mode_t size_mode;
} options_t;

typedef struct {
    char *name;
    char *path;
    struct stat st;
} entry_t;

typedef struct {
    entry_t *items;
    size_t count;
    size_t capacity;
} entry_list_t;

void options_init(options_t *opts);
int parse_options(int argc, char **argv, options_t *opts, int *first_operand);
void usage(const char *prog);
void list_operand(const char *path, const options_t *opts, int show_header);
void list_directory(const char *path, const options_t *opts, int show_header);
void entry_list_init(entry_list_t *list);
void entry_list_free(entry_list_t *list);
int entry_list_add(entry_list_t *list, const char *name, const char *path,
    const struct stat *st);
void sort_entries(entry_list_t *list, const options_t *opts);
void sort_entries_lexical(entry_list_t *list, int reverse);
void print_entries(const entry_t *items, size_t count, const options_t *opts,
    const char *display_dir);
void print_one_long(const entry_t *e, const options_t *opts);
void print_name(const entry_t *e, const options_t *opts);
int should_show_name(const char *name, const options_t *opts);
char classify_char(const struct stat *st);
void format_size(off_t size, size_mode_t mode, char *buf, size_t buflen);
void format_time_value(const struct stat *st, const options_t *opts,
    char *buf, size_t buflen);

#endif
