#include "ls.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int cmp_name(const entry_t *a, const entry_t *b) { return strcmp(a->name, b->name); }
static time_t entry_time(const entry_t *e, time_mode_t mode) {
    if (mode == TIME_CTIME) return e->st.st_ctime;
    if (mode == TIME_ATIME) return e->st.st_atime;
    return e->st.st_mtime;
}
static int compare_entries(const void *va, const void *vb, void *arg) {
    const entry_t *a = va, *b = vb;
    const options_t *opts = arg;
    int result;
    if (opts->sort_size) {
        if (a->st.st_size < b->st.st_size) result = 1;
        else if (a->st.st_size > b->st.st_size) result = -1;
        else result = cmp_name(a, b);
    } else if (opts->sort_time) {
        time_t ta = entry_time(a, opts->time_mode), tb = entry_time(b, opts->time_mode);
        if (ta < tb) result = 1;
        else if (ta > tb) result = -1;
        else result = cmp_name(a, b);
    } else result = cmp_name(a, b);
    return opts->reverse ? -result : result;
}
static const options_t *global_opts;
static int compare_entries_fallback(const void *a, const void *b) {
    return compare_entries(a, b, (void *)global_opts);
}
void sort_entries(entry_list_t *list, const options_t *opts) {
    if (!opts->no_sort && list->count > 1) {
        global_opts = opts;
        qsort(list->items, list->count, sizeof(list->items[0]), compare_entries_fallback);
    }
}
static int compare_lexical_only(const void *a, const void *b) {
    const entry_t *ea = a, *eb = b;
    return strcmp(ea->name, eb->name);
}
void sort_entries_lexical(entry_list_t *list, int reverse) {
    size_t i;
    if (list->count > 1)
        qsort(list->items, list->count, sizeof(list->items[0]), compare_lexical_only);
    if (reverse) {
        for (i = 0; i < list->count / 2; i++) {
            entry_t tmp = list->items[i];
            list->items[i] = list->items[list->count - 1 - i];
            list->items[list->count - 1 - i] = tmp;
        }
    }
}
