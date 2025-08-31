#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <dirent.h>
#include <poll.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
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

void* echoMeter(void* cmd) {
    system((char*) cmd);
    return NULL;
}

void runCommand(const char* cmd) {
    pthread_t thread;
    // strdup() so each thread gets its own copy of cmd
    char* arg = strdup(cmd);
    if (pthread_create(&thread, NULL, echoMeter, arg) == 0) {
        pthread_detach(thread); // auto-cleanup thread (no join needed)
    } else {
        perror("pthread_create failed");
        free(arg);
    }
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
                        case KEY_BRIGHTNESSUP: runCommand("echo-meter bri +"); break;
                        case KEY_BRIGHTNESSDOWN: runCommand("echo-meter bri -"); break;
                        case KEY_VOLUMEUP: runCommand("echo-meter aud +"); break;
                        case KEY_VOLUMEDOWN: runCommand("echo-meter aud -"); break;
                        case KEY_MUTE: runCommand("echo-meter mut"); break;
                        case KEY_MICMUTE: runCommand("echo-meter micmut"); break;
                        case KEY_CAPSLOCK: runCommand(isON(fds[i].fd, LED_CAPSL)?"echo-meter capon":"echo-meter cap"); break;
                        case KEY_NUMLOCK:  runCommand(isON(fds[i].fd, LED_NUML)?"echo-meter numon":"echo-meter num"); break;
                        case KEY_SCROLLLOCK: runCommand(isON(fds[i].fd, LED_SCROLLL)?"echo-meter scron":"echo-meter scr"); break;
                        case KEY_FN: {  break; }
                        case KEY_FN_ESC: {  break; }
                        case KEY_FN_F1: {  break; }
                        case KEY_FN_F2: {  break; }
                        case KEY_FN_F3: {  break; }
                        case KEY_FN_F4: {  break; }
                        case KEY_FN_F5: {  break; }
                        case KEY_FN_F6: {  break; }
                        case KEY_FN_F7: {  break; } 
                        case KEY_FN_F8: {  break; }
                        case KEY_FN_F9: {  break; }
                        case KEY_FN_F10: {  break; }
                        case KEY_FN_F11: {  break; }
                        case KEY_FN_F12: {  break; }
                        case KEY_PLAYPAUSE: {  break; }
                        case KEY_STOPCD: {  break; }
                        case KEY_NEXTSONG: {  break; }
                        case KEY_PREVIOUSSONG: {  break; }
                        case KEY_REWIND: {  break; }
                        case KEY_FASTFORWARD: {  break; }
                    }
                }
            }
        }
    }
    for (int i = 0; i < num_fds; i++) close(fds[i].fd);
    return 0;
}
