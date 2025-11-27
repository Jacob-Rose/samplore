#!/usr/bin/env python3
"""
Samplore First-Time Setup Script
Interactive wizard for configuring the build environment
"""

import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path


SCRIPT_DIR = Path(__file__).parent.resolve()
PROJECT_ROOT = SCRIPT_DIR.parent
ENV_FILE = PROJECT_ROOT / ".env"
ENV_EXAMPLE = PROJECT_ROOT / ".env.example"


def get_platform():
    """Detect the current platform."""
    system = platform.system().lower()
    if system == "darwin":
        return "macos"
    elif system == "windows":
        return "windows"
    elif system == "linux":
        return "linux"
    else:
        return system


def get_input(prompt, default=None):
    """Get user input with optional default."""
    if default:
        user_input = input(f"{prompt} [{default}]: ").strip()
        return user_input if user_input else default
    else:
        return input(f"{prompt}: ").strip()


def get_yes_no(prompt, default_yes=False):
    """Get yes/no input from user."""
    default = "Y/n" if default_yes else "y/N"
    response = input(f"{prompt} [{default}] ").strip().lower()
    
    if not response:
        return default_yes
    
    return response in ('y', 'yes')


def find_juce_installations():
    """Search for common JUCE installation locations."""
    suggestions = []
    home = Path.home()
    
    search_paths = [
        home / "JUCE",
        home / "Documents" / "JUCE",
        home / "juce",
        home / "Documents" / "juce",
        Path("/usr/local/JUCE"),
        Path("C:/JUCE") if platform.system() == "Windows" else None,
    ]
    
    for path in search_paths:
        if path and path.exists() and (path / "modules").exists():
            suggestions.append(path)
    
    return suggestions


def check_linux_dependencies():
    """Check for required Linux build dependencies."""
    required_packages = [
        "libfreetype6-dev",
        "libwebkit2gtk-4.1-dev",
        "libgtk-3-dev",
        "libasound2-dev",
        "libcurl4-openssl-dev"
    ]
    
    missing = []
    
    for pkg in required_packages:
        result = subprocess.run(
            ["dpkg", "-l", pkg],
            capture_output=True,
            text=True
        )
        if result.returncode != 0:
            missing.append(pkg)
    
    return missing


def install_linux_dependencies(packages):
    """Install missing Linux dependencies."""
    print()
    print("Installing dependencies...")
    
    try:
        # Update package list
        subprocess.run(["sudo", "apt-get", "update"], check=True)
        # Install packages
        subprocess.run(
            ["sudo", "apt-get", "install", "-y"] + packages,
            check=True
        )
        print("✓ Dependencies installed")
        return True
    except subprocess.CalledProcessError:
        print("✗ Failed to install dependencies")
        return False


def update_env_file(key, value):
    """Update a key=value pair in the .env file."""
    if not ENV_FILE.exists():
        return False
    
    # Read all lines
    with open(ENV_FILE, 'r') as f:
        lines = f.readlines()
    
    # Update the specific key
    updated = False
    for i, line in enumerate(lines):
        if line.strip().startswith(f"{key}="):
            lines[i] = f"{key}={value}\n"
            updated = True
            break
    
    # Write back
    if updated:
        with open(ENV_FILE, 'w') as f:
            f.writelines(lines)
    
    return updated


def main():
    print("=" * 70)
    print("Samplore - First-Time Setup")
    print("=" * 70)
    print()
    print("This script will help you configure Samplore for building.")
    print()
    
    # Detect platform
    plat = get_platform()
    platform_names = {
        "linux": "Linux",
        "macos": "macOS",
        "windows": "Windows"
    }
    platform_name = platform_names.get(plat, f"Unknown ({plat})")
    
    print(f"Detected platform: {platform_name}")
    print()
    
    # Check for existing .env
    if ENV_FILE.exists():
        print("✓ Found existing .env file")
        print()
        if not get_yes_no("Do you want to reconfigure?", default_yes=False):
            print()
            print("Keeping existing configuration.")
            print()
            print("To build the project, run:")
            print("  python3 scripts/build.py")
            print("  # or")
            print("  ./scripts/build.sh")
            return 0
        print()
    else:
        print("Creating .env file from template...")
        shutil.copy(ENV_EXAMPLE, ENV_FILE)
        print("✓ Created .env")
        print()
    
    # Prompt for JUCE path
    print("Please enter the path to your JUCE installation:")
    print("Examples:")
    print("  Linux:   /home/username/JUCE")
    print("  macOS:   /Users/username/JUCE")
    print("  Windows: C:/JUCE")
    print()
    
    # Find and suggest JUCE installations
    suggestions = find_juce_installations()
    if suggestions:
        print("Found possible JUCE installations:")
        for suggestion in suggestions:
            print(f"  - {suggestion}")
        print()
    
    # Get JUCE path
    juce_path_str = get_input("JUCE Path")
    
    if not juce_path_str:
        print("✗ JUCE path cannot be empty")
        return 1
    
    # Expand ~ to home directory
    juce_path_str = os.path.expanduser(juce_path_str)
    juce_path = Path(juce_path_str)
    
    # Validate path
    if not juce_path.exists():
        print()
        print(f"⚠ Warning: Directory does not exist: {juce_path}")
        print()
        if not get_yes_no("Continue anyway?", default_yes=False):
            return 1
    
    if juce_path.exists() and not (juce_path / "modules").exists():
        print()
        print("⚠ Warning: No 'modules' directory found in JUCE path")
        print(f"   Expected: {juce_path / 'modules'}")
        print()
    
    # Update .env with JUCE path
    print()
    print("Updating .env file...")
    update_env_file("JUCE_PATH", str(juce_path))
    print("✓ Updated JUCE_PATH in .env")
    print()
    
    # Ask about build configuration
    print("Select build configuration:")
    print("  1) Release (optimized, recommended)")
    print("  2) Debug (includes debug symbols)")
    
    build_choice = get_input("Choice", default="1")
    
    if build_choice == "2":
        build_config = "Debug"
    else:
        build_config = "Release"
    
    update_env_file("BUILD_CONFIG", build_config)
    print(f"✓ Set BUILD_CONFIG={build_config}")
    print()
    
    # Check dependencies on Linux
    if plat == "linux":
        print("Checking Linux dependencies...")
        print()
        
        missing = check_linux_dependencies()
        
        if missing:
            print("⚠ Missing required packages:")
            for pkg in missing:
                print(f"  - {pkg}")
            print()
            print("Install with:")
            print(f"  sudo apt-get install {' '.join(missing)}")
            print()
            
            if get_yes_no("Install now?", default_yes=True):
                if not install_linux_dependencies(missing):
                    print()
                    print("Warning: Continuing without all dependencies")
                    print("Build may fail")
        else:
            print("✓ All dependencies installed")
        
        print()
    
    # Run configure script
    print("Running configuration script...")
    print()
    
    try:
        result = subprocess.run(
            [sys.executable, str(SCRIPT_DIR / "configure.py")],
            check=True
        )
    except subprocess.CalledProcessError:
        print()
        print("✗ Configuration failed")
        return 1
    
    # Success!
    print()
    print("=" * 70)
    print("Setup Complete!")
    print("=" * 70)
    print()
    print("Your configuration:")
    print(f"  JUCE Path:    {juce_path}")
    print(f"  Build Config: {build_config}")
    print()
    print("Next steps:")
    print("  1. Build the project:")
    print("       python3 scripts/build.py")
    print("       # or")
    print("       ./scripts/build.sh")
    print()
    print("  2. Run the application:")
    print("       python3 scripts/run.py")
    print("       # or")
    print("       ./scripts/run.sh")
    print()
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
