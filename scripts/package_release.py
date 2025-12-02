#!/usr/bin/env python3
"""
Package Samplore for GitHub release
Creates platform-specific portable archives
"""

import os
import sys
import shutil
import subprocess
import platform
from pathlib import Path

# Colors
class Colors:
    BLUE = '\033[0;34m'
    GREEN = '\033[0;32m'
    RED = '\033[0;31m'
    NC = '\033[0m'

def print_info(msg):
    print(f"{Colors.BLUE}{msg}{Colors.NC}")

def print_success(msg):
    print(f"{Colors.GREEN}✓ {msg}{Colors.NC}")

def print_error(msg):
    print(f"{Colors.RED}Error: {msg}{Colors.NC}")
    sys.exit(1)

def get_version():
    """Extract version from Samplore.jucer"""
    project_dir = Path(__file__).parent.parent
    jucer_file = project_dir / "Samplore.jucer"
    
    if not jucer_file.exists():
        return "0.9.0"  # fallback
    
    with open(jucer_file, 'r') as f:
        for line in f:
            if 'version=' in line:
                import re
                match = re.search(r'version="([^"]+)"', line)
                if match:
                    return match.group(1)
    return "0.9.0"

def package_linux():
    """Package Linux release"""
    project_dir = Path(__file__).parent.parent
    version = get_version()
    
    # Check if executable exists
    executable = project_dir / "Builds/LinuxMakefile/build/Samplore"
    if not executable.exists():
        print_error("Samplore executable not found. Build first with: python3 scripts/build.py")
    
    print_info(f"Packaging Samplore v{version} for Linux...")
    
    # Create release directory
    release_name = f"samplore-{version}-linux-x86_64"
    release_dir = project_dir / "dist" / release_name
    
    if release_dir.exists():
        shutil.rmtree(release_dir)
    release_dir.mkdir(parents=True, exist_ok=True)
    
    # Copy executable
    print("Copying executable...")
    shutil.copy2(executable, release_dir / "samplore")
    os.chmod(release_dir / "samplore", 0o755)
    print_success("Executable copied")
    
    # Copy icon (resize to 512x512)
    icon_source = project_dir / "Reference/logo.PNG"
    icon_dest = release_dir / "samplore.png"
    
    print("Processing icon...")
    if shutil.which("convert"):  # ImageMagick
        subprocess.run([
            "convert", str(icon_source),
            "-resize", "512x512",
            str(icon_dest)
        ], check=True)
        print_success("Icon resized to 512x512")
    else:
        shutil.copy2(icon_source, icon_dest)
        print_success("Icon copied (install ImageMagick for auto-resize)")
    
    # Copy desktop file and installers
    shutil.copy2(project_dir / "dist/samplore.desktop", release_dir / "samplore.desktop")
    shutil.copy2(project_dir / "dist/install.sh", release_dir / "install.sh")
    shutil.copy2(project_dir / "dist/uninstall.sh", release_dir / "uninstall.sh")
    os.chmod(release_dir / "install.sh", 0o755)
    os.chmod(release_dir / "uninstall.sh", 0o755)
    print_success("Desktop integration files copied")
    
    # Create README
    readme_content = f"""# Samplore v{version} - Linux

## Quick Start

### Option 1: Run Portable
```bash
./samplore
```

### Option 2: Install System-Wide
```bash
./install.sh
```

This installs to `~/.local/bin` and adds desktop integration (no sudo required).

## Uninstall
```bash
./uninstall.sh
```

## Requirements
- Linux x86_64
- ALSA/PulseAudio
- X11 or Wayland

## About Icons
Note: Linux executables don't have embedded icons. The icon appears when you:
1. Install via `./install.sh` (recommended), or
2. Run from a file manager that reads .desktop files

For more info: https://github.com/yourusername/samplore
"""
    
    with open(release_dir / "README.txt", 'w') as f:
        f.write(readme_content)
    print_success("README created")
    
    # Create tarball
    print("Creating archive...")
    archive_name = f"{release_name}.tar.gz"
    shutil.make_archive(
        str(project_dir / "dist" / release_name),
        'gztar',
        str(release_dir.parent),
        release_name
    )
    print_success(f"Created: dist/{archive_name}")
    
    # Cleanup temp directory
    shutil.rmtree(release_dir)
    
    print()
    print_success(f"Release package ready: dist/{archive_name}")
    print()
    print("Upload to GitHub releases:")
    print(f"  gh release create v{version} dist/{archive_name}")
    print()

def package_windows():
    """Package Windows release"""
    print_info("Windows packaging not yet implemented")
    print("Build with Visual Studio and package manually for now")

def package_macos():
    """Package macOS release"""
    print_info("macOS packaging not yet implemented")
    print("Build with Xcode and create .dmg manually for now")

def main():
    system = platform.system()
    
    if system == "Linux":
        package_linux()
    elif system == "Windows":
        package_windows()
    elif system == "Darwin":
        package_macos()
    else:
        print_error(f"Unsupported platform: {system}")

if __name__ == "__main__":
    main()
