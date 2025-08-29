#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <dirent.h>
#include <poll.h>
#include <string.h>
#include <signal.h>
#include "echo-meter.h"
#define PRINT_KEY(key) puts(#key)
#define MAX_DEVICES 32
#define PATH_MAX_LEN 64

// Flag for graceful exit
static volatile sig_atomic_t running = 1;

// Signal handler for Ctrl+C
void handleSigint(int sig) { running = 0; }

int isKeyboard(int fd) {
    unsigned long evbit = 0;
    ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), &evbit);
    return evbit & (1 << EV_KEY);
}

int isON(int fd, int which) {
    unsigned long leds = 0;
    if (ioctl(fd, EVIOCGLED(sizeof(leds)), &leds) == -1) {
        perror("EVIOCGLED");
        return -1;
    }
    return !(leds & (1 << which));
}


int main() {
    struct pollfd fds[MAX_DEVICES];
    int num_fds = 0;
    DIR *dir = opendir("/dev/input");
    struct dirent *entry;
    char path[PATH_MAX_LEN];
    // Set up SIGINT handler
    signal(SIGINT, handleSigint);
    while ((entry = readdir(dir)) && num_fds < MAX_DEVICES) {
        if (strncmp(entry->d_name, "event", 5) == 0) {
            snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);
            int fd = open(path, O_RDONLY | O_NONBLOCK);
            if (fd >= 0 && isKeyboard(fd)) {
                fds[num_fds].fd = fd;
                fds[num_fds].events = POLLIN;
                num_fds++;
            } else close(fd);
        }
    }
    closedir(dir);
    struct input_event ev;
    while (running) {
        poll(fds, num_fds, -1);
        for (int i = 0; i < num_fds; i++) {
            if (fds[i].revents & POLLIN) {
                while (read(fds[i].fd, &ev, sizeof(ev)) == sizeof(ev)) {
                    if (ev.type != EV_KEY || ev.value != 1) continue;
                    switch (ev.code) {
                        case KEY_BRIGHTNESSUP: {
                            PRINT_KEY(KEY_BRIGHTNESSUP);
                            { char *argv[] = { "echo-meter", "bri", "+" }; invoke(3, argv); }
                            break;
                        }
                        case KEY_BRIGHTNESSDOWN: {
                            PRINT_KEY(KEY_BRIGHTNESSDOWN);
                            { char *argv[] = { "echo-meter", "bri", "-" }; invoke(3, argv); }
                            break;
                        }
                        case KEY_VOLUMEUP: {
                            PRINT_KEY(KEY_VOLUMEUP);
                            { char *argv[] = { "echo-meter", "aud", "+" }; invoke(3, argv); }
                            break;
                        }
                        case KEY_VOLUMEDOWN: {
                            PRINT_KEY(KEY_VOLUMEDOWN);
                            { char *argv[] = { "echo-meter", "aud", "-" }; invoke(3, argv); }
                            break;
                        }
                        case KEY_MUTE: {
                            PRINT_KEY(KEY_MUTE);
                            { char *argv[] = { "echo-meter", "aud" }; invoke(2, argv); }
                            break;
                        }
                        case KEY_MICMUTE: {
                            PRINT_KEY(KEY_MICMUTE);
                            { char *argv[] = { "echo-meter", "mic" }; invoke(2, argv); }
                            break;
                        }

                        case KEY_FN: { PRINT_KEY(KEY_FN); break; }
                        case KEY_FN_ESC: { PRINT_KEY(KEY_FN_ESC); break; }
                        case KEY_FN_F1: { PRINT_KEY(KEY_FN_F1); break; }
                        case KEY_FN_F2: { PRINT_KEY(KEY_FN_F2); break; }
                        case KEY_FN_F3: { PRINT_KEY(KEY_FN_F3); break; }
                        case KEY_FN_F4: { PRINT_KEY(KEY_FN_F4); break; }
                        case KEY_FN_F5: { PRINT_KEY(KEY_FN_F5); break; }
                        case KEY_FN_F6: { PRINT_KEY(KEY_FN_F6); break; }
                        case KEY_FN_F7: { PRINT_KEY(KEY_FN_F7); break; } 
                        case KEY_FN_F8: { PRINT_KEY(KEY_FN_F8); break; }
                        case KEY_FN_F9: { PRINT_KEY(KEY_FN_F9); break; }
                        case KEY_FN_F10: { PRINT_KEY(KEY_FN_F10); break; }
                        case KEY_FN_F11: { PRINT_KEY(KEY_FN_F11); break; }
                        case KEY_FN_F12: { PRINT_KEY(KEY_FN_F12); break; }

                        case KEY_CAPSLOCK: {
                            PRINT_KEY(KEY_CAPSLOCK);
                            printf("Caps Lock: %s\n", isON(fds[i].fd, LED_CAPSL) ? "ON" : "OFF");
                            break;
                        }
                        case KEY_NUMLOCK: {
                            PRINT_KEY(KEY_NUMLOCK);
                            printf("Num Lock: %s\n", isON(fds[i].fd, LED_NUML) ? "ON" : "OFF");
                            break;
                        }
                        case KEY_SCROLLLOCK: {
                            PRINT_KEY(KEY_SCROLLLOCK);
                            printf("Scroll Lock: %s\n", isON(fds[i].fd, LED_SCROLLL) ? "ON" : "OFF");
                            break;
                        }
                        case KEY_PLAYPAUSE: { PRINT_KEY(KEY_PLAYPAUSE); break; }
                        case KEY_STOPCD: { PRINT_KEY(KEY_STOPCD); break; }
                        case KEY_NEXTSONG: { PRINT_KEY(KEY_NEXTSONG); break; }
                        case KEY_PREVIOUSSONG: { PRINT_KEY(KEY_PREVIOUSSONG); break; }
                        case KEY_REWIND: { PRINT_KEY(KEY_REWIND); break; }
                        case KEY_FASTFORWARD: { PRINT_KEY(KEY_FASTFORWARD); break; }
                    }
                }
            }
        }
    }
    for (int i = 0; i < num_fds; i++) close(fds[i].fd);
    return 0;
}
