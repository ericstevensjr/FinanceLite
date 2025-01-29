# Compiler and flags
CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lsqlite3 -lcjson

# Directories
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = obj
BUILD_DIR = .

# Source and object files
SRC_FILES = $(SRC_DIR)/main.c $(SRC_DIR)/budget.c $(SRC_DIR)/database.c $(SRC_DIR)/utils.c $(SRC_DIR)/recurring.c
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET = $(BUILD_DIR)/finance_lite

# Default target
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(TARGET) $(LDFLAGS)

# Rule to compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	# Ensure the object directory exists before compiling
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Clean up the build directory
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean
