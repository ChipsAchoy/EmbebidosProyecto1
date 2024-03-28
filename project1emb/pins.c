#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

// Función para exportar el pin GPIO
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

// Función para configurar la dirección del pin GPIO
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

// Función para escribir en el pin GPIO
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

// Función para leer el valor del pin GPIO
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

// Función para realizar el parpadeo del pin GPIO
void blink_gpio(int pin, int num_times, int delay_ms) {
    for (int i = 0; i < num_times; i++) {
        write_gpio(pin, 1);
        usleep(delay_ms * 1000);
        write_gpio(pin, 0);
        usleep(delay_ms * 1000);
    }
}

int main() {
    int gpio_pin = 16; // Definir el número del pin GPIO que deseas controlar
    
    // Exportar el pin GPIO
    if (export_gpio(gpio_pin) == -1) {
        return 1;
    }
    
    // Configurar la dirección del pin GPIO como salida
    if (set_direction(gpio_pin, "out") == -1) {
        return 1;
    }
    
    // Realizar un parpadeo del pin GPIO
    blink_gpio(gpio_pin, 5, 500); // Parpadear 5 veces con un intervalo de 500 ms
    
    // Desexportar el pin GPIO
    int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd != -1) {
        char gpio_pin_str[4];
        sprintf(gpio_pin_str, "%d", gpio_pin);
        if (write(fd, gpio_pin_str, strlen(gpio_pin_str)) != strlen(gpio_pin_str)) {
            perror("Error al escribir en /sys/class/gpio/unexport");
        }
        close(fd);
    } else {
        perror("Error al abrir /sys/class/gpio/unexport");
    }
    
    return 0;
}

