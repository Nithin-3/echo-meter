# Compiler
CC       = gcc
CFLAGS   = -Wall -g -O2 `pkg-config --cflags gtk4 gtk4-layer-shell-0 json-glib-1.0`
LDFLAGS  = `pkg-config --libs gtk4 gtk4-layer-shell-0 json-glib-1.0` -lasound

# Directories
SRC_DIR      = src
HELPER_DIR   = helper
OBJ_DIR      = obj
ASSETS_DIR   = assets
INSTALL_DIR  = /usr/share/echo-meter
CONFIG_DIR   = $(shell getent passwd $(SUDO_USER) 2>/dev/null | cut -d: -f6 || echo $(HOME))/.config/echo-meter
BIN_DIR      = /usr/bin

# Targets
MAIN_TARGET     = echo-meter
HELPER_TARGET   = write-brightness
LISTENER_TARGET = echolis

# Sources
MAIN_SOURCES   = $(SRC_DIR)/echo-meter.c $(SRC_DIR)/conf.c $(SRC_DIR)/tool.c
HELPER_SOURCES = $(HELPER_DIR)/write-brightness.c
LISTENER_SOURCE = listener.c

# Objects
MAIN_OBJECTS   = $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(MAIN_SOURCES)))
HELPER_OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(HELPER_SOURCES)))

# Quiet flag
Q = @

# Default target
all: $(MAIN_TARGET) $(HELPER_TARGET) $(LISTENER_TARGET)

# Build main executable
$(MAIN_TARGET): $(MAIN_OBJECTS)
	$(Q)$(CC) $(MAIN_OBJECTS) -o $@ $(LDFLAGS)

# Build helper executable
$(HELPER_TARGET): $(HELPER_OBJECTS)
	$(Q)$(CC) $(HELPER_OBJECTS) -o $@ 

# Build listener executable
$(LISTENER_TARGET): $(LISTENER_SOURCE)
	$(Q)$(CC) $(CFLAGS) -Iinclude $< -o $@

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(Q)$(CC) $(CFLAGS) -Iinclude -c $< -o $@

$(OBJ_DIR)/%.o: $(HELPER_DIR)/%.c | $(OBJ_DIR)
	$(Q)$(CC) $(CFLAGS) -Iinclude -c $< -o $@

# Ensure object directory exists
$(OBJ_DIR):
	$(Q)mkdir -p $(OBJ_DIR)

# Install targets
install: $(MAIN_TARGET) $(HELPER_TARGET) $(LISTENER_TARGET)
	# Install main program
	$(Q)install -d $(BIN_DIR)
	$(Q)install -m 755 $(MAIN_TARGET) $(BIN_DIR)/$(MAIN_TARGET)

	# Install listener with setuid root
	$(Q)install -m 4755 -o root -g root $(LISTENER_TARGET) $(BIN_DIR)/$(LISTENER_TARGET)

	# Install helper with setuid root
	$(Q)install -d $(INSTALL_DIR)
	$(Q)install -m 4755 -o root -g root $(HELPER_TARGET) $(INSTALL_DIR)/$(HELPER_TARGET)

	# Install assets (system-wide)
	$(Q)install -m 644 $(ASSETS_DIR)/conf.json $(INSTALL_DIR)/
	$(Q)install -m 644 $(ASSETS_DIR)/style.css $(INSTALL_DIR)/

	# Install assets (per-user config)
	$(Q)install -d $(CONFIG_DIR)
	$(Q)install -m 644 $(ASSETS_DIR)/conf.json $(CONFIG_DIR)/
	$(Q)install -m 644 $(ASSETS_DIR)/style.css $(CONFIG_DIR)/

# Uninstall (keeps user configs)
uninstall:
	$(Q)rm -f $(BIN_DIR)/$(MAIN_TARGET)
	$(Q)rm -f $(BIN_DIR)/$(LISTENER_TARGET)
	$(Q)rm -f $(INSTALL_DIR)/$(HELPER_TARGET)
	$(Q)rm -f $(INSTALL_DIR)/conf.json
	$(Q)rm -f $(INSTALL_DIR)/style.css
	$(Q)rmdir --ignore-fail-on-non-empty $(INSTALL_DIR) 2>/dev/null || true

# Clean build artifacts
clean:
	$(Q)rm -rf $(OBJ_DIR) $(MAIN_TARGET) $(HELPER_TARGET) $(LISTENER_TARGET)

.PHONY: all install uninstall clean

