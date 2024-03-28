# Nombre del ejecutable
TARGET = main
# Directorio de salida
BIN_DIR = bin
LIB_DIR = lib
INCLUDE_DIR = include
SRC_DIR = src




all: $(BIN_DIR)/$(TARGET)


$(BIN_DIR)/$(TARGET): $(BIN_DIR) $(SRC_DIR)/main.c $(LIB_DIR)/gpio_control.c
	. /opt/poky/3.1.31/environment-setup-cortexa7t2hf-neon-vfpv4-poky-linux-gnueabi && \
	$(CC) -o $@ $(SRC_DIR)/main.c -L$(BIN_DIR) -lgpio_control

build: $(BIN_DIR)/libgpio_control.so


$(BIN_DIR):
	mkdir -p $(BIN_DIR)


$(BIN_DIR)/libgpio_control.so: $(LIB_DIR)/gpio_control.c | $(BIN_DIR)
	. /opt/poky/3.1.31/environment-setup-cortexa7t2hf-neon-vfpv4-poky-linux-gnueabi && \
	$(CC) -shared -fPIC -o $@ $<


clean:
	rm -rf $(BIN_DIR)

