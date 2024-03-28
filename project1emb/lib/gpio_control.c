#include "../include/gpio_control.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int export_gpio(int pin) {
    char gpio_pin[4];
    sprintf(gpio_pin, "%d", pin);
    int fd;
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1) {
        perror("Error al abrir /sys/class/gpio/export");
        return -1;
    }
    if (write(fd, gpio_pin, strlen(gpio_pin)) != strlen(gpio_pin)) {
        perror("Error al escribir en /sys/class/gpio/export");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int set_direction(int pin, const char *direction) {
    char path[50];
    sprintf(path, "/sys/class/gpio/gpio%d/direction", pin);
    int fd = open(path, O_WRONLY);
    if (fd == -1) {
        perror("Error al abrir archivo de dirección");
        return -1;
    }
    if (write(fd, direction, strlen(direction)) != strlen(direction)) {
        perror("Error al escribir en archivo de dirección");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int write_gpio(int pin, int value) {
    char path[50];
    sprintf(path, "/sys/class/gpio/gpio%d/value", pin);
    int fd = open(path, O_WRONLY);
    if (fd == -1) {
        perror("Error al abrir archivo de valor");
        return -1;
    }
    if (write(fd, (value == 0) ? "0" : "1", 1) != 1) {
        perror("Error al escribir en archivo de valor");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int read_gpio(int pin) {
    char path[50];
    char value;
    sprintf(path, "/sys/class/gpio/gpio%d/value", pin);
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Error al abrir archivo de valor");
        return -1;
    }
    if (read(fd, &value, 1) != 1) {
        perror("Error al leer archivo de valor");
        close(fd);
        return -1;
    }
    close(fd);
    return (value == '0') ? 0 : 1;
}

void blink_gpio(int pin, int num_times, int delay_ms) {
    for (int i = 0; i < num_times; i++) {
        write_gpio(pin, 1);
        usleep(delay_ms * 1000);
        write_gpio(pin, 0);
        usleep(delay_ms * 1000);
    }
}

