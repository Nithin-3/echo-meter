#ifndef CONF_H
#define CONF_H

#include <gtk/gtk.h>
#include <stdbool.h>
typedef struct {
    char sound[32];
    char mute[32];
    char brightness[32];
    char mic[32];
    char micOff[32];
    char capslockON[32];
    char capslockOFF[32];
    char numlockON[32];
    char numlockOFF[32];
    char scrolllockON[32];
    char scrolllockOFF[32];
} IconConfig;
typedef struct {
    char volumeTool[32], brightnessTool[32], micTool[32];
    int volumeStep, brightnessStep, micStep;
} Sys;
typedef struct {
    char orientation[16], verticalAlign[16], horizontalAlign[16];
    gboolean hasExplicitPos, invertDirection;
    int x, y, margin,timeout;
    IconConfig icon;
} Config;

bool catConf(const char *filename, Config *config);
bool catSys(const char *filename, Sys *sys);

#endif

