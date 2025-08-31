#include <gtk/gtk.h>
#include <string.h>
#include <gtk-layer-shell/gtk-layer-shell.h>
#include "conf.h"
#include "glibconfig.h"
#include "path.h"
#include "tool.h"
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

GtkWidget *globalWindow = NULL;
GtkWidget *slider = NULL;
static float lastChange = -1.1;
static char lastIcon[32] = "";
static guint timeoutId = 0;
static Type globalMode = INVALIDT;

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

static void resetTimer(int sec) {
    if (timeoutId != 0) {
        g_source_remove(timeoutId);
        timeoutId = 0;
    }
    timeoutId = g_timeout_add_seconds(sec, onClose, NULL);
    g_print(CYAN "[TIMER]" RESET " New auto-close timer set to %d seconds.\n",sec);
}

static void updateProgress(double val, const char *txt) {
    if (slider != NULL) {
        gtk_range_set_value(GTK_RANGE(slider), val);
        GtkWidget *label = g_object_get_data(G_OBJECT(slider), "progress-label");
        if (label) {
            gtk_label_set_text(GTK_LABEL(label), txt);
        }
    }
}

static const char* icon(Type which, const Config *conf) {
    switch (which) {
        case BRI:
            return conf->icon.brightness;

        case MIC:
            return getMicMute() ? conf->icon.micOff : conf->icon.mic;

        case AUD:
            return getMute() ? conf->icon.mute : conf->icon.sound;
        case CAP_ON:
            return "[CAP ON]"; // placeholder

        case NUM_ON:
            return "[NUM ON]"; // placeholder

        case SCR_ON:
            return "[SCR ON]"; // placeholder

        case CAP:
            return "[CAP OFF]"; // placeholder

        case NUM:
            return "[NUM OFF]"; // placeholder

        case SCR:
            return "[SCR OFF]"; // placeholder

        case INVALIDT:
        default:
            return ""; // safe fallback
    }
}

static gboolean updateStatus(gpointer userData) {
    float fraction = getVal(globalMode);
    const char *ico = icon(globalMode, (Config *)userData);
    if ((globalMode == INVALIDT || lastChange == fraction) && strcmp(ico, lastIcon) == 0) return G_SOURCE_CONTINUE;
    if (fraction < 0.0f) {
        GtkWidget *label = g_object_get_data(G_OBJECT(slider), "progress-label");
        if (label) gtk_label_set_text(GTK_LABEL(label), ico);
        if (gtk_widget_get_visible(slider)) gtk_widget_set_visible(slider,FALSE);
        return G_SOURCE_CONTINUE;
    }
    if (slider && !gtk_widget_get_visible(slider)) gtk_widget_set_visible(slider,TRUE);
    char statusText[128] = "";
    snprintf(statusText, sizeof(statusText), "%s %.0f%%", ico, fraction * 100);
    updateProgress(fraction, statusText);
    lastChange = fraction;
    snprintf(lastIcon, sizeof(lastIcon), "%s", ico);
    g_print(MAGENTA "[UPDATE]" RESET " : %.0f%%\n", fraction * 100);

    return G_SOURCE_CONTINUE;
}


static void sliderValChang(GtkRange *range, gpointer userData) {
    Config* config = (Config *) userData;
    float value = gtk_range_get_value(range);
    setVal(globalMode,value);
    resetTimer(config->timeout);
}
static void onActivate(GtkApplication *app, gpointer userData) {

    Config* config = (Config *) userData;
    globalWindow = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(globalWindow), "Indicator");
    gtk_window_set_resizable(GTK_WINDOW(globalWindow), FALSE);
    g_signal_connect(globalWindow, "destroy", G_CALLBACK(onDestroy), NULL);

    gtk_layer_init_for_window(GTK_WINDOW(globalWindow));
    gtk_layer_set_layer(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_exclusive_zone(GTK_WINDOW(globalWindow), -1);

    if (config->hasExplicitPos) {
        gtk_layer_set_anchor(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
        gtk_layer_set_anchor(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
        gtk_layer_set_margin(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_TOP, config->y);
        gtk_layer_set_margin(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_LEFT, config->x);
    } else {
        gtk_layer_set_anchor(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_TOP, strcmp(config->verticalAlign, "top") == 0);
        gtk_layer_set_anchor(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_BOTTOM, strcmp(config->verticalAlign, "bottom") == 0);
        gtk_layer_set_anchor(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_LEFT, strcmp(config->horizontalAlign, "left") == 0);
        gtk_layer_set_anchor(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_RIGHT, strcmp(config->horizontalAlign, "right") == 0);
        gtk_layer_set_margin(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_TOP, config->margin);
        gtk_layer_set_margin(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_BOTTOM, config->margin);
        gtk_layer_set_margin(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_LEFT, config->margin);
        gtk_layer_set_margin(GTK_WINDOW(globalWindow), GTK_LAYER_SHELL_EDGE_RIGHT, config->margin);
    }

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, findPath("style.css"));
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    GtkOrientation orient = GTK_ORIENTATION_HORIZONTAL;
    if (strcmp(config->orientation, "vertical") == 0) {
        orient = GTK_ORIENTATION_VERTICAL;
    }

    GtkWidget *box = gtk_box_new(orient, 10);
    gtk_window_set_child(GTK_WINDOW(globalWindow), box);

    slider = gtk_scale_new_with_range(orient, 0.0, 1.0, 0.01);
    gtk_scale_set_draw_value(GTK_SCALE(slider), FALSE);
    gtk_widget_set_name(slider, "status-slider");
    g_signal_connect(slider, "value-changed", G_CALLBACK(sliderValChang), config);
    GtkWidget *label = gtk_label_new("");

    gtk_widget_set_name(label, "progress-label");
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(label, GTK_ALIGN_CENTER);

    if (orient == GTK_ORIENTATION_VERTICAL) {
        gtk_widget_set_size_request(slider, 25, 100);
        gtk_range_set_inverted(GTK_RANGE(slider), !config->invertDirection);
        gtk_box_append(GTK_BOX(box), slider);
        gtk_box_append(GTK_BOX(box), label);
    }else{
        gtk_widget_set_size_request(slider, 100, 25);
        gtk_range_set_inverted(GTK_RANGE(slider), config->invertDirection);
        gtk_box_append(GTK_BOX(box), slider);
        gtk_box_append(GTK_BOX(box), label);
    }


    g_object_set_data(G_OBJECT(slider), "progress-label", label);

    gtk_window_present(GTK_WINDOW(globalWindow));

    g_print(GREEN "[INFO]" RESET " Application activated \n");
    g_timeout_add(200, updateStatus, config);
    resetTimer(config->timeout);
}

bool validate_percentage(const char *s) {
    char *end;
    long val = strtol(s, &end, 10);

    if (*end != '\0') return false;
    if (val < 0 || val > 100) return false;
    return true;
}

bool validate_args(int argc, char *argv[]) {
    if (argc < 2) return false;
    if (parseType(argv[1]) == INVALIDT) return false;
    float value = 0.0;
    int dir = -1;
    if (argc >= 3) {
        if (strcmp(argv[2], "+") == 0) {
            dir = 1;
        } else if (strcmp(argv[2], "-") == 0) {
            dir = 0;
        } else {
            printf("Error: second argument must be '+' or '-'\n");
            return false;
        }
    }
    if (argc >= 4) {
        if (!validate_percentage(argv[3])) {
            printf("Error: third argument must be an integer 0-100\n");
            return false;
        }
        value = atof(argv[3]);
    }
    if (dir != -1) step(parseType(argv[1]), dir, value);
    return true;
}

static int onCommandLine(GApplication *app, GApplicationCommandLine *cmdline, gpointer userData) {
    char **argv;
    int argc;
    argv = g_application_command_line_get_arguments(cmdline, &argc);

    if (!validate_args(argc, argv)) {
        printf("Usage: %s [aud|mic|bri] (+|-) (0-100)\n", argv[0]);
        return 1;
    }
    if (strcmp(argv[1], "mut") == 0) setMute(getMute());
    if (strcmp(argv[1], "micmut") == 0) setMicMute(getMicMute());
    if(globalMode == INVALIDT) g_application_activate(app);
    globalMode = parseType(argv[1]);

    return 0;
}
int main(int argc, char **argv) {

    Config *config = g_new0(Config,1);
    char *path = findPath("conf.json");
    gboolean configLoaded = path ? catConf(path, config) : FALSE;

    if (!configLoaded) {
        g_print(YELLOW "[WARN]" RESET " Could not load config file. Using defaults.\n");
        strcpy(config->orientation, "horizontal");
        config->hasExplicitPos = false;
        strcpy(config->verticalAlign, "top");
        strcpy(config->horizontalAlign, "center");
        config->margin = 0;
    }
    GtkApplication *app = gtk_application_new("com.nit.echo-meter", G_APPLICATION_HANDLES_COMMAND_LINE);

    g_signal_connect(app, "command-line", G_CALLBACK(onCommandLine), config);
    g_signal_connect(app, "activate", G_CALLBACK(onActivate), config);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);


    return status;
}

