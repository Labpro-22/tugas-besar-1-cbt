# Makefile for C++ OOP Project (Optimized & Recursive)

# Directories
SRC_DIR     := src
OBJ_DIR     := build
BIN_DIR     := bin
INCLUDE_DIR := include
DATA_DIR    := data
CONFIG_DIR  := config
RAYLIB_DIR  := third_party/raylib

# Compiler settings
CXX            := g++
PKG_CONFIG     := pkg-config
RAYLIB_VERSION ?= 5.5
CXXFLAGS       := -Wall -Wextra -std=c++17 -I $(INCLUDE_DIR) -I $(SRC_DIR)
LDFLAGS        :=

ifeq ($(OS),Windows_NT)
	CXX := C:/msys64/ucrt64/bin/g++.exe
	PKG_CONFIG := C:/msys64/ucrt64/bin/pkg-config.exe
	RAYLIB_CFLAGS := $(shell "$(PKG_CONFIG)" --cflags raylib 2>NUL)
	RAYLIB_LIBS := $(shell "$(PKG_CONFIG)" --libs raylib 2>NUL)
	CXXFLAGS += $(RAYLIB_CFLAGS)
	LDFLAGS += $(RAYLIB_LIBS) -lopengl32 -lgdi32 -lwinmm
	TARGET := bin/game.exe
	RUN_CMD := .\bin\game.exe
else
	RAYLIB_CFLAGS := $(shell $(PKG_CONFIG) --cflags raylib 2>/dev/null)
	RAYLIB_LIBS := $(shell $(PKG_CONFIG) --libs raylib 2>/dev/null)
	CXXFLAGS += $(RAYLIB_CFLAGS)
	LDFLAGS += $(RAYLIB_LIBS) -lpthread -lm -ldl
	TARGET := bin/game
	RUN_CMD := ./bin/game
endif

# 1. Recursive Source Finding
rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
SRCS := $(call rwildcard,$(SRC_DIR)/,*.cpp)

# 2. Dynamic Object Mapping
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Main targets
all: check-raylib directories $(TARGET)

check-raylib:
	@$(PKG_CONFIG) --exists raylib || (echo "raylib belum terpasang. Jalankan 'make download' di WSL/Linux terlebih dahulu."; exit 1)

download:
ifeq ($(OS),Windows_NT)
	@echo "Target download ini untuk WSL/Linux. Untuk MSYS2 Windows, jalankan: pacman -S --needed mingw-w64-ucrt-x86_64-raylib pkgconf"
else
	@set -e; \
	if ! command -v apt-get >/dev/null 2>&1; then \
		echo "make download saat ini mendukung distro WSL berbasis Debian/Ubuntu."; \
		echo "Install manual: raylib, pkg-config, build-essential, dan dependency X11/OpenGL."; \
		exit 1; \
	fi; \
	sudo apt-get update; \
	sudo apt-get install -y build-essential pkg-config git cmake libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev; \
	if apt-cache show libraylib-dev >/dev/null 2>&1; then \
		sudo apt-get install -y libraylib-dev; \
	else \
		mkdir -p third_party; \
		if [ ! -d "$(RAYLIB_DIR)/.git" ]; then \
			rm -rf "$(RAYLIB_DIR)"; \
			git clone --depth 1 --branch "$(RAYLIB_VERSION)" https://github.com/raysan5/raylib.git "$(RAYLIB_DIR)"; \
		fi; \
		cmake -S "$(RAYLIB_DIR)" -B "$(RAYLIB_DIR)/build" -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release; \
		cmake --build "$(RAYLIB_DIR)/build"; \
		sudo cmake --install "$(RAYLIB_DIR)/build"; \
		sudo ldconfig; \
	fi; \
	echo "raylib siap dipakai. Jalankan: make run"
endif

# Create necessary root directories
directories:
ifeq ($(OS),Windows_NT)
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
	@if not exist "$(DATA_DIR)" mkdir "$(DATA_DIR)"
	@if not exist "$(CONFIG_DIR)" mkdir "$(CONFIG_DIR)"
else
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) $(DATA_DIR) $(CONFIG_DIR)
endif

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Build successful! Executable is at $(TARGET)"

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
ifeq ($(OS),Windows_NT)
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
else
	@mkdir -p $(dir $@)
endif
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the game
run: all
	$(RUN_CMD)

# Clean up generated files
clean:
ifeq ($(OS),Windows_NT)
	@if exist "$(OBJ_DIR)" rmdir /s /q "$(OBJ_DIR)"
	@if exist "$(BIN_DIR)" rmdir /s /q "$(BIN_DIR)"
else
	rm -rf $(OBJ_DIR) $(BIN_DIR)
endif
	@echo "Cleaned up $(OBJ_DIR) and $(BIN_DIR)"

# Rebuild everything from scratch
rebuild: clean all

.PHONY: all check-raylib download clean rebuild run directories
