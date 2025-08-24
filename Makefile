# Compiler
CC = gcc

# Compiler flags: warnings, debug info, and pkg-config cflags for GTK4 + others
CFLAGS = -Wall -Wextra -g `pkg-config --cflags gtk4 gtk4-layer-shell-0 json-glib-1.0`

# Linker flags: pkg-config libs for GTK4 + others
LDFLAGS = `pkg-config --libs gtk4 gtk4-layer-shell-0 json-glib-1.0`

# Include directory for headers
INCLUDES = -Iinclude

# Directories
SRC_DIR = src
OBJ_DIR = obj
ASSETS_DIR = assets

# Final binary name
BIN = echo-meter

# Install destinations
USER_CONFIG_DIR = $(HOME)/.config/echo-meter
SYSTEM_SHARE_DIR = /usr/share/echo-meter

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)

# Object files corresponding to source files in obj/
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

# Default target: build binary
$(BIN): $(OBJECTS)
	@echo "Linking $@ ..."
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $@"

# Compile source files to object files, creating obj/ if needed
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "Compiling $< ..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Create obj directory if it doesn't exist
$(OBJ_DIR):
	@mkdir -p $@

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(OBJ_DIR) $(BIN)
	@echo "Clean complete."

# Run the binary (build first if needed)
.PHONY: run
run: $(BIN)
	@echo "Running $(BIN)..."
	./$(BIN)

# Install assets and binary
.PHONY: install
install: $(BIN)
	@echo "Installing binary to /usr/local/bin/ ..."
	install -Dm755 $(BIN) /usr/local/bin/$(BIN)
	@echo "Installing assets to user config directory: $(USER_CONFIG_DIR)"
	mkdir -p $(USER_CONFIG_DIR)
	cp -r $(ASSETS_DIR)/* $(USER_CONFIG_DIR)/
	@echo "Installing assets to system directory: $(SYSTEM_SHARE_DIR) (may require sudo)"
	mkdir -p $(SYSTEM_SHARE_DIR)
	cp -r $(ASSETS_DIR)/* $(SYSTEM_SHARE_DIR)/
	@echo "Installation complete."

# Uninstall binary and assets
.PHONY: uninstall
uninstall:
	@echo "Removing binary from $(SYSTEM_BIN_DIR)/$(BIN) (may require sudo)"
	rm -f $(SYSTEM_BIN_DIR)/$(BIN)
	@echo "Removing system assets directory $(SYSTEM_SHARE_DIR) (may require sudo)"
	rm -rf $(SYSTEM_SHARE_DIR)
	@echo "Removing user config directory $(USER_CONFIG_DIR)"
	rm -rf $(USER_CONFIG_DIR)
	@echo "Uninstallation complete."
