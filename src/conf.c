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

bool catConf(const char *filename, Config *config) {
    GError *error = NULL;
    JsonParser *parser = json_parser_new();

    if (!json_parser_load_from_file(parser, filename, &error)) {
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

    if (!json_object_has_member(root_obj, "orientation")) {
        fprintf(stderr, RED "[ERROR]" RESET " Missing 'orientation' field\n");
        g_object_unref(parser);
        return false;
    }

    const char *orientation = json_object_get_string_member(root_obj, "orientation");
    if (!is_valid_orientation(orientation)) {
        fprintf(stderr, RED "[ERROR]" RESET " Invalid orientation: '%s'. Must be 'horizontal' or 'vertical'.\n", orientation);
        g_object_unref(parser);
        return false;
    }
    if (json_object_has_member(root_obj, "invert-direction")) {
        config->invertDirection = json_object_get_boolean_member(root_obj, "invert-direction");
    } else {
        config->invertDirection = false;  // Default
    }
    strncpy(config->orientation, orientation, sizeof(config->orientation));
    config->orientation[sizeof(config->orientation)-1] = '\0';

    if (!json_object_has_member(root_obj, "window_position")) {
        fprintf(stderr, RED "[ERROR]" RESET " Missing 'window_position' field\n");
        g_object_unref(parser);
        return false;
    }

    JsonObject *pos_obj = json_object_get_object_member(root_obj, "window_position");

    if (json_object_has_member(pos_obj, "x") && json_object_has_member(pos_obj, "y")) {
        config->has_explicit_pos = true;
        config->x = json_object_get_int_member(pos_obj, "x");
        config->y = json_object_get_int_member(pos_obj, "y");
    } else {
        config->has_explicit_pos = false;

        if (json_object_has_member(pos_obj, "vertical")) {
            const char *v_align = json_object_get_string_member(pos_obj, "vertical");
            if (!is_valid_vertical_align(v_align)) {
                fprintf(stderr, RED "[ERROR]" RESET " Invalid vertical alignment: '%s'. Must be 'top', 'bottom' or 'center'.\n", v_align);
                g_object_unref(parser);
                return false;
            }
            strncpy(config->vertical_align, v_align, sizeof(config->vertical_align));
            config->vertical_align[sizeof(config->vertical_align)-1] = '\0';
        } else {
            strcpy(config->vertical_align, "top");
        }

        if (json_object_has_member(pos_obj, "horizontal")) {
            const char *h_align = json_object_get_string_member(pos_obj, "horizontal");
            if (!is_valid_horizontal_align(h_align)) {
                fprintf(stderr, RED "[ERROR]" RESET " Invalid horizontal alignment: '%s'. Must be 'left', 'right' or 'center'.\n", h_align);
                g_object_unref(parser);
                return false;
            }
            strncpy(config->horizontal_align, h_align, sizeof(config->horizontal_align));
            config->horizontal_align[sizeof(config->horizontal_align)-1] = '\0';
        } else {
            strcpy(config->horizontal_align, "center");
        }

        if (json_object_has_member(pos_obj, "margin")) {
            int margin = json_object_get_int_member(pos_obj, "margin");
            if (margin < 0) {
                fprintf(stderr, RED "[ERROR]" RESET " Invalid margin value: %d. Must be >= 0.\n", margin);
                g_object_unref(parser);
                return false;
            }
            config->margin = margin;
        } else {
            config->margin = 0;
        }
    }

    g_object_unref(parser);
    return true;
}

