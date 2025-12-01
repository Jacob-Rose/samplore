# CMakeLists.txt for Samplore Tests
cmake_minimum_required(VERSION 3.16)

project(SamploreTests VERSION 1.0.0 LANGUAGES C CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE
add_subdirectory(/home/jakee/Documents/juce ${CMAKE_BINARY_DIR}/juce)

# Add Catch2
add_library(Catch2 INTERFACE)
target_include_directories(Catch2 INTERFACE 3rd_party/Catch2/single_include)

# Collect all Samplore header files (for includes only)
file(GLOB SAMPLORE_HEADERS "../*.h")

# Test source files - explicitly list what we're testing
set(TEST_SOURCES
    main_test.cpp
    BasicThemeTest.cpp
)

# Create test executable
juce_add_console_app(SamploreTests
    PRODUCT_NAME "Samplore Tests"
)

target_sources(SamploreTests
    PRIVATE
        ${TEST_SOURCES}
        ${SAMPLORE_HEADERS}
)

# Link JUCE modules
target_link_libraries(SamploreTests
    PRIVATE
        Catch2
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Include directories
target_include_directories(SamploreTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../../JuceLibraryCode
)

# JUCE compile definitions
target_compile_definitions(SamploreTests
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_STANDALONE_APPLICATION=1
)

# Enable testing
enable_testing()
add_test(NAME SamploreTests COMMAND SamploreTests)
