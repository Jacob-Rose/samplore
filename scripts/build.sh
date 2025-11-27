#!/bin/bash
# Samplore Build Script
# Universal build script for Linux, macOS, and Windows (Git Bash/MSYS2)

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
    echo ""
    echo "⚠ .env file not found!"
    echo ""
    echo "Please run the configuration script first:"
    echo "  cp .env.example .env"
    echo "  # Edit .env and set JUCE_PATH"
    echo "  python3 scripts/configure.py"
    echo ""
    exit 1
fi

# Default values if not set in .env
BUILD_CONFIG=${BUILD_CONFIG:-Release}
BUILD_JOBS=${BUILD_JOBS:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}
JUCE_PATH=${JUCE_PATH:-$HOME/Documents/juce}

# Detect platform
PLATFORM=$(uname -s)

echo "======================================================================"
echo "Samplore Build Script"
echo "======================================================================"
echo "Platform:       $PLATFORM"
echo "Config:         $BUILD_CONFIG"
echo "Jobs:           $BUILD_JOBS"
echo "JUCE Path:      $JUCE_PATH"
echo "======================================================================"
echo ""

case "$PLATFORM" in
    Linux)
        echo "Building for Linux (Makefile)..."
        BUILD_DIR="$PROJECT_ROOT/Builds/LinuxMakefile"

        if [ ! -f "$BUILD_DIR/Makefile" ]; then
            echo "⚠ Makefile not found at $BUILD_DIR/Makefile"
            echo ""
            echo "Attempting to generate build files..."
            python3 "$SCRIPT_DIR/configure.py" || {
                echo ""
                echo "Failed to generate build files automatically."
                echo "Please run: python3 scripts/configure.py"
                exit 1
            }
            echo ""
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
            echo "⚠ Xcode project not found at $XCODE_PROJECT"
            echo ""
            echo "Attempting to generate build files..."
            python3 "$SCRIPT_DIR/configure.py" || {
                echo ""
                echo "Failed to generate build files automatically."
                echo "Please run: python3 scripts/configure.py"
                exit 1
            }
            echo ""
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
        echo "Building for Windows (MSBuild)..."
        
        # Try to find MSBuild
        MSBUILD=""
        MSBUILD_PATHS=(
            "/c/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe"
            "/c/Program Files/Microsoft Visual Studio/2022/Professional/MSBuild/Current/Bin/MSBuild.exe"
            "/c/Program Files/Microsoft Visual Studio/2022/Enterprise/MSBuild/Current/Bin/MSBuild.exe"
            "/c/Program Files (x86)/Microsoft Visual Studio/2019/Community/MSBuild/Current/Bin/MSBuild.exe"
            "/c/Program Files (x86)/Microsoft Visual Studio/2019/Professional/MSBuild/Current/Bin/MSBuild.exe"
        )
        
        for path in "${MSBUILD_PATHS[@]}"; do
            if [ -f "$path" ]; then
                MSBUILD="$path"
                break
            fi
        done
        
        if [ -z "$MSBUILD" ]; then
            # Try to find in PATH
            MSBUILD=$(which msbuild.exe 2>/dev/null || which MSBuild.exe 2>/dev/null || echo "")
        fi
        
        if [ -z "$MSBUILD" ]; then
            echo "✗ MSBuild not found!"
            echo ""
            echo "Please install Visual Studio 2019 or 2022 with C++ tools."
            echo "Or open the solution file manually:"
            echo "  Builds/VisualStudio2022/SamplifyPlus.sln"
            exit 1
        fi
        
        echo "Found MSBuild: $MSBUILD"
        
        # Try VS2022 first, fall back to VS2019
        BUILD_DIR="$PROJECT_ROOT/Builds/VisualStudio2022"
        SLN_FILE="$BUILD_DIR/SamplifyPlus.sln"
        
        if [ ! -f "$SLN_FILE" ]; then
            BUILD_DIR="$PROJECT_ROOT/Builds/VisualStudio2019"
            SLN_FILE="$BUILD_DIR/SamplifyPlus.sln"
        fi
        
        if [ ! -f "$SLN_FILE" ]; then
            echo "⚠ Visual Studio solution not found"
            echo ""
            echo "Attempting to generate build files..."
            python3 "$SCRIPT_DIR/configure.py" || {
                echo ""
                echo "Failed to generate build files automatically."
                echo "Please run: python3 scripts/configure.py"
                exit 1
            }
            
            # Re-check for solution file
            BUILD_DIR="$PROJECT_ROOT/Builds/VisualStudio2022"
            SLN_FILE="$BUILD_DIR/SamplifyPlus.sln"
            if [ ! -f "$SLN_FILE" ]; then
                BUILD_DIR="$PROJECT_ROOT/Builds/VisualStudio2019"
                SLN_FILE="$BUILD_DIR/SamplifyPlus.sln"
            fi
            
            if [ ! -f "$SLN_FILE" ]; then
                echo "✗ Solution file still not found after generation"
                exit 1
            fi
        fi
        
        echo "Building: $SLN_FILE"
        "$MSBUILD" "$SLN_FILE" //p:Configuration="$BUILD_CONFIG" //p:Platform=x64 //m
        
        BINARY_PATH="$BUILD_DIR/x64/$BUILD_CONFIG/SamplifyPlus.exe"
        if [ -f "$BINARY_PATH" ]; then
            echo ""
            echo "✓ Build successful!"
            echo "Binary location: $BINARY_PATH"
        else
            echo ""
            echo "✗ Build completed but binary not found at expected location."
            echo "Expected: $BINARY_PATH"
            exit 1
        fi
        ;;

    *)
        echo "Error: Unsupported platform: $PLATFORM"
        exit 1
        ;;
esac

echo ""
echo "Build complete!"
