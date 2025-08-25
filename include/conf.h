#ifndef CONF_H
#define CONF_H

#include <gtk/gtk.h>
#include <stdbool.h>
typedef struct {
    char sound[32];
    char mute[32];
    char brightness[32];
    char mic[32];
    char mic_off[32];
} IconConfig;
typedef struct {
    char volume_tool[32], brightness_tool[32], mic_tool[32];
    int volume_step, brightness_step, mic_step;
} Sys;
typedef struct {
    char orientation[16], vertical_align[16], horizontal_align[16];
    gboolean has_explicit_pos, invertDirection;
    int x, y, margin,timeout;
    IconConfig icon;
} Config;

bool catConf(const char *filename, Config *config);
bool catSys(const char *filename, Sys *sys);

#endif

