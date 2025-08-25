#include "conf.h"
#include <string.h>
#include <json-glib/json-glib.h>
#include <stdio.h>
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"

static bool is_valid_orientation(const char *val) {
    return (strcmp(val, "horizontal") == 0 || strcmp(val, "vertical") == 0);
}

static bool is_valid_vertical_align(const char *val) {
    return (strcmp(val, "top") == 0 || strcmp(val, "bottom") == 0 || strcmp(val, "center") == 0);
}

static bool is_valid_horizontal_align(const char *val) {
    return (strcmp(val, "left") == 0 || strcmp(val, "right") == 0 || strcmp(val, "center") == 0);
}

static bool is_valid_tool(const char *val, const char *type) {
    if (strcmp(type, "volume") == 0 || strcmp(type, "mic") == 0)
        return (strcmp(val, "pactl") == 0 || strcmp(val, "wpctl") == 0);
    if (strcmp(type, "brightness") == 0)
        return (strcmp(val, "brightnessctl") == 0);
    return false;
}

static int clamp_step(int val) {
    return (val >= 0 && val <= 50) ? val : -1;
}

bool catConf(const char *path, Config *config) {
    // Set default values
    strncpy(config->orientation, "horizontal", sizeof(config->orientation) - 1);
    config->orientation[sizeof(config->orientation) - 1] = '\0';

    config->invertDirection = false;
    config->timeout = 5;

    config->has_explicit_pos = false;
    strncpy(config->vertical_align, "top", sizeof(config->vertical_align) - 1);
    config->vertical_align[sizeof(config->vertical_align) - 1] = '\0';

    strncpy(config->horizontal_align, "center", sizeof(config->horizontal_align) - 1);
    config->horizontal_align[sizeof(config->horizontal_align) - 1] = '\0';

    config->margin = 0;

    // Default icons as empty strings
    config->icon.sound[0] = '\0';
    config->icon.mute[0] = '\0';
    config->icon.brightness[0] = '\0';
    config->icon.mic[0] = '\0';
    config->icon.mic_off[0] = '\0';

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

    JsonObject *root_obj = json_node_get_object(root);

    if (json_object_has_member(root_obj, "orientation")) {
        const char *orientation = json_object_get_string_member(root_obj, "orientation");
        if (is_valid_orientation(orientation)) {
            strncpy(config->orientation, orientation, sizeof(config->orientation) - 1);
            config->orientation[sizeof(config->orientation) - 1] = '\0';
        }
    }

    if (json_object_has_member(root_obj, "invert-direction")) {
        config->invertDirection = json_object_get_boolean_member(root_obj, "invert-direction");
    }

    if (json_object_has_member(root_obj, "timeout")) {
        config->timeout = json_object_get_int_member(root_obj, "timeout");
    }

    if (json_object_has_member(root_obj, "window_position")) {
        JsonObject *pos_obj = json_object_get_object_member(root_obj, "window_position");

        if (json_object_has_member(pos_obj, "x") && json_object_has_member(pos_obj, "y")) {
            config->has_explicit_pos = true;
            config->x = json_object_get_int_member(pos_obj, "x");
            config->y = json_object_get_int_member(pos_obj, "y");
        } else {
            config->has_explicit_pos = false;

            if (json_object_has_member(pos_obj, "vertical")) {
                const char *v_align = json_object_get_string_member(pos_obj, "vertical");
                if (is_valid_vertical_align(v_align)) {
                    strncpy(config->vertical_align, v_align, sizeof(config->vertical_align) - 1);
                    config->vertical_align[sizeof(config->vertical_align) - 1] = '\0';
                }
            }

            if (json_object_has_member(pos_obj, "horizontal")) {
                const char *h_align = json_object_get_string_member(pos_obj, "horizontal");
                if (is_valid_horizontal_align(h_align)) {
                    strncpy(config->horizontal_align, h_align, sizeof(config->horizontal_align) - 1);
                    config->horizontal_align[sizeof(config->horizontal_align) - 1] = '\0';
                }
            }

            if (json_object_has_member(pos_obj, "margin")) {
                int margin = json_object_get_int_member(pos_obj, "margin");
                config->margin = margin < 0 ? 0 : margin;
            }
        }
    }

    if (json_object_has_member(root_obj, "icon")) {
        JsonObject *icon_obj = json_object_get_object_member(root_obj, "icon");

        if (json_object_has_member(icon_obj, "sound")) {
            strncpy(config->icon.sound, json_object_get_string_member(icon_obj, "sound"), sizeof(config->icon.sound) - 1);
            config->icon.sound[sizeof(config->icon.sound) - 1] = '\0';
        }

        if (json_object_has_member(icon_obj, "mute")) {
            strncpy(config->icon.mute, json_object_get_string_member(icon_obj, "mute"), sizeof(config->icon.mute) - 1);
            config->icon.mute[sizeof(config->icon.mute) - 1] = '\0';
        }

        if (json_object_has_member(icon_obj, "brightness")) {
            strncpy(config->icon.brightness, json_object_get_string_member(icon_obj, "brightness"), sizeof(config->icon.brightness) - 1);
            config->icon.brightness[sizeof(config->icon.brightness) - 1] = '\0';
        }

        if (json_object_has_member(icon_obj, "mic")) {
            strncpy(config->icon.mic, json_object_get_string_member(icon_obj, "mic"), sizeof(config->icon.mic) - 1);
            config->icon.mic[sizeof(config->icon.mic) - 1] = '\0';
        }

        if (json_object_has_member(icon_obj, "mic_off")) {
            strncpy(config->icon.mic_off, json_object_get_string_member(icon_obj, "mic_off"), sizeof(config->icon.mic_off) - 1);
            config->icon.mic_off[sizeof(config->icon.mic_off) - 1] = '\0';
        }
    }

    g_object_unref(parser);
    return true;
}

bool catSys(const char *path, Sys *sys) {
    // Set default values before JSON parsing
    strncpy(sys->volume_tool, "wpctl", sizeof(sys->volume_tool) - 1);
    sys->volume_tool[sizeof(sys->volume_tool) - 1] = '\0';

    strncpy(sys->brightness_tool, "brightnessctl", sizeof(sys->brightness_tool) - 1);
    sys->brightness_tool[sizeof(sys->brightness_tool) - 1] = '\0';

    strncpy(sys->mic_tool, "pactl", sizeof(sys->mic_tool) - 1);
    sys->mic_tool[sizeof(sys->mic_tool) - 1] = '\0';

    sys->volume_step = 5;
    sys->brightness_step = 10;
    sys->mic_step = 3;

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

    JsonObject *root_obj = json_node_get_object(root);

    if (json_object_has_member(root_obj, "system_info")) {
        JsonObject *sys_obj = json_object_get_object_member(root_obj, "system_info");

        const char *vol_tool = json_object_get_string_member_with_default(sys_obj, "volume_tool", sys->volume_tool);
        if (is_valid_tool(vol_tool, "volume")) {
            strncpy(sys->volume_tool, vol_tool, sizeof(sys->volume_tool) - 1);
            sys->volume_tool[sizeof(sys->volume_tool) - 1] = '\0';
        }

        const char *bri_tool = json_object_get_string_member_with_default(sys_obj, "brightness_tool", sys->brightness_tool);
        if (is_valid_tool(bri_tool, "brightness")) {
            strncpy(sys->brightness_tool, bri_tool, sizeof(sys->brightness_tool) - 1);
            sys->brightness_tool[sizeof(sys->brightness_tool) - 1] = '\0';
        }

        const char *mic_tool = json_object_get_string_member_with_default(sys_obj, "mic_tool", sys->mic_tool);
        if (is_valid_tool(mic_tool, "mic")) {
            strncpy(sys->mic_tool, mic_tool, sizeof(sys->mic_tool) - 1);
            sys->mic_tool[sizeof(sys->mic_tool) - 1] = '\0';
        }

        int v_step = json_object_get_int_member(sys_obj, "volume_step");
        if (json_object_has_member(sys_obj, "volume_step") && clamp_step(v_step) != -1)
            sys->volume_step = v_step;

        int b_step = json_object_get_int_member(sys_obj, "brightness_step");
        if (json_object_has_member(sys_obj, "brightness_step") && clamp_step(b_step) != -1)
            sys->brightness_step = b_step;

        int m_step = json_object_get_int_member(sys_obj, "mic_step");
        if (json_object_has_member(sys_obj, "mic_step") && clamp_step(m_step) != -1)
            sys->mic_step = m_step;
    }

    g_object_unref(parser);
    return true;
}

