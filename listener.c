#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <dirent.h>
#include <poll.h>
#include <string.h>
#include <sys/ioctl.h>

#define MAX_DEVICES 32

int is_keyboard(int fd) {
    unsigned long evbit = 0;
    ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), &evbit);
    return evbit & (1 << EV_KEY);
}

int main() {
    struct pollfd fds[MAX_DEVICES];
    int num_fds = 0;
    DIR *dir = opendir("/dev/input");
    struct dirent *entry;
    char path[64];
    while ((entry = readdir(dir)) && num_fds < MAX_DEVICES) {
        if (strncmp(entry->d_name, "event", 5) == 0) {
            snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);
            int fd = open(path, O_RDONLY | O_NONBLOCK);
            if (fd >= 0 && is_keyboard(fd)) {
                fds[num_fds].fd = fd;
                fds[num_fds].events = POLLIN;
                num_fds++;
            } else close(fd);
        }
    }
    closedir(dir);

    struct input_event ev;
    while (1) {
        poll(fds, num_fds, -1);
        for (int i = 0; i < num_fds; i++) {
            if (fds[i].revents & POLLIN) {
                while (read(fds[i].fd, &ev, sizeof(ev)) == sizeof(ev)) {
                    if (ev.type != EV_KEY || ev.value != 1) continue;
                    switch (ev.code) {
                        case KEY_BRIGHTNESSUP:    puts("KEY_BRIGHTNESSUP"); break;
                        case KEY_BRIGHTNESSDOWN:  puts("KEY_BRIGHTNESSDOWN"); break;
                        case KEY_VOLUMEUP:        puts("KEY_VOLUMEUP"); break;
                        case KEY_VOLUMEDOWN:      puts("KEY_VOLUMEDOWN"); break;
                        case KEY_MUTE:            puts("KEY_MUTE"); break;
                        case KEY_MICMUTE:         puts("KEY_MICMUTE"); break;
                        case KEY_FN:              puts("KEY_FN"); break;
                        case KEY_FN_ESC:          puts("KEY_FN_ESC"); break;
                        case KEY_FN_F1:           puts("KEY_FN_F1"); break;
                        case KEY_FN_F2:           puts("KEY_FN_F2"); break;
                        case KEY_FN_F3:           puts("KEY_FN_F3"); break;
                        case KEY_FN_F4:           puts("KEY_FN_F4"); break;
                        case KEY_FN_F5:           puts("KEY_FN_F5"); break;
                        case KEY_FN_F6:           puts("KEY_FN_F6"); break;
                        case KEY_FN_F7:           puts("KEY_FN_F7"); break;
                        case KEY_FN_F8:           puts("KEY_FN_F8"); break;
                        case KEY_FN_F9:           puts("KEY_FN_F9"); break;
                        case KEY_FN_F10:          puts("KEY_FN_F10"); break;
                        case KEY_FN_F11:          puts("KEY_FN_F11"); break;
                        case KEY_FN_F12:          puts("KEY_FN_F12"); break;
                        case KEY_CAPSLOCK:        puts("KEY_CAPSLOCK"); break;
                        case KEY_NUMLOCK:         puts("KEY_NUMLOCK"); break;
                        case KEY_SCROLLLOCK:      puts("KEY_SCROLLLOCK"); break;
                        case KEY_PLAYPAUSE:       puts("KEY_PLAYPAUSE"); break;
                        case KEY_STOPCD:          puts("KEY_STOPCD"); break;
                        case KEY_NEXTSONG:        puts("KEY_NEXTSONG"); break;
                        case KEY_PREVIOUSSONG:    puts("KEY_PREVIOUSSONG"); break;
                        case KEY_REWIND:          puts("KEY_REWIND"); break;
                        case KEY_FASTFORWARD:     puts("KEY_FASTFORWARD"); break;
                    }
                }
            }
        }
    }

    for (int i = 0; i < num_fds; i++) close(fds[i].fd);
    return 0;
}

