# Compiler and flags
CXX = clang++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -g
INCLUDES = -I/opt/homebrew/include/SDL2
LDFLAGS = -L/opt/homebrew/lib
LIBS = -lSDL2

# Directories
SRC_DIR = src
BIN_DIR = bin

# Source and target
SOURCE = $(SRC_DIR)/chip8.cpp
TARGET = $(BIN_DIR)/chip8

# Default target
all: $(TARGET)

# Build
$(TARGET): $(SOURCE) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) $< -o $@ $(LIBS)

# Create bin directory
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean
clean:
	rm -rf $(BIN_DIR)

# Run
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CXXFLAGS += -DDEBUG -g3 -O0
debug: clean all

.PHONY: all clean run debug
