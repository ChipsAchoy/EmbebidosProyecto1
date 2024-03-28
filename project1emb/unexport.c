#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_GPIO_NUM 27 // Cambia este valor según la cantidad máxima de pines GPIO que quieras liberar

int main() {
    int fd, i;
    char buf[MAX_GPIO_NUM];

    // Abrir el archivo de unexport
    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd == -1) {
        perror("Error al abrir el archivo de unexport");
        return 1;
    }

    // Escribir los números de pin de los GPIO que quieres liberar
    for (i = 0; i < MAX_GPIO_NUM; i++) {
        sprintf(buf, "%d", i);
        write(fd, buf, sizeof(buf));
    }

    // Cerrar el archivo de unexport
    close(fd);

    printf("Todos los pines GPIO han sido liberados.\n");

    return 0;
}

