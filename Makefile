# ──────────────────────────────────────────────────────────────
# Procedural Forest — Makefile
# ──────────────────────────────────────────────────────────────

CXX := g++

TARGET := ltree_forest

SRC := main.cpp

# Header search path
INCLUDE := -Iinclude

# Common compiler flags
CXXFLAGS := -std=c++17 -Wall -Wextra $(INCLUDE)

# OpenGL + FreeGLUT libraries
LIBS := -lopengl32 -lglu32 -lfreeglut

# Build modes
CXXFLAGS_DEBUG := $(CXXFLAGS) -g -O1
CXXFLAGS_RELEASE := $(CXXFLAGS) -O3 -march=native -DNDEBUG

.PHONY: all debug release run clean info

all: debug

# ──────────────────────────────────────────────────────────────
# Info
# ──────────────────────────────────────────────────────────────

info:
	@echo.
	@echo ========================================
	@echo      PROCEDURAL FOREST BUILD SYSTEM
	@echo ========================================
	@echo.
	@echo Compiler : $(CXX)
	@echo Target   : $(TARGET).exe
	@echo Source   : $(SRC)
	@echo.
	@echo ========================================
	@echo.

# ──────────────────────────────────────────────────────────────
# Debug Build
# ──────────────────────────────────────────────────────────────

debug: info
	@echo [1/3] Compiling Debug Build...
	@$(CXX) $(CXXFLAGS_DEBUG) $(SRC) $(LIBS) -o $(TARGET)

	@echo [2/3] Linking Executable...
	@echo.

	@echo ========================================
	@echo DEBUG BUILD SUCCESS
	@echo Executable : $(TARGET).exe
	@echo Mode       : DEBUG
	@echo ========================================
	@echo.

# ──────────────────────────────────────────────────────────────
# Release Build
# ──────────────────────────────────────────────────────────────

release: info
	@echo [1/3] Compiling Release Build...
	@$(CXX) $(CXXFLAGS_RELEASE) $(SRC) $(LIBS) -o $(TARGET)

	@echo [2/3] Optimizing Binary...
	@echo.

	@echo ========================================
	@echo RELEASE BUILD SUCCESS
	@echo Executable : $(TARGET).exe
	@echo Mode       : RELEASE
	@echo ========================================
	@echo.

# ──────────────────────────────────────────────────────────────
# Run
# ──────────────────────────────────────────────────────────────

run: debug
	@echo Launching $(TARGET).exe ...
	@echo.
	@./$(TARGET)

# ──────────────────────────────────────────────────────────────
# Clean
# ──────────────────────────────────────────────────────────────

clean:
	@echo Cleaning build files...
	@-del /Q $(TARGET).exe 2>nul

	@echo.
	@echo ========================================
	@echo CLEAN COMPLETE
	@echo ========================================
	@echo.