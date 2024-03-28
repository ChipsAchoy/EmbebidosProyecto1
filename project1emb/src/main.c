#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../include/gpio_control.h"


#define PORT 8765
#define BUF_SIZE 1024

int main() {
    // Exportar pines GPIO
    for (int i = 20; i <= 24; i++) {
        if (export_gpio(i) == -1) {
            return 1;
        }
        if (set_direction(i, "out") == -1) {
            return 1;
        }
    }
    for (int i = 15; i <= 18; i++) {
        if (export_gpio(i) == -1) {
            return 1;
        }
        if (set_direction(i, "in") == -1) {
            return 1;
        }
    }

    // Crear socket
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUF_SIZE] = {0};

    // Crear socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error al crear el socket");
        return 1;
    }

    // Configurar opciones del socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Error al configurar el socket");
        return 1;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Vincular el socket al puerto
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error al vincular el socket al puerto");
        return 1;
    }

    // Escuchar en el puerto
    if (listen(server_fd, 3) < 0) {
        perror("Error al escuchar en el puerto");
        return 1;
    }

    while (1) {
        // Aceptar la conexión entrante
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Error al aceptar la conexión");
            return 1;
        }

        // Leer el mensaje del socket
        valread = read(new_socket, buffer, BUF_SIZE);
        if (valread < 0) {
            perror("Error al leer del socket");
            close(new_socket);
            continue;
        }

        // Controlar las salidas GPIO según el mensaje recibido
        for (int i = 20; i <= 24; i++) {
            if (buffer[i - 20] == '1') {
                write_gpio(i, 1);
            } else {
                write_gpio(i, 0);
            }
        }

        // Enviar el estado de las entradas GPIO 15 a 18 si se solicita
        if (strcmp(buffer, "get_inputs") == 0) {
            char input_states[5] = {0};
            for (int i = 15; i <= 18; i++) {
                input_states[i - 15] = read_gpio(i) + '0';
            }
            send(new_socket, input_states, strlen(input_states), 0);
        }

        // Cerrar el socket
        close(new_socket);
    }

    return 0;
}
