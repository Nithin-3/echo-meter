# Compiler to use
CC = gcc

# Verbosity: default quiet; use `make V=1` for verbose
ifeq ($(V),1)
Q :=
else
Q := @
endif

# Compiler flags
CFLAGS = -Wall -g -O2 `pkg-config --cflags gtk4 gtk4-layer-shell-0 json-glib-1.0` -MMD -MP

# Linker flags
MAIN_LDFLAGS = `pkg-config --libs gtk4 gtk4-layer-shell-0 json-glib-1.0` -lasound
HELPER_LDFLAGS =

# Directories
SRC_DIR = src
HELPER_DIR = helper
OBJ_DIR = obj
ASSETS_DIR = assets
INSTALL_DIR = /usr/share/echo-meter
BIN_DIR = /usr/bin

# Detect real user's home if using sudo, fallback to current HOME
CONFIG_DIR = $(shell getent passwd $(SUDO_USER) 2>/dev/null | cut -d: -f6 || echo $(HOME))/.config/echo-meter

# Target executables
MAIN_TARGET = echo-meter
HELPER_TARGET = write-brightness

# Source files
MAIN_SOURCES = $(SRC_DIR)/echo-meter.c $(SRC_DIR)/conf.c $(SRC_DIR)/tool.c
HELPER_SOURCES = $(HELPER_DIR)/write-brightness.c

# Object files
MAIN_OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(MAIN_SOURCES)))
HELPER_OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(HELPER_SOURCES)))

# Dependency files
DEPS = $(MAIN_OBJECTS:.o=.d) $(HELPER_OBJECTS:.o=.d)

# Default target
all: $(MAIN_TARGET) $(HELPER_TARGET)

# Link main executable
$(MAIN_TARGET): $(MAIN_OBJECTS)
	$(Q)$(CC) $(MAIN_OBJECTS) -o $@ $(MAIN_LDFLAGS)

# Link helper executable
$(HELPER_TARGET): $(HELPER_OBJECTS)
	$(Q)$(CC) $(HELPER_OBJECTS) -o $@ $(HELPER_LDFLAGS)

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(Q)$(CC) $(CFLAGS) -Iinclude -c $< -o $@

$(OBJ_DIR)/%.o: $(HELPER_DIR)/%.c | $(OBJ_DIR)
	$(Q)$(CC) $(CFLAGS) -Iinclude -c $< -o $@

# Create obj directory
$(OBJ_DIR):
	$(Q)mkdir -p $(OBJ_DIR)

# Install targets
install: $(MAIN_TARGET) $(HELPER_TARGET)
	$(Q)install -d $(BIN_DIR)
	$(Q)install -m 755 $(MAIN_TARGET) $(BIN_DIR)/$(MAIN_TARGET)
	$(Q)install -d $(INSTALL_DIR)
	$(Q)install -m 4755 -o root -g root $(HELPER_TARGET) $(INSTALL_DIR)/$(HELPER_TARGET)
	$(Q)install -m 644 $(ASSETS_DIR)/conf.json $(INSTALL_DIR)/
	$(Q)install -m 644 $(ASSETS_DIR)/style.css $(INSTALL_DIR)/
	$(Q)install -d $(CONFIG_DIR)
	$(Q)install -m 644 $(ASSETS_DIR)/conf.json $(CONFIG_DIR)/
	$(Q)install -m 644 $(ASSETS_DIR)/style.css $(CONFIG_DIR)/

# Uninstall targets (system files only, keep user configs)
uninstall:
	$(Q)rm -f $(BIN_DIR)/$(MAIN_TARGET)
	$(Q)rm -f $(INSTALL_DIR)/$(HELPER_TARGET)
	$(Q)rm -f $(INSTALL_DIR)/conf.json
	$(Q)rm -f $(INSTALL_DIR)/style.css
	$(Q)rmdir --ignore-fail-on-non-empty $(INSTALL_DIR) 2>/dev/null || true

# Clean up
clean:
	$(Q)rm -rf $(OBJ_DIR) $(MAIN_TARGET) $(HELPER_TARGET)

# Include dependencies
-include $(DEPS)

# Phony targets
.PHONY: all install uninstall clean

