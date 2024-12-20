#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(const char *path, const char *target) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    // open the directory
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // get the status of the directory
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        close(fd);
        return;
    }

    // ensure it is a directory
    if (st.type != T_DIR) {
        fprintf(2, "find: %s is not a directory\n", path);
        close(fd);
        return;
    }

    // read directory entries
    strcpy(buf, path);
    p = buf+strlen(buf); // point p to the null terminator
    *p++ = '/'; // append a slash to the path -> for the next file

    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        // skip empty directory entries
        if (de.inum == 0) {
            continue;
        }

        // skip "." and ".."
        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
            continue;
        }

        // Build the full path
        memmove(p, de.name, DIRSIZ);
        // ensure the path is null-terminated
        p[DIRSIZ] = 0;

        // get the status of the current file
        if (stat(buf, &st) < 0) {
            printf("find: cannot stat %s\n", buf);
            continue;
        }

        // check if the current file matches the target
        if (strcmp(de.name, target) == 0) {
            printf("%s\n", buf);
        }

        // if it's a directory, recurse into it
        if (st.type == T_DIR) {
            find(buf, target);
        }
    }

    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: find <path> <filename>\n");
        exit(1);
    }

    find(argv[1], argv[2]);
    exit(0);
}
