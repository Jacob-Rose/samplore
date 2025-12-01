# Makefile for Samplore Tests
# Simple standalone build for Catch2 unit tests

# Compiler
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g

# pkg-config for system libraries
PKG_CONFIG := pkg-config

# Directories
JUCE_MODULES := /home/jakee/Documents/juce/modules
JUCE_ROOT := ../../JuceLibraryCode
SOURCE_DIR := ..
TEST_DIR := .
CATCH2_INCLUDE := 3rd_party/Catch2/single_include

# Include paths (with pkg-config for system libraries)
INCLUDES := -I$(TEST_DIR) \
            -I$(SOURCE_DIR) \
            -I$(JUCE_ROOT) \
            -I$(JUCE_MODULES) \
            -I$(CATCH2_INCLUDE) \
            $(shell $(PKG_CONFIG) --cflags freetype2 fontconfig harfbuzz x11 xext xinerama alsa)

# JUCE compile definitions
DEFINES := -DJUCE_WEB_BROWSER=0 \
           -DJUCE_USE_CURL=0 \
           -DJUCE_DISPLAY_SPLASH_SCREEN=0 \
           -DJUCE_USE_DARK_SPLASH_SCREEN=1 \
           -DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 \
           -DJUCE_STRICT_REFCOUNTEDPOINTER=1 \
           -DJUCE_STANDALONE_APPLICATION=1 \
           -DLINUX=1

# Platform-specific libraries (Linux) - match main project
LIBS := -lpthread -ldl -lrt $(shell $(PKG_CONFIG) --libs alsa freetype2 fontconfig gl libcurl x11 xext xinerama)

# Samplore source files - include everything
SAMPLORE_SOURCES := $(wildcard $(SOURCE_DIR)/*.cpp)

# Test source files  
TEST_SOURCES := main_test.cpp \
                BasicThemeTest.cpp

# JUCE module sources (from JuceLibraryCode)
JUCE_SOURCES := $(JUCE_ROOT)/include_juce_core.cpp \
                $(JUCE_ROOT)/include_juce_data_structures.cpp \
                $(JUCE_ROOT)/include_juce_events.cpp \
                $(JUCE_ROOT)/include_juce_graphics.cpp \
                $(JUCE_ROOT)/include_juce_gui_basics.cpp \
                $(JUCE_ROOT)/include_juce_audio_basics.cpp

# All source files
ALL_SOURCES := $(TEST_SOURCES) $(SAMPLORE_SOURCES) $(JUCE_SOURCES)

# Object files
OBJECTS := $(ALL_SOURCES:.cpp=.o)

# Output executable
TARGET := SamploreTests

# Default target
all: $(TARGET)

# Link executable
$(TARGET): $(OBJECTS)
	@echo "Linking $@..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo "Build complete: $@"

# Compile source files
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

# Run tests
test: $(TARGET)
	@echo "Running tests..."
	./$(TARGET)

# Clean build artifacts
clean:
	@echo "Cleaning..."
	rm -f $(OBJECTS) $(TARGET)
	rm -f $(SOURCE_DIR)/*.o
	rm -f $(JUCE_ROOT)/*.o
	@echo "Clean complete"

# Phony targets
.PHONY: all test clean

# Print configuration (for debugging)
info:
	@echo "CXX: $(CXX)"
	@echo "CXXFLAGS: $(CXXFLAGS)"
	@echo "INCLUDES: $(INCLUDES)"
	@echo "DEFINES: $(DEFINES)"
	@echo "LIBS: $(LIBS)"
	@echo "TEST_SOURCES: $(TEST_SOURCES)"
	@echo "SAMPLORE_SOURCES: $(SAMPLORE_SOURCES)"
	@echo "JUCE_SOURCES: $(JUCE_SOURCES)"
