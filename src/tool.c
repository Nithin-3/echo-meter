#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tool.h"
#include "conf.h"
#include "path.h"

#define RED   "\x1b[31m"
#define GRN   "\x1b[32m"
#define YEL   "\x1b[33m"
#define RESET "\x1b[0m"

static void logInfo(const char *msg) {
    printf(GRN "[INFO] %s" RESET "\n", msg);
}

static void logError(const char *msg) {
    fprintf(stderr, RED "[ERROR] %s" RESET "\n", msg);
}

static float clamp(float val) {
    if (val < 0.0) return 0.0;
    if (val > 1.0) return 1.0;
    return val;
}

static Type parseType(const char *type) {
    if (strcmp(type, "aud") == 0) return AUD;
    if (strcmp(type, "bri") == 0) return BRI;
    if (strcmp(type, "mic") == 0) return MIC;
    return INVALIDT;
}

static Sys *getCachedSys() {
    static Sys sys;
    static int loaded = 0;
    if (!loaded) {
        char *path = findPath("conf.json");
        if (path == NULL || !catSys(path, &sys)) {
            logError("Failed to load conf.json");
            return NULL;
        }
        loaded = 1;
        logInfo(sys.volumeTool);
    }
    return &sys;
}

float getVal(const char *mode) {
    Type type = parseType(mode);
    if (type == INVALIDT) {
        logError("Invalid mode in getVal");
        return -1.0;
    }

    Sys *s = getCachedSys();
    char cmd[256] = "";
    switch (type) {
        case AUD:
            if (strcmp(s->volumeTool, "wpctl") == 0)
                snprintf(cmd, sizeof(cmd), "wpctl get-volume @DEFAULT_AUDIO_SINK@ | awk '{print $2}'");
            else if (strcmp(s->volumeTool, "pactl") == 0)
                snprintf(cmd, sizeof(cmd), "pactl get-sink-volume @DEFAULT_SINK@ | awk '{print $5}'");
            break;
        case BRI:
            if (strcmp(s->brightnessTool, "brightnessctl") == 0)
                snprintf(cmd, sizeof(cmd), "brightnessctl -m | awk -F',' '{print $4}'");
            break;
        case MIC:
            if (strcmp(s->micTool, "wpctl") == 0)
                snprintf(cmd, sizeof(cmd), "wpctl get-volume @DEFAULT_AUDIO_SOURCE@ | awk '{print $2}'");
            else if (strcmp(s->micTool, "pactl") == 0)
                snprintf(cmd, sizeof(cmd), "pactl get-source-volume @DEFAULT_SOURCE@ | awk '{print $5}'");
            break;
        default: return -1.0;
    }

    if (cmd[0] == '\0') return -1.0;

    FILE *fp = popen(cmd, "r");
    if (!fp) {
        logError("Failed to run shell command");
        return -1.0;
    }

    char buf[32];
    if (!fgets(buf, sizeof(buf), fp)) {
        logError("No output from shell");
        pclose(fp);
        return -1.0;
    }
    pclose(fp);
    buf[strcspn(buf, "\n")] = '\0';

    if (!isdigit(buf[0]) && buf[0] != '.') {
        logError("Unexpected output format");
        return -1.0;
    }

    float val = atof(buf);
    return strchr(buf, '%') ? val / 100.0f : val;
}

void setVal(const char *mode, float val) {
    Type type = parseType(mode);
    if (type == INVALIDT) {
        logError("Invalid mode in setVal");
        return;
    }

    Sys *s = getCachedSys();
    if (!s) return;

    val = clamp(val);

    char cmd[256] = "";

    switch (type) {
        case AUD:
            if (strcmp(s->volumeTool, "wpctl") == 0)
                snprintf(cmd, sizeof(cmd), "wpctl set-volume @DEFAULT_AUDIO_SINK@ %.2f", val);
            else if (strcmp(s->volumeTool, "pactl") == 0)
                snprintf(cmd, sizeof(cmd), "pactl set-sink-volume @DEFAULT_SINK@ %d%%", (int)(val * 100));
            break;
        case BRI:
            if (strcmp(s->brightnessTool, "brightnessctl") == 0)
                snprintf(cmd, sizeof(cmd), "brightnessctl s %d%%", (int)(val * 100));
            break;
        case MIC:
            if (strcmp(s->micTool, "wpctl") == 0)
                snprintf(cmd, sizeof(cmd), "wpctl set-volume @DEFAULT_AUDIO_SOURCE@ %.2f", val);
            else if (strcmp(s->micTool, "pactl") == 0)
                snprintf(cmd, sizeof(cmd), "pactl set-source-volume @DEFAULT_SOURCE@ %d%%", (int)(val * 100));
            break;
        default: return;
    }

    if (cmd[0] != '\0') {
        logInfo(cmd);
        system(cmd);
    }
}

void step(const char *mode, bool direction) {
    Type type = parseType(mode);
    if (type == INVALIDT) {
        logError("Invalid mode in step");
        return;
    }

    Sys *s = getCachedSys();
    if (!s) return;

    float current = getVal(mode);
    if (current < 0.0f) return;

    float stepVal = 0.0;
    switch (type) {
        case AUD: stepVal = s->volumeStep; break;
        case BRI: stepVal = s->brightnessStep; break;
        case MIC: stepVal = s->micStep; break;
        default: return;
    }

    float newVal = direction ? current + stepVal : current - stepVal;
    setVal(mode, newVal);
}

