#!/bin/bash
# Samplore Uninstaller

set -e

GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}Samplore Uninstaller${NC}"
echo "===================="
echo

rm -f "$HOME/.local/bin/samplore"
rm -f "$HOME/.local/share/icons/hicolor/512x512/apps/samplore.png"
rm -f "$HOME/.local/share/applications/samplore.desktop"

if command -v gtk-update-icon-cache &> /dev/null; then
    gtk-update-icon-cache -f -t "$HOME/.local/share/icons/hicolor" 2>/dev/null || true
fi
if command -v update-desktop-database &> /dev/null; then
    update-desktop-database "$HOME/.local/share/applications" 2>/dev/null || true
fi

echo -e "${GREEN}Uninstalled successfully${NC}"
