#!/usr/bin/env python3
"""
Samplore Configuration Script
Configures the build environment and generates platform-specific build files.
"""

import argparse
import os
import platform
import shutil
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path


SCRIPT_DIR = Path(__file__).parent.resolve()
PROJECT_ROOT = SCRIPT_DIR.parent
JUCER_FILE = PROJECT_ROOT / "Samplore.jucer"
ENV_FILE = PROJECT_ROOT / ".env"
ENV_EXAMPLE_FILE = PROJECT_ROOT / ".env.example"


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


def load_env_file(env_path):
    """Load environment variables from .env file."""
    if not env_path.exists():
        return {}
    
    env_vars = {}
    with open(env_path, 'r') as f:
        for line in f:
            line = line.strip()
            # Skip comments and empty lines
            if not line or line.startswith('#'):
                continue
            # Parse KEY=VALUE
            if '=' in line:
                key, value = line.split('=', 1)
                env_vars[key.strip()] = value.strip()
    
    return env_vars


def normalize_path(path_str):
    """Normalize path to use forward slashes (cross-platform compatible)."""
    # Convert backslashes to forward slashes
    path_str = path_str.replace('\\', '/')
    # Expand user home directory
    if path_str.startswith('~'):
        path_str = str(Path(path_str).expanduser())
        path_str = path_str.replace('\\', '/')
    return path_str


def update_jucer_file(jucer_path, juce_path):
    """Update MODULEPATH entries in the .jucer file."""
    print(f"Reading {jucer_path.name}...")
    
    # Parse XML
    tree = ET.parse(jucer_path)
    root = tree.getroot()
    
    # Normalize the JUCE path
    juce_modules_path = normalize_path(juce_path)
    if not juce_modules_path.endswith('/modules'):
        juce_modules_path += '/modules'
    
    print(f"Setting JUCE modules path to: {juce_modules_path}")
    
    # Find all MODULEPATH elements in all EXPORTFORMATS
    exportformats = root.find('EXPORTFORMATS')
    if exportformats is None:
        print("Error: No EXPORTFORMATS section found in .jucer file")
        return False
    
    changes_made = 0
    for exporter in exportformats:
        exporter_name = exporter.tag
        modulepaths = exporter.find('MODULEPATHS')
        
        if modulepaths is not None:
            for modulepath in modulepaths.findall('MODULEPATH'):
                old_path = modulepath.get('path', '')
                # Update the path
                modulepath.set('path', juce_modules_path)
                if old_path != juce_modules_path:
                    changes_made += 1
                    print(f"  Updated {exporter_name}/{modulepath.get('id')}")
    
    if changes_made > 0:
        # Write back to file
        print(f"\nWriting changes to {jucer_path.name}...")
        tree.write(jucer_path, encoding='UTF-8', xml_declaration=True)
        print(f"✓ Updated {changes_made} module path(s)")
        return True
    else:
        print("✓ All module paths already up to date")
        return True


def find_projucer(juce_path):
    """Try to find the Projucer executable in the JUCE installation."""
    juce_root = Path(juce_path).expanduser()
    
    # Common Projucer locations
    search_paths = [
        # Pre-built binaries
        juce_root / "Projucer.app/Contents/MacOS/Projucer",  # macOS
        juce_root / "Projucer",  # Linux
        juce_root / "Projucer.exe",  # Windows
        # extras folder
        juce_root / "extras/Projucer/Builds/MacOSX/build/Release/Projucer.app/Contents/MacOS/Projucer",
        juce_root / "extras/Projucer/Builds/LinuxMakefile/build/Projucer",
        juce_root / "extras/Projucer/Builds/VisualStudio2022/x64/Release/App/Projucer.exe",
        juce_root / "extras/Projucer/Builds/VisualStudio2019/x64/Release/App/Projucer.exe",
        # CMake builds
        juce_root / "cmake-build/extras/Projucer/Projucer_artefacts/Projucer",
        juce_root / "cmake-build/extras/Projucer/Projucer_artefacts/Release/Projucer.exe",
        juce_root / "cmake-build-release/extras/Projucer/Projucer_artefacts/Projucer",
    ]
    
    for path in search_paths:
        if path.exists():
            return path
    
    # Try to find in PATH
    projucer_in_path = shutil.which("Projucer")
    if projucer_in_path:
        return Path(projucer_in_path)
    
    return None


def build_projucer(juce_path, plat):
    """Build Projucer from source for the current platform."""
    juce_root = Path(juce_path).expanduser()
    projucer_dir = juce_root / "extras/Projucer"
    
    if not projucer_dir.exists():
        print(f"✗ Projucer source not found at {projucer_dir}")
        return None
    
    print(f"Building Projucer from source...")
    print(f"Source: {projucer_dir}")
    print()
    
    if plat == "linux":
        build_dir = projucer_dir / "Builds/LinuxMakefile"
        makefile = build_dir / "Makefile"
        
        if not makefile.exists():
            print(f"✗ Projucer Makefile not found at {makefile}")
            print("  Build files need to be generated first (bootstrapping problem)")
            return None
        
        # Build with make
        jobs = os.cpu_count() or 4
        print(f"Running: make CONFIG=Release -j{jobs}")
        print(f"In: {build_dir}")
        print("-" * 60)
        
        result = subprocess.run(
            ["make", "CONFIG=Release", f"-j{jobs}"],
            cwd=build_dir
        )
        
        if result.returncode != 0:
            print("✗ Projucer build failed")
            return None
        
        output_binary = build_dir / "build/Projucer"
        if output_binary.exists():
            print(f"✓ Projucer built successfully: {output_binary}")
            return output_binary
        else:
            print(f"✗ Build completed but binary not found at {output_binary}")
            return None
    
    elif plat == "macos":
        build_dir = projucer_dir / "Builds/MacOSX"
        
        # Find the xcodeproj
        xcodeproj = None
        if build_dir.exists():
            for item in build_dir.iterdir():
                if item.suffix == ".xcodeproj":
                    xcodeproj = item
                    break
        
        if not xcodeproj:
            print(f"✗ Projucer Xcode project not found in {build_dir}")
            print("  Build files need to be generated first (bootstrapping problem)")
            return None
        
        # Build with xcodebuild
        print(f"Running: xcodebuild -configuration Release")
        print(f"In: {build_dir}")
        print("-" * 60)
        
        result = subprocess.run(
            [
                "xcodebuild",
                "-project", str(xcodeproj),
                "-configuration", "Release",
                "-jobs", str(os.cpu_count() or 4),
                "build",
            ],
            cwd=build_dir
        )
        
        if result.returncode != 0:
            print("✗ Projucer build failed")
            return None
        
        output_binary = build_dir / "build/Release/Projucer.app/Contents/MacOS/Projucer"
        if output_binary.exists():
            print(f"✓ Projucer built successfully: {output_binary}")
            return output_binary
        else:
            print(f"✗ Build completed but binary not found at {output_binary}")
            return None
    
    elif plat == "windows":
        # Try VS2022 first, then VS2019
        build_dirs = [
            projucer_dir / "Builds/VisualStudio2022",
            projucer_dir / "Builds/VisualStudio2019",
        ]
        
        build_dir = None
        sln_file = None
        
        for bd in build_dirs:
            if bd.exists():
                for item in bd.iterdir():
                    if item.suffix == ".sln":
                        build_dir = bd
                        sln_file = item
                        break
                if sln_file:
                    break
        
        if not sln_file or not build_dir:
            print(f"✗ Projucer Visual Studio solution not found")
            print("  Build files need to be generated first (bootstrapping problem)")
            return None
        
        # Try to find MSBuild
        msbuild_paths = [
            r"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
            r"C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
            r"C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
            r"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe",
            r"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe",
            "msbuild",  # Try PATH
        ]
        
        msbuild = None
        for path in msbuild_paths:
            if Path(path).exists() or shutil.which(path):
                msbuild = path
                break
        
        if not msbuild:
            print("✗ MSBuild not found. Install Visual Studio or add MSBuild to PATH.")
            return None
        
        # Build with MSBuild
        print(f"Running: MSBuild /p:Configuration=Release")
        print(f"In: {build_dir}")
        print("-" * 60)
        
        result = subprocess.run(
            [
                msbuild,
                str(sln_file),
                "/p:Configuration=Release",
                "/p:Platform=x64",
                "/m",  # Parallel build
            ],
            cwd=build_dir
        )
        
        if result.returncode != 0:
            print("✗ Projucer build failed")
            return None
        
        output_binary = build_dir / "x64/Release/App/Projucer.exe"
        if output_binary.exists():
            print(f"✓ Projucer built successfully: {output_binary}")
            return output_binary
        else:
            print(f"✗ Build completed but binary not found at {output_binary}")
            return None
    
    else:
        print(f"✗ Unsupported platform: {plat}")
        return None


def generate_build_files(jucer_path, juce_path, plat):
    """Generate platform-specific build files using Projucer."""
    projucer = find_projucer(juce_path)
    
    if not projucer:
        print("\n" + "!" * 70)
        print("WARNING: Projucer not found!")
        print("!" * 70)
        print()
        print("Attempting to build Projucer from source...")
        print()
        
        # Try to build Projucer automatically
        projucer = build_projucer(juce_path, plat)
        
        if not projucer:
            print()
            print("✗ Failed to build Projucer automatically")
            print()
            print("Manual options:")
            print("  1. Download pre-built Projucer from https://juce.com/")
            print("  2. Open Samplore.jucer in Projucer")
            print("  3. Click 'Save Project' to generate build files")
            print()
            return False
        
        print()
        print("✓ Successfully built Projucer!")
        print()
    
    print()
    print(f"Found Projucer: {projucer}")
    print(f"Generating build files for {jucer_path.name}...")
    print()
    
    try:
        result = subprocess.run(
            [str(projucer), "--resave", str(jucer_path)],
            capture_output=True,
            text=True,
            timeout=60
        )
        
        if result.returncode == 0:
            print("✓ Build files generated successfully!")
            return True
        else:
            print(f"✗ Projucer failed with exit code {result.returncode}")
            if result.stderr:
                print("Error output:")
                print(result.stderr)
            return False
            
    except subprocess.TimeoutExpired:
        print("✗ Projucer timed out (took longer than 60 seconds)")
        return False
    except Exception as e:
        print(f"✗ Failed to run Projucer: {e}")
        return False


def check_dependencies(plat):
    """Check if platform-specific build dependencies are installed."""
    if plat == "linux":
        print("\nChecking Linux dependencies...")
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
        
        if missing:
            print("\n⚠ Missing dependencies:")
            for pkg in missing:
                print(f"  - {pkg}")
            print("\nInstall with:")
            print(f"  sudo apt-get install {' '.join(missing)}")
            return False
        else:
            print("✓ All Linux dependencies installed")
            return True
    
    return True


def main():
    parser = argparse.ArgumentParser(
        description="Samplore Build Configuration Tool",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 scripts/configure.py                  # Configure and generate build files
  python3 scripts/configure.py --no-generate    # Only update .jucer, don't generate
  python3 scripts/configure.py --check-deps     # Check platform dependencies
        """
    )
    
    parser.add_argument(
        "--no-generate",
        action="store_true",
        help="Skip generating build files (only update .jucer file)"
    )
    parser.add_argument(
        "--check-deps",
        action="store_true",
        help="Check for platform-specific build dependencies"
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Force regeneration even if paths haven't changed"
    )
    
    args = parser.parse_args()
    
    print("=" * 70)
    print("Samplore Build Configuration")
    print("=" * 70)
    print()
    
    plat = get_platform()
    print(f"Platform: {plat}")
    print(f"Project:  {PROJECT_ROOT}")
    print()
    
    # Check dependencies if requested
    if args.check_deps:
        check_dependencies(plat)
        print()
    
    # Check if .env exists
    if not ENV_FILE.exists():
        print(f"Error: .env file not found at {ENV_FILE}")
        print()
        print("Please create a .env file from .env.example:")
        print(f"  cp {ENV_EXAMPLE_FILE} {ENV_FILE}")
        print(f"  # Edit {ENV_FILE} and set JUCE_PATH")
        print()
        return 1
    
    # Load environment variables
    print(f"Loading configuration from .env...")
    env_vars = load_env_file(ENV_FILE)
    
    # Get JUCE path
    juce_path = env_vars.get('JUCE_PATH')
    if not juce_path or juce_path == '/path/to/juce':
        print()
        print("Error: JUCE_PATH not configured in .env")
        print(f"Please edit {ENV_FILE} and set JUCE_PATH to your JUCE installation")
        print()
        print("Example:")
        print("  JUCE_PATH=/home/username/JUCE")
        print()
        return 1
    
    # Validate JUCE path exists
    juce_path_obj = Path(juce_path).expanduser()
    if not juce_path_obj.exists():
        print()
        print(f"Warning: JUCE path does not exist: {juce_path_obj}")
        print("Make sure JUCE is installed at this location.")
        print()
        if not args.force:
            response = input("Continue anyway? [y/N] ").strip().lower()
            if response != 'y':
                return 1
    
    # Check for modules directory
    modules_path = juce_path_obj / "modules"
    if not modules_path.exists():
        print()
        print(f"Warning: JUCE modules directory not found: {modules_path}")
        print("Make sure your JUCE installation includes the modules directory.")
        print()
        if not args.force:
            response = input("Continue anyway? [y/N] ").strip().lower()
            if response != 'y':
                return 1
    
    print()
    print("Configuration:")
    print(f"  JUCE Path: {juce_path}")
    print()
    
    # Update .jucer file
    if not update_jucer_file(JUCER_FILE, juce_path):
        return 1
    
    # Generate build files
    if not args.no_generate:
        print()
        success = generate_build_files(JUCER_FILE, juce_path, plat)
        if not success:
            print("\nContinuing without generated build files...")
            print("You'll need to generate them manually using Projucer.")
    
    print()
    print("=" * 70)
    print("Configuration complete!")
    print("=" * 70)
    print()
    print("Next steps:")
    if args.no_generate:
        print("  1. Open Projucer and load Samplore.jucer")
        print("  2. Save the project to regenerate platform-specific build files")
    print("  - Build the project:")
    print("      ./scripts/build.sh")
    print("  - Or use Python build script:")
    print("      python3 scripts/build.py")
    print()
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
