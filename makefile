# Makefile for C++ OOP Project (Optimized & Recursive)

# Compiler settings
CXX        := g++
PKG_CONFIG := pkg-config
CXXFLAGS   := -Wall -Wextra -std=c++17 -I include -I src
LDFLAGS    :=

ifeq ($(OS),Windows_NT)
	CXX := C:/msys64/ucrt64/bin/g++.exe
	PKG_CONFIG := C:/msys64/ucrt64/bin/pkg-config.exe
	CXXFLAGS += $(shell "$(PKG_CONFIG)" --cflags raylib)
	LDFLAGS += $(shell "$(PKG_CONFIG)" --libs raylib) -lopengl32 -lgdi32 -lwinmm
	TARGET := bin/game.exe
	RUN_CMD := .\bin\game.exe
else
	CXXFLAGS += $(shell $(PKG_CONFIG) --cflags raylib)
	LDFLAGS += $(shell $(PKG_CONFIG) --libs raylib)
	TARGET := bin/game
	RUN_CMD := ./bin/game
endif

# Directories
SRC_DIR     := src
OBJ_DIR     := build
BIN_DIR     := bin
INCLUDE_DIR := include
DATA_DIR    := data
CONFIG_DIR  := config

# 1. Recursive Source Finding
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')

# 2. Dynamic Object Mapping
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Main targets
all: directories $(TARGET)

# Create necessary root directories
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) $(DATA_DIR) $(CONFIG_DIR)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Build successful! Executable is at $(TARGET)"

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the game
run: all
	$(RUN_CMD)

# Clean up generated files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Cleaned up $(OBJ_DIR) and $(BIN_DIR)"

# Rebuild everything from scratch
rebuild: clean all

.PHONY: all clean rebuild run directories