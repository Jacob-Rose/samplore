#!/usr/bin/env python3
"""
Samplore Clean Script
Cleans build artifacts for the current platform
"""

import argparse
import platform
import shutil
import subprocess
import sys
from pathlib import Path


SCRIPT_DIR = Path(__file__).parent.resolve()
PROJECT_ROOT = SCRIPT_DIR.parent
BUILDS_DIR = PROJECT_ROOT / "Builds"


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


def clean_linux():
    """Clean Linux build artifacts."""
    print("Cleaning Linux build...")
    build_dir = BUILDS_DIR / "LinuxMakefile"
    
    # Try to run make clean if Makefile exists
    makefile = build_dir / "Makefile"
    if makefile.exists():
        try:
            subprocess.run(
                ["make", "clean"],
                cwd=build_dir,
                check=True
            )
            print("✓ Linux build artifacts cleaned")
        except subprocess.CalledProcessError:
            print("⚠ Make clean failed, removing build directory manually")
    
    # Remove build directory
    build_output = build_dir / "build"
    if build_output.exists():
        shutil.rmtree(build_output)
        print("✓ Removed build directory")
    
    return 0


def clean_macos():
    """Clean macOS build artifacts."""
    print("Cleaning macOS build...")
    build_dir = BUILDS_DIR / "MacOSX"
    
    build_output = build_dir / "build"
    if build_output.exists():
        shutil.rmtree(build_output)
        print("✓ macOS build artifacts cleaned")
    else:
        print("No build artifacts found")
    
    return 0


def clean_windows():
    """Clean Windows build artifacts."""
    print("Cleaning Windows build...")
    
    cleaned = False
    
    # Clean VS2022
    vs2022_dir = BUILDS_DIR / "VisualStudio2022" / "x64"
    if vs2022_dir.exists():
        shutil.rmtree(vs2022_dir)
        print("✓ Removed VisualStudio2022/x64/")
        cleaned = True
    
    # Clean VS2019
    vs2019_dir = BUILDS_DIR / "VisualStudio2019" / "x64"
    if vs2019_dir.exists():
        shutil.rmtree(vs2019_dir)
        print("✓ Removed VisualStudio2019/x64/")
        cleaned = True
    
    if not cleaned:
        print("No build artifacts found")
    
    return 0


def clean_all():
    """Clean all platform build artifacts."""
    print("Cleaning all platform builds...")
    print()
    
    # Clean Linux
    if (BUILDS_DIR / "LinuxMakefile").exists():
        clean_linux()
        print()
    
    # Clean macOS
    if (BUILDS_DIR / "MacOSX").exists():
        clean_macos()
        print()
    
    # Clean Windows
    if (BUILDS_DIR / "VisualStudio2022").exists() or (BUILDS_DIR / "VisualStudio2019").exists():
        clean_windows()
        print()
    
    return 0


def main():
    parser = argparse.ArgumentParser(
        description="Samplore Build Artifact Cleaner",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 scripts/clean.py           # Clean current platform
  python3 scripts/clean.py --all     # Clean all platforms
        """
    )
    
    parser.add_argument(
        "--all",
        action="store_true",
        help="Clean build artifacts for all platforms"
    )
    
    args = parser.parse_args()
    
    print("=" * 70)
    print("Samplore Clean Script")
    print("=" * 70)
    
    if args.all:
        return clean_all()
    
    plat = get_platform()
    print(f"Platform: {plat}")
    print()
    
    if plat == "linux":
        return clean_linux()
    elif plat == "macos":
        return clean_macos()
    elif plat == "windows":
        return clean_windows()
    else:
        print(f"Error: Unsupported platform: {plat}")
        return 1
    
    print()
    print("Clean complete!")
    return 0


if __name__ == "__main__":
    sys.exit(main())
