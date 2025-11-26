#!/bin/bash
# SamplifyPlus Clean Script
# Cleans build artifacts

set -e

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Detect platform
PLATFORM=$(uname -s)

echo "================================"
echo "SamplifyPlus Clean Script"
echo "================================"
echo "Platform: $PLATFORM"
echo ""

case "$PLATFORM" in
    Linux)
        echo "Cleaning Linux build..."
        BUILD_DIR="$PROJECT_ROOT/Builds/LinuxMakefile"

        if [ -f "$BUILD_DIR/Makefile" ]; then
            make -C "$BUILD_DIR" clean
            echo "✓ Linux build artifacts cleaned"
        else
            echo "No Makefile found, skipping..."
        fi

        # Also remove build directory
        if [ -d "$BUILD_DIR/build" ]; then
            rm -rf "$BUILD_DIR/build"
            echo "✓ Removed build directory"
        fi
        ;;

    Darwin)
        echo "Cleaning macOS build..."
        BUILD_DIR="$PROJECT_ROOT/Builds/MacOSX"

        if [ -d "$BUILD_DIR/build" ]; then
            rm -rf "$BUILD_DIR/build"
            echo "✓ macOS build artifacts cleaned"
        fi
        ;;

    MINGW* | MSYS* | CYGWIN*)
        echo "Cleaning Windows build..."
        echo "Note: Manual clean recommended for Visual Studio builds"
        echo "Use 'Clean Solution' in Visual Studio or delete:"
        echo "  Builds/VisualStudio2022/x64/"
        echo "  Builds/VisualStudio2019/x64/"
        ;;

    *)
        echo "Error: Unsupported platform: $PLATFORM"
        exit 1
        ;;
esac

echo ""
echo "Clean complete!"
