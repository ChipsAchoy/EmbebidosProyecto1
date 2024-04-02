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
#define CHUNK_SIZE 1024

char* imageToBase64(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Failed to open the image file.\n");
        return NULL;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the image data
    unsigned char* image_data = (unsigned char*)malloc(file_size);
    if (image_data == NULL) {
        printf("Failed to allocate memory for the image data.\n");
        fclose(file);
        return NULL;
    }

    // Read the image data
    size_t bytes_read = fread(image_data, 1, file_size, file);
    fclose(file);

    if (bytes_read != file_size) {
        printf("Failed to read the image data.\n");
        free(image_data);
        return NULL;
    }

    // Encode the image data to base64 string
    const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t base64_size = ((file_size + 2) / 3) * 4; 
    char* base64_string = (char*)malloc(base64_size + 1);
    if (base64_string == NULL) {
        printf("Failed to allocate memory for the base64 string.\n");
        free(image_data);
        return NULL;
    }

    size_t i = 0, j = 0;
    unsigned char byte1, byte2, byte3;
    while (i < file_size) {
        byte1 = image_data[i++];
        byte2 = (i < file_size) ? image_data[i++] : 0;
        byte3 = (i < file_size) ? image_data[i++] : 0;

        base64_string[j++] = base64_chars[byte1 >> 2];
        base64_string[j++] = base64_chars[((byte1 & 0x03) << 4) | (byte2 >> 4)];
        base64_string[j++] = base64_chars[((byte2 & 0x0F) << 2) | (byte3 >> 6)];
        base64_string[j++] = base64_chars[byte3 & 0x3F];
    }

    // Add padding characters if necessary
    if (bytes_read % 3 == 1) {
        base64_string[base64_size - 1] = '=';
        base64_string[base64_size - 2] = '=';
    } else if (bytes_read % 3 == 2) {
        base64_string[base64_size - 1] = '=';
    }

    // Null-terminate the base64 string
    base64_string[base64_size] = '\0';

    // Free the image data
    free(image_data);

    return base64_string;
}


int main() {

    int in_pins[4] = {17, 27, 22, 23};
    int out_pins[5] = {24, 25, 16, 5, 6};

    // Exportar pines GPIO
    for (int i = 0; i < 5; i++) {
        if (export_gpio(out_pins[i]) == -1) {
            return 1;
        }
        if (set_direction(out_pins[i], "out") == -1) {
            return 1;
        }
    }
    for (int i = 0; i < 4; i++) {
        if (export_gpio(in_pins[i]) == -1) {
            return 1;
        }
        if (set_direction(in_pins[i], "in") == -1) {
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

    int send_buff_size = 131072; // Set your desired buffer size here
    if (setsockopt(server_fd, SOL_SOCKET, SO_SNDBUF, &send_buff_size, sizeof(send_buff_size)) == -1) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
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
        for (int i = 0; i < 5; i++) {
            if (buffer[i] == '1') {
                write_gpio(out_pins[i], 1);
            } else if (buffer[i] == '0'){
                write_gpio(out_pins[i], 0);
            }
        }
        
        // Enviar el estado de las entradas GPIO 15 a 18 si se solicita
        if (strcmp(buffer, "get_inputs") == 0) {
            char input_states[5] = {0};
            for (int i = 0; i < 4; i++) {
                input_states[i] = read_gpio(in_pins[i]) + '0';
            }
            send(new_socket, input_states, strlen(input_states), 0);
        }

        // Enviar la imagen
        printf("Buffer: %s\n", buffer);
        if (strcmp(buffer, "get_images") == 0) {
            
            system("fswebcam -r 400x400 imgs/image.png");
            
            const char* filename = "imgs/image.png";
            char* base64_string = imageToBase64(filename);
            
            // Calculate the length of the string
                int length = strlen(base64_string);

            if (base64_string != NULL) {
                printf("Base64 string: %s\n", base64_string);
                char * string_corrected = (char *) malloc((length+1024) * sizeof(char));
                for (int i = 0; i < 1024; i++) {
                    string_corrected[i] = '0';
                }

                for (int i = 1024; i < length; i++) {
                    string_corrected[i] = base64_string[i-1024];
                }

                
                // Send the string in chunks
                int sent_bytes = 0;
                while (sent_bytes < length) {
                    int remaining_bytes = length - sent_bytes;
                    int chunk_size = remaining_bytes < CHUNK_SIZE ? remaining_bytes : CHUNK_SIZE;
                    
                    // Send chunk
                    if (send(new_socket, string_corrected+sent_bytes, chunk_size, 0) == -1) {
                        perror("Send failed");
                        exit(EXIT_FAILURE);
                    }

                    sent_bytes += chunk_size;
                }

                printf("String sent successfully.\n");
                
                free(base64_string);
            }
            else {
                send(new_socket, "Error al convertir la imagen a base64", 36, 0);
            }
            
        }

        // Cerrar el socket
        close(new_socket);
    }

    return 0;
}
