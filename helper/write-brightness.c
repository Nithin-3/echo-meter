#include <stdio.h>
#include <stdlib.h>

static int catFile(const char *p, int *v) {
    FILE *f = fopen(p, "r");
    if (!f) return 0;
    int r = fscanf(f, "%d", v);
    fclose(f);
    return r == 1;
}
int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <backlight-dir> <value>\n", argv[0]);
        return 1;
    }

    int value = atoi(argv[2]), max = 0;
    char path[128];
    snprintf(path,sizeof(path),"/sys/class/backlight/%s/max_brightness",argv[1]);
    catFile(path, &max);
    snprintf(path, sizeof(path), "/sys/class/backlight/%s/brightness", argv[1]);
    if (value < 0 || value > max) return 1;
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("fopen");
        return 1;
    }

    if (fprintf(f, "%d\n", value) < 0) {
        perror("fprintf");
        fclose(f);
        return 1;
    }

    fclose(f);
    return 0;
}
