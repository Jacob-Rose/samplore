#!/bin/bash
# SamplifyPlus Run Script
# Launches the built application

set -e  # Exit on error

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Load environment variables from .env if it exists
if [ -f "$PROJECT_ROOT/.env" ]; then
    set -a  # automatically export all variables
    source "$PROJECT_ROOT/.env"
    set +a
fi

# Default values if not set in .env
BUILD_CONFIG=${BUILD_CONFIG:-Release}

# Detect platform
PLATFORM=$(uname -s)

case "$PLATFORM" in
    Linux)
        BINARY_PATH="$PROJECT_ROOT/Builds/LinuxMakefile/build/SamplifyPlus"
        
        if [ ! -f "$BINARY_PATH" ]; then
            echo "Error: Binary not found at $BINARY_PATH"
            echo "Run './scripts/build.sh' first to build the application."
            exit 1
        fi
        
        echo "Launching SamplifyPlus..."
        "$BINARY_PATH"
        ;;

    Darwin)
        APP_PATH="$PROJECT_ROOT/Builds/MacOSX/build/$BUILD_CONFIG/SamplifyPlus.app"
        
        if [ ! -d "$APP_PATH" ]; then
            echo "Error: Application not found at $APP_PATH"
            echo "Run './scripts/build.sh' first to build the application."
            exit 1
        fi
        
        echo "Launching SamplifyPlus..."
        open "$APP_PATH"
        ;;

    MINGW* | MSYS* | CYGWIN*)
        EXE_PATH="$PROJECT_ROOT/Builds/VisualStudio2022/x64/$BUILD_CONFIG/App/SamplifyPlus.exe"
        
        if [ ! -f "$EXE_PATH" ]; then
            # Try VS2019 path
            EXE_PATH="$PROJECT_ROOT/Builds/VisualStudio2019/x64/$BUILD_CONFIG/App/SamplifyPlus.exe"
        fi
        
        if [ ! -f "$EXE_PATH" ]; then
            echo "Error: Executable not found."
            echo "Build the application in Visual Studio first."
            exit 1
        fi
        
        echo "Launching SamplifyPlus..."
        "$EXE_PATH"
        ;;

    *)
        echo "Error: Unsupported platform: $PLATFORM"
        exit 1
        ;;
esac
