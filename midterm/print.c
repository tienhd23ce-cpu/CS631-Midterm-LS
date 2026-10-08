#include "ls.h"

#include <grp.h>
#include <pwd.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

static void print_quoted(const char *s, int quote, int raw)
{
    const unsigned char *p = (const unsigned char *)s;

    if (raw) {
        fputs(s, stdout);
        return;
    }

    while (*p) {
        if (quote && (*p < 32 || *p == 127))
            putchar('?');
        else
            putchar(*p);
        p++;
    }
}

int should_show_name(const char *name, const options_t *opts)
{
    if (name[0] != '.')
        return 1;
    if (opts->all)
        return 1;
    if (opts->almost_all && strcmp(name, ".") != 0 && strcmp(name, "..") != 0)
        return 1;
    return 0;
}

char classify_char(const struct stat *st)
{
    if (S_ISDIR(st->st_mode)) return '/';
    if (S_ISLNK(st->st_mode)) return '@';
    if (S_ISFIFO(st->st_mode)) return '|';
    if (S_ISSOCK(st->st_mode)) return '=';
    if (S_ISREG(st->st_mode) && (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
        return '*';
    return '\0';
}

void format_size(off_t size, size_mode_t mode, char *buf, size_t buflen)
{
    if (mode == SIZE_BYTES) {
        snprintf(buf, buflen, "%lld", (long long)size);
        return;
    }

    if (mode == SIZE_KILOBYTES) {
        long long kb = ((long long)size + 1023) / 1024;
        snprintf(buf, buflen, "%lld", kb);
        return;
    }

    {
        const char *units = "BKMGTPE";
        double value = (double)size;
        int unit = 0;
        while (value >= 1024.0 && unit < 6) {
            value /= 1024.0;
            unit++;
        }
        if (unit == 0)
            snprintf(buf, buflen, "%lldB", (long long)size);
        else if (value >= 10.0)
            snprintf(buf, buflen, "%.0f%c", value, units[unit]);
        else
            snprintf(buf, buflen, "%.1f%c", value, units[unit]);
    }
}

void format_time_value(const struct stat *st, const options_t *opts,
    char *buf, size_t buflen)
{
    time_t t;
    struct tm tmv;

    if (opts->time_mode == TIME_CTIME)
        t = st->st_ctime;
    else if (opts->time_mode == TIME_ATIME)
        t = st->st_atime;
    else
        t = st->st_mtime;

    if (localtime_r(&t, &tmv) == NULL) {
        snprintf(buf, buflen, "??? ?? ??:??");
        return;
    }
    strftime(buf, buflen, "%b %e %H:%M", &tmv);
}

static void mode_string(mode_t mode, char *buf)
{
    buf[0] = S_ISDIR(mode) ? 'd' :
        S_ISLNK(mode) ? 'l' :
        S_ISCHR(mode) ? 'c' :
        S_ISBLK(mode) ? 'b' :
        S_ISFIFO(mode) ? 'p' :
        S_ISSOCK(mode) ? 's' : '-';

    buf[1] = (mode & S_IRUSR) ? 'r' : '-';
    buf[2] = (mode & S_IWUSR) ? 'w' : '-';
    buf[3] = (mode & S_IXUSR) ? 'x' : '-';
    buf[4] = (mode & S_IRGRP) ? 'r' : '-';
    buf[5] = (mode & S_IWGRP) ? 'w' : '-';
    buf[6] = (mode & S_IXGRP) ? 'x' : '-';
    buf[7] = (mode & S_IROTH) ? 'r' : '-';
    buf[8] = (mode & S_IWOTH) ? 'w' : '-';
    buf[9] = (mode & S_IXOTH) ? 'x' : '-';

    if (mode & S_ISUID) buf[3] = (mode & S_IXUSR) ? 's' : 'S';
    if (mode & S_ISGID) buf[6] = (mode & S_IXGRP) ? 's' : 'S';
    if (mode & S_ISVTX) buf[9] = (mode & S_IXOTH) ? 't' : 'T';
    buf[10] = '\0';
}

static void owner_name(uid_t uid, gid_t gid, const options_t *opts,
    char *owner, size_t owner_len, char *group, size_t group_len)
{
    struct passwd *pw;
    struct group *gr;

    if (opts->numeric) {
        snprintf(owner, owner_len, "%u", (unsigned)uid);
        snprintf(group, group_len, "%u", (unsigned)gid);
        return;
    }

    pw = getpwuid(uid);
    if (pw)
        snprintf(owner, owner_len, "%s", pw->pw_name);
    else
        snprintf(owner, owner_len, "%u", (unsigned)uid);

    gr = getgrgid(gid);
    if (gr)
        snprintf(group, group_len, "%s", gr->gr_name);
    else
        snprintf(group, group_len, "%u", (unsigned)gid);
}

void print_name(const entry_t *e, const options_t *opts)
{
    print_quoted(e->name, opts->quote, opts->raw);
    if (opts->classify) {
        char c = classify_char(&e->st);
        if (c)
            putchar(c);
    }
}

void print_one_long(const entry_t *e, const options_t *opts)
{
    char mode[11];
    char owner[128];
    char group[128];
    char sizebuf[64];
    char timebuf[64];
    char linkbuf[LS_NAME_MAX];
    char suffix = '\0';

    mode_string(e->st.st_mode, mode);
    owner_name(e->st.st_uid, e->st.st_gid, opts,
        owner, sizeof(owner), group, sizeof(group));

    format_size(e->st.st_size,
        opts->human ? SIZE_HUMAN : SIZE_BYTES,
        sizebuf, sizeof(sizebuf));
    format_time_value(&e->st, opts, timebuf, sizeof(timebuf));

    if (opts->blocks) {
        long long blocks = ((long long)e->st.st_blocks);
        if (opts->human) {
            char bbuf[64];
            format_size(blocks * 512, SIZE_HUMAN, bbuf, sizeof(bbuf));
            printf("%s ", bbuf);
        } else if (opts->kilobytes) {
            printf("%lld ", (blocks + 1) / 2);
        } else {
            printf("%lld ", blocks);
        }
    }

    if (opts->inode)
        printf("%llu ", (unsigned long long)e->st.st_ino);

    printf("%s %3lu %-8s %-8s %8s %s ",
        mode, (unsigned long)e->st.st_nlink, owner, group,
        sizebuf, timebuf);

    print_name(e, opts);

    if (S_ISLNK(e->st.st_mode)) {
        ssize_t n = readlink(e->path, linkbuf, sizeof(linkbuf) - 1);
        if (n >= 0) {
            linkbuf[n] = '\0';
            printf(" -> ");
            print_quoted(linkbuf, opts->quote, opts->raw);
        }
    }
    suffix = classify_char(&e->st);
    (void)suffix;
    putchar('\n');
}

void print_entries(const entry_t *items, size_t count, const options_t *opts,
    const char *display_dir)
{
    size_t i;
    long long total = 0;
    (void)display_dir;

    if (opts->long_format || opts->blocks) {
        for (i = 0; i < count; i++)
            total += (long long)items[i].st.st_blocks;
        if (opts->human)
            printf("total %lldK\n", (total + 1) / 2);
        else
            printf("total %lld\n", total);
    }

    for (i = 0; i < count; i++) {
        if (opts->long_format) {
            print_one_long(&items[i], opts);
        } else {
            if (opts->inode)
                printf("%llu ", (unsigned long long)items[i].st.st_ino);
            if (opts->blocks) {
                if (opts->human) {
                    char bbuf[64];
                    format_size(items[i].st.st_blocks * 512,
                        SIZE_HUMAN, bbuf, sizeof(bbuf));
                    printf("%s ", bbuf);
                } else if (opts->kilobytes) {
                    printf("%lld ", ((long long)items[i].st.st_blocks + 1) / 2);
                } else {
                    printf("%lld ", (long long)items[i].st.st_blocks);
                }
            }
            print_name(&items[i], opts);
            putchar('\n');
        }
    }
}
