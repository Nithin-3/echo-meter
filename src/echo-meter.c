#include <gtk/gtk.h>
#include <string.h>
#include <gtk-layer-shell/gtk-layer-shell.h>
#include "conf.h"
#include "path.h"


GtkWidget *globalWindow = NULL;
GtkWidget *progressBar = NULL;
static double lastVol = -1.0;
static double lastBri = -1.0;

static void onDestroy(GtkWidget *_, gpointer _data) {
    globalWindow = NULL;
}

static gboolean onClose(gpointer data) {
    if (globalWindow) {
        gtk_window_close(GTK_WINDOW(globalWindow));
        globalWindow = NULL;
    }

    GApplication *app = g_application_get_default();
    if (app) {
        g_application_quit(app);
    }

    return G_SOURCE_REMOVE;
}

static void prog(double val, char* txt) {
    if (progressBar != NULL) {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressBar), val);

        // Get the overlay label and update its text
        GtkWidget *label = g_object_get_data(G_OBJECT(progressBar), "progress-label");
        if (label) {
            gtk_label_set_text(GTK_LABEL(label), txt);
        }
    }
}

static gboolean update(gpointer data) {
    if (progressBar == NULL) {
        // Safety check: Progress bar not ready yet
        return G_SOURCE_CONTINUE;
    }

    const char *mode = (const char *)data;
    if (mode == NULL) {
        g_warning("update(): mode is NULL");
        return G_SOURCE_CONTINUE;
    }

    double fraction_vol = 0.0;
    double fraction_bri = 0.0;
    char status_text[128] = "";

    FILE *fp_vol = popen("wpctl get-volume @DEFAULT_AUDIO_SINK@ | awk '{print $2}'", "r");
    if (fp_vol) {
        if (fscanf(fp_vol, "%lf", &fraction_vol) != 1) {
            fraction_vol = 0.0;
        }
        pclose(fp_vol);
    } else {
        g_warning("Failed to run wpctl");
    }

    FILE *fp = popen("brightnessctl -m | awk -F',' '{print $4}'", "r");
    if (fp) {
        char percent_str[16] = {0};  // zero-initialize to avoid garbage
        if (fgets(percent_str, sizeof(percent_str), fp)) {
            int percent = 0;
            if (sscanf(percent_str, "%d%%", &percent) == 1) {
                fraction_bri = percent / 100.0;
            } else {
                g_warning("Failed to parse brightness: %s", percent_str);
            }
        } else {
            g_warning("Failed to read brightness output");
        }
        pclose(fp);
    } else {
        g_warning("Failed to run brightnessctl");
    }

    if (lastVol != fraction_vol) {
        snprintf(status_text, sizeof(status_text), " %.0f%%", fraction_vol * 100);
        prog(fraction_vol, status_text);
        if (strcmp(mode, "aud") == 0 && lastVol == -1.0) {
            lastVol = fraction_vol;
            lastBri = fraction_bri;
            return G_SOURCE_CONTINUE;
        }
        lastVol = fraction_vol;
    }

    if (lastBri != fraction_bri) {
        snprintf(status_text, sizeof(status_text), "🔆 %.0f%%", fraction_bri * 100);
        prog(fraction_bri, status_text);
        if (strcmp(mode, "bri") == 0 && lastVol == -1.0) {
            lastVol = fraction_vol;
            lastBri = fraction_bri;
            return G_SOURCE_CONTINUE;
        }
        lastBri = fraction_bri;
    }

    return G_SOURCE_CONTINUE;
}

static void activate(GtkApplication *app, gpointer user_data) {
    if (globalWindow != NULL) {
        gtk_window_present(GTK_WINDOW(globalWindow));
        g_timeout_add_seconds(15, onClose, NULL);  // Reset close timer
        return;
    }

    Config config;
    char* path = find_path("conf.json");
    gboolean config_loaded = path ? catConf(path, &config) : FALSE;

    // Fallback defaults if config not loaded
    if (!config_loaded) {
        strcpy(config.orientation, "horizontal");
        config.has_explicit_pos = false;
        strcpy(config.vertical_align, "center");
        strcpy(config.horizontal_align, "center");
        config.margin = 0;
    }

    globalWindow = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(globalWindow), "Indicator");
    gtk_window_set_resizable(GTK_WINDOW(globalWindow), FALSE);

    g_signal_connect(globalWindow, "destroy", G_CALLBACK(onDestroy), NULL);

    gtk_layer_init_for_window(GTK_WINDOW(globalWindow));
    gtk_layer_set_layer(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_exclusive_zone(GTK_WINDOW(globalWindow), -1); // Do NOT reserve space

    if (config.has_explicit_pos) {
        gtk_layer_set_anchor(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
        gtk_layer_set_anchor(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
        gtk_layer_set_margin(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_TOP, config.y);
        gtk_layer_set_margin(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_LEFT, config.x);
    } else {
        gtk_layer_set_anchor(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_TOP, strcmp(config.vertical_align, "top") == 0);
        gtk_layer_set_anchor(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_BOTTOM, strcmp(config.vertical_align, "bottom") == 0);
        gtk_layer_set_anchor(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_LEFT, strcmp(config.horizontal_align, "left") == 0);
        gtk_layer_set_anchor(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_RIGHT, strcmp(config.horizontal_align, "right") == 0);

        gtk_layer_set_margin(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_TOP, config.margin);
        gtk_layer_set_margin(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_BOTTOM, config.margin);
        gtk_layer_set_margin(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_LEFT, config.margin);
        gtk_layer_set_margin(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_RIGHT, config.margin);
    }

    // Apply CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, find_path("style.css"));
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    GtkOrientation orient = GTK_ORIENTATION_HORIZONTAL;
    if (strcmp(config.orientation, "vertical") == 0) {
        orient = GTK_ORIENTATION_VERTICAL;
    }

    GtkWidget *box = gtk_box_new(orient, 10);
    gtk_window_set_child(GTK_WINDOW(globalWindow), box);

    GtkWidget *overlay = gtk_overlay_new();

    progressBar = gtk_progress_bar_new();
    gtk_orientable_set_orientation(GTK_ORIENTABLE(progressBar), orient);
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progressBar), FALSE);  // Hide built-in text

    gtk_progress_bar_set_inverted(GTK_PROGRESS_BAR(progressBar), config.invertDirection);
    if (orient == GTK_ORIENTATION_VERTICAL) {
        gtk_widget_set_size_request(progressBar, 25, 50);
    }
    gtk_widget_set_name(progressBar, "status-bar");

    GtkWidget *label = gtk_label_new("");
    gtk_widget_set_name(label, "progress-label");
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(label, GTK_ALIGN_CENTER);

    gtk_overlay_set_child(GTK_OVERLAY(overlay), progressBar);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), label);

    // Save label pointer for update function
    g_object_set_data(G_OBJECT(progressBar), "progress-label", label);

    gtk_box_append(GTK_BOX(box), overlay);
    gtk_window_present(GTK_WINDOW(globalWindow));

    g_timeout_add(100, update, user_data);
    g_timeout_add_seconds(15, onClose, NULL);
}

static void onOpen(GApplication *app, GFile **files, int n_files, const char *hint, gpointer user_data) {
    g_application_activate(app);
}

int main(int argc, char **argv) {
    const char *valid_args[] = {"aud", "bri", "mic"};
    const int valid_count = sizeof(valid_args) / sizeof(valid_args[0]);
    int valid = 0;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [aud|bri|mic]\n", argv[0]);
        return 1;
    }

    for (int i = 0; i < valid_count; i++) {
        if (strcmp(argv[1], valid_args[i]) == 0) {
            valid = 1;
            break;
        }
    }

    if (!valid) {
        fprintf(stderr, "Invalid argument. Usage: %s [aud|bri|mic]\n", argv[0]);
        return 1;
    }

    GtkApplication *app = gtk_application_new("com.nit.echo-meter", G_APPLICATION_HANDLES_OPEN);
    g_signal_connect(app, "open", G_CALLBACK(onOpen), NULL);
    g_signal_connect(app, "activate", G_CALLBACK(activate), argv[1]);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}

