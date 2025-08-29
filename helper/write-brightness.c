#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <backlight-dir> <value>\n", argv[0]);
        return 1;
    }

    int value = atoi(argv[2]);
    char path[128];
    snprintf(path, sizeof(path), "/sys/class/backlight/%s/brightness", argv[1]);

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

