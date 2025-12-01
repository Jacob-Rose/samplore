#!/usr/bin/env python3
"""
Samplore Run Script
Launches the built application
"""

import os
import platform
import subprocess
import sys
from pathlib import Path


SCRIPT_DIR = Path(__file__).parent.resolve()
PROJECT_ROOT = SCRIPT_DIR.parent
BUILDS_DIR = PROJECT_ROOT / "Builds"
ENV_FILE = PROJECT_ROOT / ".env"


def load_env_file():
    """Load environment variables from .env file."""
    if not ENV_FILE.exists():
        return {}
    
    env_vars = {}
    with open(ENV_FILE, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            if '=' in line:
                key, value = line.split('=', 1)
                env_vars[key.strip()] = value.strip()
    
    return env_vars


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


def run_linux():
    """Run Linux binary."""
    binary_path = BUILDS_DIR / "LinuxMakefile" / "build" / "Samplore"
    
    if not binary_path.exists():
        print(f"✗ Binary not found at {binary_path}")
        print()
        print("Build the application first:")
        print("  ./scripts/build.sh")
        return 1
    
    print(f"Launching Samplore...")
    print()
    
    try:
        subprocess.run([str(binary_path)], check=True)
        return 0
    except subprocess.CalledProcessError as e:
        print(f"✗ Application exited with error code {e.returncode}")
        return e.returncode
    except KeyboardInterrupt:
        print("\nApplication terminated by user")
        return 0


def run_macos(config):
    """Run macOS application."""
    app_path = BUILDS_DIR / "MacOSX" / "build" / config / "Samplore.app"
    
    if not app_path.exists():
        print(f"✗ Application not found at {app_path}")
        print()
        print("Build the application first:")
        print("  ./scripts/build.sh")
        return 1
    
    print(f"Launching Samplore...")
    print()
    
    try:
        subprocess.run(["open", str(app_path)], check=True)
        return 0
    except subprocess.CalledProcessError as e:
        print(f"✗ Failed to launch application: {e}")
        return 1


def run_windows(config):
    """Run Windows executable."""
    # Try VS2022 first

    win_paths = [
        BUILDS_DIR / "VisualStudio2022" / "x64" / config / "App" / "Samplore.exe",
        BUILDS_DIR / "VisualStudio2022" / "x64" / config / "Samplore.exe",
        BUILDS_DIR / "VisualStudio2019" / "x64" / config / "App" / "Samplore.exe",
        BUILDS_DIR / "VisualStudio2019" / "x64" / config / "Samplore.exe"
    ]

    found_path = None

    for exe_path in win_paths:
        if exe_path.exists():
            found_path = exe_path
            break
    
    if not found_path:
        print(f"✗ Executable not found")
        print()
        print("Expected locations:")
        for exe_path in win_paths:
            print(f" {exe_path}")
        print()
        print("Build the application first:")
        print("  ./scripts/build.sh")
        return 1
    
    print(f"Launching Samplore...")
    print()
    
    try:
        subprocess.run([str(exe_path)], check=True)
        return 0
    except subprocess.CalledProcessError as e:
        print(f"✗ Application exited with error code {e.returncode}")
        return e.returncode
    except KeyboardInterrupt:
        print("\nApplication terminated by user")
        return 0


def main():
    print("=" * 70)
    print("Samplore Launcher")
    print("=" * 70)
    print()
    
    # Load config
    env_vars = load_env_file()
    config = env_vars.get('BUILD_CONFIG', 'Release')
    
    plat = get_platform()
    print(f"Platform: {plat}")
    print(f"Config:   {config}")
    print()
    
    if plat == "linux":
        return run_linux()
    elif plat == "macos":
        return run_macos(config)
    elif plat == "windows":
        return run_windows(config)
    else:
        print(f"✗ Unsupported platform: {plat}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
