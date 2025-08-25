#ifndef PATH_H
#define PATH_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_PATH 512

static inline char* findPath(const char* filename) {
    static char path[MAX_PATH];
    const char* home = getenv("HOME");

    if (home) {
        snprintf(path, MAX_PATH, "%s/.config/echo-meter/%s", home, filename);
        if (access(path, R_OK) == 0)
            return path;
    }

    snprintf(path, MAX_PATH, "/usr/share/echo-meter/%s", filename);
    if (access(path, R_OK) == 0)
        return path;

    return NULL;
}

#endif

