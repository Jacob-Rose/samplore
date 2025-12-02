#!/bin/bash
# Samplore Portable Installer
# Run from the extracted release directory

set -e

INSTALL_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${BLUE}Samplore Installer${NC}"
echo "=================="
echo

# Check if we're in the right directory
if [ ! -f "$INSTALL_DIR/samplore" ]; then
    echo -e "${RED}Error: samplore executable not found${NC}"
    echo "Please run this script from the extracted Samplore directory"
    exit 1
fi

# Create directories
mkdir -p "$HOME/.local/share/applications"
mkdir -p "$HOME/.local/share/icons/hicolor/512x512/apps"
mkdir -p "$HOME/.local/bin"

# Copy executable
echo "Installing executable..."
cp "$INSTALL_DIR/samplore" "$HOME/.local/bin/samplore"
chmod +x "$HOME/.local/bin/samplore"
echo -e "${GREEN}✓${NC} Installed to: $HOME/.local/bin/samplore"

# Copy icon
if [ -f "$INSTALL_DIR/samplore.png" ]; then
    echo "Installing icon..."
    cp "$INSTALL_DIR/samplore.png" "$HOME/.local/share/icons/hicolor/512x512/apps/samplore.png"
    echo -e "${GREEN}✓${NC} Icon installed"
fi

# Install desktop entry
if [ -f "$INSTALL_DIR/samplore.desktop" ]; then
    echo "Installing desktop entry..."
    cp "$INSTALL_DIR/samplore.desktop" "$HOME/.local/share/applications/samplore.desktop"
    chmod +x "$HOME/.local/share/applications/samplore.desktop"
    echo -e "${GREEN}✓${NC} Desktop entry installed"
fi

# Update caches
if command -v gtk-update-icon-cache &> /dev/null; then
    gtk-update-icon-cache -f -t "$HOME/.local/share/icons/hicolor" 2>/dev/null || true
fi
if command -v update-desktop-database &> /dev/null; then
    update-desktop-database "$HOME/.local/share/applications" 2>/dev/null || true
fi

echo
echo -e "${GREEN}Installation complete!${NC}"
echo
echo "Run 'samplore' from terminal or find it in your application menu"
echo

# Check PATH
if [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
    echo -e "${BLUE}Note:${NC} Add to your ~/.bashrc or ~/.zshrc:"
    echo "  export PATH=\"\$HOME/.local/bin:\$PATH\""
fi
