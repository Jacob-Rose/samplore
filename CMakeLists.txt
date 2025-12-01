# CMakeLists.txt for Samplore (basic CMake support)
cmake_minimum_required(VERSION 3.16)

project(Samplore VERSION 1.0.0 LANGUAGES C CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE
add_subdirectory(/home/jakee/Documents/juce ${CMAKE_BINARY_DIR}/juce)

# Add executable
juce_add_console_app(Samplore
    PRODUCT_NAME "Samplore"
)

# Add all source files from main directory and specific subdirectories
file(GLOB SAMPLORE_SOURCES "Source/*.cpp")
file(GLOB UI_SOURCES "Source/UI/*.cpp")
file(GLOB ANIMATION_SOURCES "Source/Animation/*.cpp")

target_sources(Samplore
    PRIVATE
        ${SAMPLORE_SOURCES}
        ${UI_SOURCES}
        ${ANIMATION_SOURCES}
)

# Link JUCE modules
target_link_libraries(Samplore
    PRIVATE
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
target_include_directories(Samplore
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode
)

# JUCE compile definitions
target_compile_definitions(Samplore
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_STANDALONE_APPLICATION=1
)

# Enable testing
enable_testing()
add_test(NAME SamploreTest COMMAND Samplore)
