#!/bin/bash
# Samplore First-Time Setup Script
# Guides users through initial project configuration

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
ENV_FILE="$PROJECT_ROOT/.env"
ENV_EXAMPLE="$PROJECT_ROOT/.env.example"

echo "======================================================================"
echo "Samplore - First-Time Setup"
echo "======================================================================"
echo ""
echo "This script will help you configure Samplore for building."
echo ""

# Detect platform
PLATFORM=$(uname -s)
case "$PLATFORM" in
    Linux)
        PLATFORM_NAME="Linux"
        ;;
    Darwin)
        PLATFORM_NAME="macOS"
        ;;
    MINGW* | MSYS* | CYGWIN*)
        PLATFORM_NAME="Windows"
        ;;
    *)
        PLATFORM_NAME="Unknown ($PLATFORM)"
        ;;
esac

echo "Detected platform: $PLATFORM_NAME"
echo ""

# Check for .env file
if [ -f "$ENV_FILE" ]; then
    echo "✓ Found existing .env file"
    echo ""
    read -p "Do you want to reconfigure? [y/N] " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Keeping existing configuration."
        echo ""
        echo "To build the project, run:"
        echo "  ./scripts/build.sh"
        exit 0
    fi
    echo ""
else
    echo "Creating .env file from template..."
    cp "$ENV_EXAMPLE" "$ENV_FILE"
    echo "✓ Created .env"
    echo ""
fi

# Prompt for JUCE path
echo "Please enter the path to your JUCE installation:"
echo "Examples:"
echo "  Linux:   /home/username/JUCE"
echo "  macOS:   /Users/username/JUCE"
echo "  Windows: C:/JUCE"
echo ""

# Try to detect common JUCE locations
JUCE_SUGGESTIONS=()
if [ -d "$HOME/JUCE" ]; then
    JUCE_SUGGESTIONS+=("$HOME/JUCE")
fi
if [ -d "$HOME/Documents/JUCE" ]; then
    JUCE_SUGGESTIONS+=("$HOME/Documents/JUCE")
fi
if [ -d "/usr/local/JUCE" ]; then
    JUCE_SUGGESTIONS+=("/usr/local/JUCE")
fi

if [ ${#JUCE_SUGGESTIONS[@]} -gt 0 ]; then
    echo "Found possible JUCE installations:"
    for suggestion in "${JUCE_SUGGESTIONS[@]}"; do
        echo "  - $suggestion"
    done
    echo ""
fi

read -p "JUCE Path: " JUCE_PATH
JUCE_PATH=$(echo "$JUCE_PATH" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')  # Trim whitespace

# Validate JUCE path
if [ -z "$JUCE_PATH" ]; then
    echo "✗ JUCE path cannot be empty"
    exit 1
fi

# Expand ~ if used
JUCE_PATH="${JUCE_PATH/#\~/$HOME}"

if [ ! -d "$JUCE_PATH" ]; then
    echo ""
    echo "⚠ Warning: Directory does not exist: $JUCE_PATH"
    echo ""
    read -p "Continue anyway? [y/N] " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

if [ -d "$JUCE_PATH" ] && [ ! -d "$JUCE_PATH/modules" ]; then
    echo ""
    echo "⚠ Warning: No 'modules' directory found in JUCE path"
    echo "   Expected: $JUCE_PATH/modules"
    echo ""
fi

# Update .env file
echo ""
echo "Updating .env file..."

# Use sed to update JUCE_PATH in .env
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS sed
    sed -i '' "s|^JUCE_PATH=.*|JUCE_PATH=$JUCE_PATH|" "$ENV_FILE"
else
    # GNU sed (Linux, Windows Git Bash)
    sed -i "s|^JUCE_PATH=.*|JUCE_PATH=$JUCE_PATH|" "$ENV_FILE"
fi

echo "✓ Updated JUCE_PATH in .env"
echo ""

# Ask about build configuration
echo "Select build configuration:"
echo "  1) Release (optimized, recommended)"
echo "  2) Debug (includes debug symbols)"
read -p "Choice [1]: " BUILD_CHOICE
BUILD_CHOICE=${BUILD_CHOICE:-1}

if [ "$BUILD_CHOICE" = "2" ]; then
    BUILD_CONFIG="Debug"
else
    BUILD_CONFIG="Release"
fi

if [[ "$OSTYPE" == "darwin"* ]]; then
    sed -i '' "s|^BUILD_CONFIG=.*|BUILD_CONFIG=$BUILD_CONFIG|" "$ENV_FILE"
else
    sed -i "s|^BUILD_CONFIG=.*|BUILD_CONFIG=$BUILD_CONFIG|" "$ENV_FILE"
fi

echo "✓ Set BUILD_CONFIG=$BUILD_CONFIG"
echo ""

# Check dependencies
if [ "$PLATFORM_NAME" = "Linux" ]; then
    echo "Checking Linux dependencies..."
    echo ""
    
    REQUIRED_PACKAGES=(
        "libfreetype6-dev"
        "libwebkit2gtk-4.1-dev"
        "libgtk-3-dev"
        "libasound2-dev"
        "libcurl4-openssl-dev"
    )
    
    MISSING_PACKAGES=()
    for pkg in "${REQUIRED_PACKAGES[@]}"; do
        if ! dpkg -l "$pkg" 2>/dev/null | grep -q "^ii"; then
            MISSING_PACKAGES+=("$pkg")
        fi
    done
    
    if [ ${#MISSING_PACKAGES[@]} -gt 0 ]; then
        echo "⚠ Missing required packages:"
        for pkg in "${MISSING_PACKAGES[@]}"; do
            echo "  - $pkg"
        done
        echo ""
        echo "Install with:"
        echo "  sudo apt-get install ${MISSING_PACKAGES[*]}"
        echo ""
        read -p "Install now? [Y/n] " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Nn]$ ]]; then
            sudo apt-get update
            sudo apt-get install -y "${MISSING_PACKAGES[@]}"
            echo ""
            echo "✓ Dependencies installed"
        fi
    else
        echo "✓ All dependencies installed"
    fi
    echo ""
fi

# Run configure script
echo "Running configuration script..."
echo ""
python3 "$SCRIPT_DIR/configure.py" || {
    echo ""
    echo "✗ Configuration failed"
    exit 1
}

echo ""
echo "======================================================================"
echo "Setup Complete!"
echo "======================================================================"
echo ""
echo "Your configuration:"
echo "  JUCE Path: $JUCE_PATH"
echo "  Build Config: $BUILD_CONFIG"
echo ""
echo "Next steps:"
echo "  1. Build the project:"
echo "       ./scripts/build.sh"
echo ""
echo "  2. Or use the Python build script:"
echo "       python3 scripts/build.py"
echo ""
echo "  3. Run the application:"
echo "       ./scripts/run.sh"
echo ""
