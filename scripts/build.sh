#!/bin/bash
# SamplifyPlus Build Script
# Builds the application for the current platform

set -e  # Exit on error

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Load environment variables from .env if it exists
if [ -f "$PROJECT_ROOT/.env" ]; then
    echo "Loading environment from .env..."
    set -a  # automatically export all variables
    source "$PROJECT_ROOT/.env"
    set +a
else
    echo "Warning: .env file not found. Using defaults."
    echo "Copy .env.example to .env and configure for your system."
fi

# Default values if not set in .env
BUILD_CONFIG=${BUILD_CONFIG:-Release}
BUILD_JOBS=${BUILD_JOBS:-4}
JUCE_PATH=${JUCE_PATH:-$HOME/Documents/juce}

# Detect platform
PLATFORM=$(uname -s)

echo "================================"
echo "SamplifyPlus Build Script"
echo "================================"
echo "Platform:       $PLATFORM"
echo "Config:         $BUILD_CONFIG"
echo "Jobs:           $BUILD_JOBS"
echo "JUCE Path:      $JUCE_PATH"
echo "================================"
echo ""

case "$PLATFORM" in
    Linux)
        echo "Building for Linux (Makefile)..."
        BUILD_DIR="$PROJECT_ROOT/Builds/LinuxMakefile"

        if [ ! -f "$BUILD_DIR/Makefile" ]; then
            echo "Error: Makefile not found at $BUILD_DIR/Makefile"
            echo "Run Projucer to generate build files first."
            exit 1
        fi

        echo "Running: make -C $BUILD_DIR CONFIG=$BUILD_CONFIG -j$BUILD_JOBS"
        make -C "$BUILD_DIR" CONFIG="$BUILD_CONFIG" -j"$BUILD_JOBS"

        BINARY_PATH="$BUILD_DIR/build/SamplifyPlus"
        if [ -f "$BINARY_PATH" ]; then
            echo ""
            echo "✓ Build successful!"
            echo "Binary location: $BINARY_PATH"
        else
            echo ""
            echo "✗ Build completed but binary not found at expected location."
            exit 1
        fi
        ;;

    Darwin)
        echo "Building for macOS (Xcode)..."
        BUILD_DIR="$PROJECT_ROOT/Builds/MacOSX"
        XCODE_PROJECT="$BUILD_DIR/SamplifyPlus.xcodeproj"

        if [ ! -d "$XCODE_PROJECT" ]; then
            echo "Error: Xcode project not found at $XCODE_PROJECT"
            echo "Run Projucer to generate build files first."
            exit 1
        fi

        echo "Running: xcodebuild -project $XCODE_PROJECT -configuration $BUILD_CONFIG"
        xcodebuild -project "$XCODE_PROJECT" -configuration "$BUILD_CONFIG" -jobs "$BUILD_JOBS"

        APP_PATH="$BUILD_DIR/build/$BUILD_CONFIG/SamplifyPlus.app"
        if [ -d "$APP_PATH" ]; then
            echo ""
            echo "✓ Build successful!"
            echo "Application: $APP_PATH"
        else
            echo ""
            echo "✗ Build completed but app not found at expected location."
            exit 1
        fi
        ;;

    MINGW* | MSYS* | CYGWIN*)
        echo "Building for Windows..."
        echo "Note: Windows builds should use Visual Studio directly."
        echo "Open Builds/VisualStudio2022/SamplifyPlus.sln or"
        echo "      Builds/VisualStudio2019/SamplifyPlus.sln"
        echo ""
        echo "To build from command line, use MSBuild:"
        echo "  msbuild Builds/VisualStudio2022/SamplifyPlus.sln /p:Configuration=$BUILD_CONFIG"
        exit 1
        ;;

    *)
        echo "Error: Unsupported platform: $PLATFORM"
        exit 1
        ;;
esac

echo ""
echo "Build complete!"
