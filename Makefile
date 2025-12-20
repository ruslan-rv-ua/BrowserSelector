# Compiler settings
CC = gcc
WINDRES = windres
CFLAGS = -std=c11 -Wall -Wextra -O2 -Iinclude
LDFLAGS = -mwindows -static-libgcc -static
LIBS = -lcomctl32 -ladvapi32 -lshell32 -luser32 -lgdi32 -lkernel32 -lcomdlg32 -lole32

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/config/config.c \
          $(SRC_DIR)/ui/mainwindow.c \
          $(SRC_DIR)/ui/settings.c \
          $(SRC_DIR)/ui/commandeditor.c \
          $(SRC_DIR)/executor/executor.c \
          $(SRC_DIR)/registry/registry.c \
          include/cJSON.c

# Object files
OBJECTS = $(OBJ_DIR)/main.o \
          $(OBJ_DIR)/config.o \
          $(OBJ_DIR)/mainwindow.o \
          $(OBJ_DIR)/settings.o \
          $(OBJ_DIR)/commandeditor.o \
          $(OBJ_DIR)/executor.o \
          $(OBJ_DIR)/registry.o \
          $(OBJ_DIR)/cJSON.o

RESOURCE_OBJ = $(OBJ_DIR)/app.res

# Target executable
TARGET = $(BIN_DIR)/BrowserSelector.exe

# Default target
all: directories $(TARGET)
	@echo Build complete: $(TARGET)

# Create directories
directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

# Compile source files
$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/config.o: $(SRC_DIR)/config/config.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/mainwindow.o: $(SRC_DIR)/ui/mainwindow.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/settings.o: $(SRC_DIR)/ui/settings.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/commandeditor.o: $(SRC_DIR)/ui/commandeditor.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/executor.o: $(SRC_DIR)/executor/executor.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/registry.o: $(SRC_DIR)/registry/registry.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/cJSON.o: include/cJSON.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile resource file
$(RESOURCE_OBJ): resources/app.rc
	$(WINDRES) resources/app.rc -O coff -o $(RESOURCE_OBJ)

# Link executable
$(TARGET): $(OBJECTS) $(RESOURCE_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo Clean complete

# Release build (optimized, stripped)
release: CFLAGS += -DNDEBUG -Os
release: LDFLAGS += -s
release: clean all
	@echo Release build complete

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: LDFLAGS = -mconsole -static-libgcc
debug: clean all
	@echo Debug build complete

# Run the application
run: all
	$(TARGET)

# Install (copy to bin with config)
install: all
	cp config.json $(BIN_DIR)/
	@echo Installation complete

.PHONY: all clean release debug run install directories
