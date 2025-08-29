# Compiler to use
CC = gcc

# Compiler flags
CFLAGS = -Wall -g -O2 `pkg-config --cflags gtk4 gtk4-layer-shell-0 json-glib-1.0`

# Linker flags
LDFLAGS = `pkg-config --libs gtk4 gtk4-layer-shell-0 json-glib-1.0` -lasound

# Directories
SRC_DIR = src
HELPER_DIR = helper
OBJ_DIR = obj
ASSETS_DIR = assets
INSTALL_DIR = /usr/share/echo-meter
CONFIG_DIR = $(HOME)/.config/echo-meter
BIN_DIR = /usr/bin

# Target executables
MAIN_TARGET = echo-meter
HELPER_TARGET = write-brightness

# Source files
MAIN_SOURCES = $(SRC_DIR)/echo-meter.c $(SRC_DIR)/conf.c $(SRC_DIR)/tool.c
HELPER_SOURCES = $(HELPER_DIR)/write-brightness.c

# Object files
MAIN_OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(MAIN_SOURCES)))
HELPER_OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(HELPER_SOURCES)))

# Default target
all: $(MAIN_TARGET) $(HELPER_TARGET)
	@echo "Build completed: $(MAIN_TARGET) and $(HELPER_TARGET)"

# Link main executable
$(MAIN_TARGET): $(MAIN_OBJECTS)
	@echo "Linking $(MAIN_TARGET) from $(MAIN_OBJECTS)"
	$(CC) $(MAIN_OBJECTS) -o $@ $(LDFLAGS)
	@echo "Created executable: $(MAIN_TARGET)"

# Link helper executable
$(HELPER_TARGET): $(HELPER_OBJECTS)
	@echo "Linking $(HELPER_TARGET) from $(HELPER_OBJECTS)"
	$(CC) $(HELPER_OBJECTS) -o $@ $(LDFLAGS)
	@echo "Created executable: $(HELPER_TARGET)"

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "Compiling $< to $@"
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

$(OBJ_DIR)/%.o: $(HELPER_DIR)/%.c | $(OBJ_DIR)
	@echo "Compiling $< to $@"
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

# Create obj directory
$(OBJ_DIR):
	@echo "Creating object directory: $(OBJ_DIR)"
	mkdir -p $(OBJ_DIR)

# Install targets
install: $(MAIN_TARGET) $(HELPER_TARGET)
	@echo "Installing executables and assets..."
	# Install main executable to /usr/bin
	@echo "Installing $(MAIN_TARGET) to $(BIN_DIR)"
	install -d $(BIN_DIR)
	install -m 755 $(MAIN_TARGET) $(BIN_DIR)/$(MAIN_TARGET)
	# Install helper executable with setuid
	@echo "Creating directory: $(INSTALL_DIR)"
	install -d $(INSTALL_DIR)
	@echo "Installing $(HELPER_TARGET) to $(INSTALL_DIR) with setuid"
	install -m 4755 -o root -g root $(HELPER_TARGET) $(INSTALL_DIR)/$(HELPER_TARGET)
	# Install assets to system and user config directories
	@echo "Installing assets to $(INSTALL_DIR)"
	install -d $(INSTALL_DIR)
	install -m 644 $(ASSETS_DIR)/conf.json $(INSTALL_DIR)/
	@echo "Installed $(ASSETS_DIR)/conf.json to $(INSTALL_DIR)"
	install -m 644 $(ASSETS_DIR)/style.css $(INSTALL_DIR)/
	@echo "Installed $(ASSETS_DIR)/style.css to $(INSTALL_DIR)"
	@echo "Installing assets to $(CONFIG_DIR)"
	install -d $(CONFIG_DIR)
	install -m 644 $(ASSETS_DIR)/conf.json $(CONFIG_DIR)/
	@echo "Installed $(ASSETS_DIR)/conf.json to $(CONFIG_DIR)"
	install -m 644 $(ASSETS_DIR)/style.css $(CONFIG_DIR)/
	@echo "Installed $(ASSETS_DIR)/style.css to $(CONFIG_DIR)"
	@echo "Installation completed"

# Clean up
clean:
	@echo "Cleaning up object files and executables..."
	rm -rf $(OBJ_DIR) $(MAIN_TARGET) $(HELPER_TARGET)
	@echo "Clean completed"

# Phony targets
.PHONY: all install clean
