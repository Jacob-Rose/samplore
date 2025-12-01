# CMakeLists.txt for Samplore Tests
# Builds Catch2 tests for Samplore core classes

cmake_minimum_required(VERSION 3.16)

project(SamploreTests
    VERSION 1.0.0
    DESCRIPTION "Samplore Unit Tests"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add Catch2 (v2.x single-header version)
add_library(Catch2 INTERFACE)
target_include_directories(Catch2 INTERFACE 3rd_party/Catch2/single_include)

# Find JUCE modules (from JuceLibraryCode)
set(JUCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../JuceLibraryCode")

# Samplore source files needed for tests
set(SAMPLORE_SOURCES
    ../ThemeManager.cpp
    ../Sample.cpp
    ../SampleLibrary.cpp
)

# Test source files
set(TEST_SOURCES
    main_test.cpp
    ThemeManagerTests.cpp
    SampleTests.cpp
    SampleLibraryTests.cpp
)

# Create test executable
add_executable(SamploreTests
    ${TEST_SOURCES}
    ${SAMPLORE_SOURCES}
)

# Link Catch2
target_link_libraries(SamploreTests
    PRIVATE
        Catch2
)

# Include directories
target_include_directories(SamploreTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${JUCE_ROOT}
)

# Platform-specific settings
if(UNIX AND NOT APPLE)
    # Linux
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FREETYPE REQUIRED freetype2)
    pkg_check_modules(X11 REQUIRED x11 xext xinerama)
    pkg_check_modules(ALSA REQUIRED alsa)
    
    target_link_libraries(SamploreTests
        PRIVATE
            ${FREETYPE_LIBRARIES}
            ${X11_LIBRARIES}
            ${ALSA_LIBRARIES}
            pthread
            dl
    )
    
    target_include_directories(SamploreTests
        PRIVATE
            ${FREETYPE_INCLUDE_DIRS}
            ${X11_INCLUDE_DIRS}
            ${ALSA_INCLUDE_DIRS}
    )
    
    target_compile_definitions(SamploreTests
        PRIVATE
            LINUX=1
    )
elseif(APPLE)
    # macOS
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    find_library(COREMIDI_LIBRARY CoreMIDI)
    find_library(ACCELERATE_LIBRARY Accelerate)
    
    target_link_libraries(SamploreTests
        PRIVATE
            ${COCOA_LIBRARY}
            ${IOKIT_LIBRARY}
            ${COREAUDIO_LIBRARY}
            ${COREMIDI_LIBRARY}
            ${ACCELERATE_LIBRARY}
    )
elseif(WIN32)
    # Windows
    target_link_libraries(SamploreTests
        PRIVATE
            winmm
            ole32
            user32
            gdi32
            comctl32
    )
endif()

# JUCE compile definitions
target_compile_definitions(SamploreTests
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_USE_DARK_SPLASH_SCREEN=1
        JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_STANDALONE_APPLICATION=1
        JUCE_MODULE_AVAILABLE_juce_core=1
        JUCE_MODULE_AVAILABLE_juce_data_structures=1
        JUCE_MODULE_AVAILABLE_juce_events=1
        JUCE_MODULE_AVAILABLE_juce_graphics=1
        JUCE_MODULE_AVAILABLE_juce_gui_basics=1
        JUCE_MODULE_AVAILABLE_juce_audio_basics=1
        JUCE_MODULE_AVAILABLE_juce_audio_devices=1
        JUCE_MODULE_AVAILABLE_juce_audio_formats=1
)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(SamploreTests PRIVATE -Wall -Wextra)
elseif(MSVC)
    target_compile_options(SamploreTests PRIVATE /W4)
endif()

# Enable testing
enable_testing()

# Add test
add_test(NAME SamploreTests COMMAND SamploreTests)
