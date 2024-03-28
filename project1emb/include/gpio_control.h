#ifndef GPIO_CONTROL_H
#define GPIO_CONTROL_H

// Función para exportar el pin GPIO
int export_gpio(int pin);

// Función para configurar la dirección del pin GPIO
int set_direction(int pin, const char *direction);

// Función para escribir en el pin GPIO
int write_gpio(int pin, int value);

// Función para leer el valor del pin GPIO
int read_gpio(int pin);

// Función para realizar el parpadeo del pin GPIO
void blink_gpio(int pin, int num_times, int delay_ms);

#endif /* GPIO_CONTROL_H */

