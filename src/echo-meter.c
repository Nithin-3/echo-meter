#include <gtk/gtk.h>
#include <string.h>
#include <gtk-layer-shell/gtk-layer-shell.h>
#include "conf.h"
#include "path.h"
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

GtkWidget *globalWindow = NULL;
GtkWidget *progressBar = NULL;
static double lastVol = -1.0;
static double lastBri = -1.0;
static guint timeoutId = 0;
static char *globalMode = NULL;

static void onDestroy(GtkWidget *widget, gpointer data) {
    (void)widget;
    (void)data;
    globalWindow = NULL;
    g_print(GREEN "[INFO]" RESET " Window destroyed.\n");
}

static gboolean onClose(gpointer data) {
    (void)data;
    if (globalWindow) {
        g_print(GREEN "[INFO]" RESET " Closing window...\n");
        gtk_window_close(GTK_WINDOW(globalWindow));
        globalWindow = NULL;
    }

    GApplication *app = g_application_get_default();
    if (app) {
        g_print(GREEN "[INFO]" RESET " Quitting application...\n");
        g_application_quit(app);
    }

    return G_SOURCE_REMOVE;
}

static void resetTimer() {
    if (timeoutId != 0) {
        g_source_remove(timeoutId);
        g_print(CYAN "[TIMER]" RESET " Existing timer removed.\n");
    }
    timeoutId = g_timeout_add_seconds(5, onClose, NULL);
    g_print(CYAN "[TIMER]" RESET " New auto-close timer set to 5 seconds.\n");
}

static void updateProgress(double val, const char *txt) {
    if (progressBar != NULL) {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressBar), val);

        GtkWidget *label = g_object_get_data(G_OBJECT(progressBar), "progress-label");
        if (label) {
            gtk_label_set_text(GTK_LABEL(label), txt);
        }
    }
}

static gboolean updateStatus(gpointer data) {
    (void)data;

    if (progressBar == NULL || globalMode == NULL) {
        return G_SOURCE_CONTINUE;
    }

    double fractionVol = 0.0;
    double fractionBri = 0.0;
    char statusText[128] = "";

    FILE *fpVol = popen("wpctl get-volume @DEFAULT_AUDIO_SINK@ | awk '{print $2}'", "r");
    if (fpVol) {
        if (fscanf(fpVol, "%lf", &fractionVol) != 1) {
            fractionVol = 0.0;
        }
        pclose(fpVol);
    }

    FILE *fp = popen("brightnessctl -m | awk -F',' '{print $4}'", "r");
    if (fp) {
        char percentStr[16] = {0};
        if (fgets(percentStr, sizeof(percentStr), fp)) {
            int percent = 0;
            if (sscanf(percentStr, "%d%%", &percent) == 1) {
                fractionBri = percent / 100.0;
            }
        }
        pclose(fp);
    }

    if (lastVol != fractionVol && strcmp(globalMode, "aud") == 0) {
        snprintf(statusText, sizeof(statusText), " %.0f%%", fractionVol * 100);
        updateProgress(fractionVol, statusText);
        lastVol = fractionVol;
        g_print(MAGENTA "[UPDATE]" RESET " Audio Volume: %.0f%%\n", fractionVol * 100);
    }

    if (lastBri != fractionBri && strcmp(globalMode, "bri") == 0) {
        snprintf(statusText, sizeof(statusText), "🔆 %.0f%%", fractionBri * 100);
        updateProgress(fractionBri, statusText);
        lastBri = fractionBri;
        g_print(MAGENTA "[UPDATE]" RESET " Brightness: %.0f%%\n", fractionBri * 100);
    }

    return G_SOURCE_CONTINUE;
}

static void onActivate(GtkApplication *app, gpointer userData) {

    Config config;
    char *path = find_path("conf.json");
    gboolean configLoaded = path ? catConf(path, &config) : FALSE;

    if (!configLoaded) {
        g_print(YELLOW "[WARN]" RESET " Could not load config file. Using defaults.\n");
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
    gtk_layer_set_exclusive_zone(GTK_WINDOW(globalWindow), -1);

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
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progressBar), FALSE);
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

    g_object_set_data(G_OBJECT(progressBar), "progress-label", label);

    gtk_box_append(GTK_BOX(box), overlay);
    gtk_window_present(GTK_WINDOW(globalWindow));

    g_timeout_add(100, updateStatus, NULL);
    resetTimer();
    g_print(GREEN "[INFO]" RESET " Application activated with mode: %s\n", globalMode);
}

static bool validateMode(char *mode) {
    const char *validArgs[] = {"aud", "bri", "mic"};
    const int validCount = sizeof(validArgs) / sizeof(validArgs[0]);
    int valid = 0;

    for (int i = 0; i < validCount; i++) {
        if (strcmp(mode, validArgs[i]) == 0) {
            valid = 1;
            break;
        }
    }

    if (!valid) {
        g_printerr(RED "[ERROR]" RESET " Invalid argument. Usage: [aud|bri|mic]\n");
        return FALSE;
    }
    return TRUE;
}

static int onCommandLine(GApplication *app, GApplicationCommandLine *cmdline, gpointer userData) {
    char **argv;
    int argc;
    argv = g_application_command_line_get_arguments(cmdline, &argc);
    if(globalMode == NULL) g_application_activate(app);

    if (globalMode) {
        g_free(globalMode);
    }
    globalMode = g_strdup(argv[1]);
    resetTimer();

    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2 && !validateMode(argv[1])) {
        g_printerr(RED "[ERROR]" RESET " Usage: %s [aud|bri|mic]\n", argv[0]);
        return 1;
    }
    GtkApplication *app = gtk_application_new("com.nit.echo-meter", G_APPLICATION_HANDLES_COMMAND_LINE);

    g_signal_connect(app, "command-line", G_CALLBACK(onCommandLine), argv[1]);
    g_signal_connect(app, "activate", G_CALLBACK(onActivate), argv[1]);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    if (globalMode) 
        g_free(globalMode);

    return status;
}

