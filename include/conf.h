#ifndef CONF_H
#define CONF_H

#include <gtk/gtk.h>
#include <stdbool.h>

typedef struct {
    char orientation[16];
    gboolean has_explicit_pos;
    gboolean invertDirection;
    int x, y;
    char vertical_align[16];
    char horizontal_align[16];
    int margin;
} Config;

bool catConf(const char *filename, Config *config);

#endif

