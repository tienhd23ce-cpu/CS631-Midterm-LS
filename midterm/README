# CS631 Midterm - ls(1)

Student: Hoang Duc Tien
Student ID: 23CE078

## Part I - grep filtering scenario

A sample filtering scenario required by the 3-point section is to inspect a log file and select error lines:

```sh
printf '%s\n' 'INFO login ok' 'ERROR disk full' 'INFO backup ok' 'ERROR timeout' > activity.log
grep ERROR activity.log
```

This demonstrates `grep` selecting lines containing a requested text pattern. A pipe can be demonstrated with:

```sh
ls | grep ".c"
```

The pipe sends the output of `ls` to `grep`, which filters the names containing `.c`.

## Assignment

This project implements the `ls(1)` command according to the supplied NetBSD `ls(1)` manual page for the CS631 Midterm. The implementation intentionally follows the option set specified by the assignment rather than attempting to reproduce every GNU/Linux `ls` extension.

Supported options:

`-A -a -c -d -F -f -h -i -k -l -n -q -R -r -S -s -t -u -w`

## Source organization

- `ls.c`: command-line parsing, operand handling, directory traversal, main control flow.
- `print.c`: long-format output, permissions, owner/group names, sizes, timestamps and file classification.
- `cmp.c`: sorting comparators for name, size and time.
- `ls.h`: shared data structures, option state and function declarations.
- `Makefile`: build and basic test targets.
- `checklist`: assignment checklist and test checklist.
- `LOG`: git history exported for submission.

## Build

```sh
make
```

The executable is named `ls` in the current directory.

## Basic usage

```sh
./ls
./ls -a
./ls -A
./ls -l
./ls -la
./ls -i
./ls -n
./ls -F
./ls -R .
./ls -r
./ls -S
./ls -t
./ls -u
./ls -c
./ls -s
./ls -sk
./ls -lh
./ls -d .
```

## Testing

The implementation should be compared against the supplied manual and, where useful, against the system `ls` for individual behaviors. Because the assignment manual is the authoritative specification, differences caused by GNU/Linux-specific features should not be treated as failures.

Recommended tests:

1. Empty invocation: `./ls`
2. Hidden entries: `./ls -a` and `./ls -A`
3. Long format: `./ls -l`
4. Numeric owner/group: `./ls -n`
5. Inodes: `./ls -i`
6. File classification: `./ls -F`
7. Directory-as-file: `./ls -d .`
8. Recursive traversal: `./ls -R .`
9. Reverse ordering: `./ls -r`
10. Size sorting: `./ls -S`
11. Time sorting: `./ls -t`
12. Access/change time: `./ls -u`, `./ls -c`
13. Block counts: `./ls -s`, `./ls -sk`
14. Human-readable sizes: `./ls -lh`
15. Multiple operands, including both files and directories.
16. Invalid paths and permission errors; verify a non-zero exit status.

## Platform note

The assignment manual is for NetBSD while the development environment may be Ubuntu/Linux. The implementation uses POSIX interfaces available on Linux (`opendir`, `readdir`, `lstat`, `stat`, `getpwuid`, `getgrgid`, etc.). NetBSD-specific filesystem features that have no direct Linux equivalent are not invented or substituted with unrelated GNU `ls` behavior.

In particular, the `-F` whiteout marker described by the NetBSD manual has no ordinary Linux equivalent; the implementation covers the regular file types available through Linux `stat` information.

## Git

Before final submission, update the repository and record the final history:

```sh
git pull
git status
git add .
git commit -m "implement ls midterm"
git push
git log > LOG
```

## Archive

The required final archive format is `<username>-midterm.tar`. For the Stevens shell account, create it from the parent directory according to the assignment's required layout.
