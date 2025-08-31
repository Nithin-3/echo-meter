#include "conf.h"
#include <string.h>
#include <json-glib/json-glib.h>
#include <stdio.h>
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"

static bool isValidOrientation(const char *val) {
    return (strcmp(val, "horizontal") == 0 || strcmp(val, "vertical") == 0);
}

static bool isValidVerticalAlign(const char *val) {
    return (strcmp(val, "top") == 0 || strcmp(val, "bottom") == 0 || strcmp(val, "center") == 0);
}

static bool isValidHorizontalAlign(const char *val) {
    return (strcmp(val, "left") == 0 || strcmp(val, "right") == 0 || strcmp(val, "center") == 0);
}

static int clampStep(int val) {
    return (val >= 0 && val <= 50) ? val : -1;
}

bool catConf(const char *path, Config *config) {
    // Set default values
    strncpy(config->orientation, "horizontal", sizeof(config->orientation) - 1);
    config->orientation[sizeof(config->orientation) - 1] = '\0';

    config->invertDirection = false;
    config->timeout = 5;

    config->hasExplicitPos = false;
    strncpy(config->verticalAlign, "top", sizeof(config->verticalAlign) - 1);
    config->verticalAlign[sizeof(config->verticalAlign) - 1] = '\0';

    strncpy(config->horizontalAlign, "center", sizeof(config->horizontalAlign) - 1);
    config->horizontalAlign[sizeof(config->horizontalAlign) - 1] = '\0';

    config->margin = 0;

    // Default icons as empty strings
    config->icon.sound[0] = '\0';
    config->icon.mute[0] = '\0';
    config->icon.brightness[0] = '\0';
    config->icon.mic[0] = '\0';
    config->icon.micOff[0] = '\0';
    config->icon.capslockON[0] = '\0';
    config->icon.capslockOFF[0] = '\0';
    config->icon.numlockON[0] = '\0';
    config->icon.numlockOFF[0] = '\0';
    config->icon.scrolllockON[0] = '\0';
    config->icon.scrolllockOFF[0] = '\0';

    // Parse JSON config
    GError *error = NULL;
    JsonParser *parser = json_parser_new();

    if (!json_parser_load_from_file(parser, path, &error)) {
        fprintf(stderr, RED "[ERROR]" RESET " Failed to load JSON file: %s\n", error->message);
        g_error_free(error);
        g_object_unref(parser);
        return false;
    }

    JsonNode *root = json_parser_get_root(parser);
    if (!JSON_NODE_HOLDS_OBJECT(root)) {
        fprintf(stderr, RED "[ERROR]" RESET " Root element is not an object\n");
        g_object_unref(parser);
        return false;
    }

    JsonObject *rootObj = json_node_get_object(root);

    if (json_object_has_member(rootObj, "orientation")) {
        const char *orientation = json_object_get_string_member(rootObj, "orientation");
        if (isValidOrientation(orientation)) {
            strncpy(config->orientation, orientation, sizeof(config->orientation) - 1);
            config->orientation[sizeof(config->orientation) - 1] = '\0';
        }
    }

    if (json_object_has_member(rootObj, "invert-direction")) {
        config->invertDirection = json_object_get_boolean_member(rootObj, "invert-direction");
    }

    if (json_object_has_member(rootObj, "timeout")) {
        config->timeout = json_object_get_int_member(rootObj, "timeout");
    }

    if (json_object_has_member(rootObj, "window_position")) {
        JsonObject *posObj = json_object_get_object_member(rootObj, "window_position");

        if (json_object_has_member(posObj, "x") && json_object_has_member(posObj, "y")) {
            config->hasExplicitPos = true;
            config->x = json_object_get_int_member(posObj, "x");
            config->y = json_object_get_int_member(posObj, "y");
        } else {
            config->hasExplicitPos = false;

            if (json_object_has_member(posObj, "vertical")) {
                const char *vAlign = json_object_get_string_member(posObj, "vertical");
                if (isValidVerticalAlign(vAlign)) {
                    strncpy(config->verticalAlign, vAlign, sizeof(config->verticalAlign) - 1);
                    config->verticalAlign[sizeof(config->verticalAlign) - 1] = '\0';
                }
            }

            if (json_object_has_member(posObj, "horizontal")) {
                const char *hAlign = json_object_get_string_member(posObj, "horizontal");
                if (isValidHorizontalAlign(hAlign)) {
                    strncpy(config->horizontalAlign, hAlign, sizeof(config->horizontalAlign) - 1);
                    config->horizontalAlign[sizeof(config->horizontalAlign) - 1] = '\0';
                }
            }

            if (json_object_has_member(posObj, "margin")) {
                int margin = json_object_get_int_member(posObj, "margin");
                config->margin = margin < 0 ? 0 : margin;
            }
        }
    }

    if (json_object_has_member(rootObj, "icon")) {
        JsonObject *iconObj = json_object_get_object_member(rootObj, "icon");

        if (json_object_has_member(iconObj, "sound")) {
            strncpy(config->icon.sound, json_object_get_string_member(iconObj, "sound"), sizeof(config->icon.sound) - 1);
            config->icon.sound[sizeof(config->icon.sound) - 1] = '\0';
        }

        if (json_object_has_member(iconObj, "mute")) {
            strncpy(config->icon.mute, json_object_get_string_member(iconObj, "mute"), sizeof(config->icon.mute) - 1);
            config->icon.mute[sizeof(config->icon.mute) - 1] = '\0';
        }

        if (json_object_has_member(iconObj, "brightness")) {
            strncpy(config->icon.brightness, json_object_get_string_member(iconObj, "brightness"), sizeof(config->icon.brightness) - 1);
            config->icon.brightness[sizeof(config->icon.brightness) - 1] = '\0';
        }

        if (json_object_has_member(iconObj, "mic")) {
            strncpy(config->icon.mic, json_object_get_string_member(iconObj, "mic"), sizeof(config->icon.mic) - 1);
            config->icon.mic[sizeof(config->icon.mic) - 1] = '\0';
        }

        if (json_object_has_member(iconObj, "mic_off")) {
            strncpy(config->icon.micOff, json_object_get_string_member(iconObj, "mic_off"), sizeof(config->icon.micOff) - 1);
            config->icon.micOff[sizeof(config->icon.micOff) - 1] = '\0';
        }
        if (json_object_has_member(iconObj, "capslock_on")) {
            strncpy(config->icon.capslockON, json_object_get_string_member(iconObj, "capslock_on"), sizeof(config->icon.capslockON) - 1);
            config->icon.capslockON[sizeof(config->icon.capslockON) - 1] = '\0';
        }
        if (json_object_has_member(iconObj, "capslock_off")) {
            strncpy(config->icon.capslockOFF, json_object_get_string_member(iconObj, "capslock_off"), sizeof(config->icon.capslockOFF) - 1);
            config->icon.capslockOFF[sizeof(config->icon.capslockOFF) - 1] = '\0';
        }
        if (json_object_has_member(iconObj, "numlock_on")) {
            strncpy(config->icon.numlockON, json_object_get_string_member(iconObj, "numlock_on"), sizeof(config->icon.numlockON) - 1);
            config->icon.numlockON[sizeof(config->icon.numlockON) - 1] = '\0';
        }
        if (json_object_has_member(iconObj, "numlock_off")) {
            strncpy(config->icon.numlockOFF, json_object_get_string_member(iconObj, "numlock_off"), sizeof(config->icon.numlockOFF) - 1);
            config->icon.numlockOFF[sizeof(config->icon.numlockOFF) - 1] = '\0';
        }
        if (json_object_has_member(iconObj, "scrolllock_on")) {
            strncpy(config->icon.scrolllockON, json_object_get_string_member(iconObj, "scrolllock_on"), sizeof(config->icon.scrolllockON) - 1);
            config->icon.scrolllockON[sizeof(config->icon.scrolllockON) - 1] = '\0';
        }
        if (json_object_has_member(iconObj, "scrolllock_off")) {
            strncpy(config->icon.scrolllockOFF, json_object_get_string_member(iconObj, "scrolllock_off"), sizeof(config->icon.scrolllockOFF) - 1);
            config->icon.scrolllockOFF[sizeof(config->icon.scrolllockOFF) - 1] = '\0';
        }
    }

    g_object_unref(parser);
    return true;
}

bool catSys(const char *path, Sys *sys) {
    sys->volumeStep = 5;
    sys->brightnessStep = 10;
    sys->micStep = 3;

    // Now load JSON and override if valid
    GError *error = NULL;
    JsonParser *parser = json_parser_new();

    if (!json_parser_load_from_file(parser, path, &error)) {
        fprintf(stderr, RED "[ERROR]" RESET " Failed to load JSON file: %s\n", error->message);
        g_error_free(error);
        g_object_unref(parser);
        return false;
    }

    JsonNode *root = json_parser_get_root(parser);
    if (!JSON_NODE_HOLDS_OBJECT(root)) {
        fprintf(stderr, RED "[ERROR]" RESET " Root element is not an object\n");
        g_object_unref(parser);
        return false;
    }

    JsonObject *rootObj = json_node_get_object(root);

    if (json_object_has_member(rootObj, "system_info")) {
        JsonObject *sysObj = json_object_get_object_member(rootObj, "system_info");

        int vStep = json_object_get_int_member(sysObj, "volume_step");
        if (json_object_has_member(sysObj, "volume_step") && clampStep(vStep) != -1)
            sys->volumeStep = vStep;

        int bStep = json_object_get_int_member(sysObj, "brightness_step");
        if (json_object_has_member(sysObj, "brightness_step") && clampStep(bStep) != -1)
            sys->brightnessStep = bStep;

        int mStep = json_object_get_int_member(sysObj, "mic_step");
        if (json_object_has_member(sysObj, "mic_step") && clampStep(mStep) != -1)
            sys->micStep = mStep;
    }

    g_object_unref(parser);
    return true;
}

