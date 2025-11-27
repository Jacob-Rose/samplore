#!/usr/bin/env python3
"""
Samplore Cross-Platform Build Script
Supports: Windows, macOS, Linux
"""

import argparse
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path


# Project paths relative to this script
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
        raise RuntimeError(f"Unsupported platform: {system}")


def get_build_dir(plat):
    """Get the build directory for the current platform."""
    dirs = {
        "linux": BUILDS_DIR / "LinuxMakefile",
        "macos": BUILDS_DIR / "MacOSX",
        "windows": BUILDS_DIR / "VisualStudio2022",
    }
    # Fallback for Windows - try VS2019 if VS2022 doesn't exist
    if plat == "windows" and not dirs["windows"].exists():
        alt = BUILDS_DIR / "VisualStudio2019"
        if alt.exists():
            return alt
    return dirs.get(plat)


def get_output_binary(plat, config):
    """Get the expected output binary path."""
    build_dir = get_build_dir(plat)
    if not build_dir:
        return None
    if plat == "linux":
        return build_dir / "build" / "SamplifyPlus"
    elif plat == "macos":
        return build_dir / "build" / config / "SamplifyPlus.app"
    elif plat == "windows":
        return build_dir / f"x64/{config}/SamplifyPlus.exe"
    return None


def run_command(cmd, cwd=None, env=None):
    """Run a command and stream output."""
    print(f"Running: {' '.join(str(c) for c in cmd)}")
    print(f"In: {cwd or os.getcwd()}")
    print("-" * 60)

    process = subprocess.Popen(
        cmd,
        cwd=cwd,
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
    )

    if process.stdout:
        for line in process.stdout:
            print(line, end="")

    process.wait()
    return process.returncode


def clean_build(plat):
    """Clean build artifacts."""
    build_dir = get_build_dir(plat)
    if not build_dir or not build_dir.exists():
        print(f"Build directory not found: {build_dir}")
        return 1

    if plat == "linux":
        return run_command(["make", "clean"], cwd=build_dir)
    elif plat == "macos":
        build_output = build_dir / "build"
        if build_output.exists():
            shutil.rmtree(build_output)
            print(f"Removed: {build_output}")
        return 0
    elif plat == "windows":
        return run_command(
            ["msbuild", "/t:Clean", "/p:Configuration=Release"],
            cwd=build_dir
        )
    return 1


def build_linux(config, jobs):
    """Build on Linux using make."""
    build_dir = get_build_dir("linux")
    if not build_dir or not build_dir.exists():
        print(f"Error: Linux build directory not found: {build_dir}")
        print("Run: python3 scripts/configure.py")
        return 1

    cmd = ["make", f"CONFIG={config}", f"-j{jobs}"]
    return run_command(cmd, cwd=build_dir)


def build_macos(config):
    """Build on macOS using xcodebuild."""
    build_dir = get_build_dir("macos")
    if not build_dir or not build_dir.exists():
        print(f"Error: macOS build directory not found: {build_dir}")
        print("Run: python3 scripts/configure.py")
        return 1

    # Find the xcodeproj
    xcodeproj = None
    for item in build_dir.iterdir():
        if item.suffix == ".xcodeproj":
            xcodeproj = item
            break

    if not xcodeproj:
        print("Error: No .xcodeproj found in build directory")
        return 1

    cmd = [
        "xcodebuild",
        "-project", str(xcodeproj),
        "-configuration", config,
        "-jobs", str(os.cpu_count() or 4),
        "build",
    ]
    return run_command(cmd, cwd=build_dir)


def build_windows(config):
    """Build on Windows using MSBuild."""
    build_dir = get_build_dir("windows")
    if not build_dir or not build_dir.exists():
        print(f"Error: Windows build directory not found: {build_dir}")
        print("Run: python3 scripts/configure.py")
        return 1

    # Find the solution file
    sln_file = None
    for item in build_dir.iterdir():
        if item.suffix == ".sln":
            sln_file = item
            break

    if not sln_file:
        print("Error: No .sln file found in build directory")
        return 1

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
        print("Error: MSBuild not found. Install Visual Studio or add MSBuild to PATH.")
        return 1

    cmd = [
        msbuild,
        str(sln_file),
        f"/p:Configuration={config}",
        "/p:Platform=x64",
        "/m",  # Parallel build
    ]
    return run_command(cmd, cwd=build_dir)


def build(plat, config, jobs):
    """Run the build for the specified platform."""
    print(f"Building SamplifyPlus for {plat} ({config})")
    print("=" * 60)

    if plat == "linux":
        return build_linux(config, jobs)
    elif plat == "macos":
        return build_macos(config)
    elif plat == "windows":
        return build_windows(config)
    else:
        print(f"Unsupported platform: {plat}")
        return 1


def run_app(plat, config):
    """Run the built application."""
    binary = get_output_binary(plat, config)
    if not binary or not binary.exists():
        print(f"Error: Binary not found at {binary}")
        print("Build the project first.")
        return 1

    print(f"Running: {binary}")
    if plat == "macos":
        return run_command(["open", str(binary)])
    else:
        return run_command([str(binary)])


def main():
    parser = argparse.ArgumentParser(
        description="SamplifyPlus Cross-Platform Build Script",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python build.py                    # Build Release
  python build.py --config Debug     # Build Debug
  python build.py --clean            # Clean build artifacts
  python build.py --clean --build    # Clean then build
  python build.py --run              # Build and run
  python build.py -j8                # Build with 8 parallel jobs
        """
    )

    parser.add_argument(
        "--config", "-c",
        choices=["Debug", "Release"],
        default="Release",
        help="Build configuration (default: Release)"
    )
    parser.add_argument(
        "--clean",
        action="store_true",
        help="Clean build artifacts"
    )
    parser.add_argument(
        "--build", "-b",
        action="store_true",
        help="Build the project (default if no other action specified)"
    )
    parser.add_argument(
        "--run", "-r",
        action="store_true",
        help="Run the application after building"
    )
    parser.add_argument(
        "-j", "--jobs",
        type=int,
        default=os.cpu_count() or 4,
        help=f"Number of parallel jobs (default: {os.cpu_count() or 4})"
    )
    parser.add_argument(
        "--platform", "-p",
        choices=["linux", "macos", "windows"],
        default=None,
        help="Target platform (default: auto-detect)"
    )

    args = parser.parse_args()

    # Auto-detect platform if not specified
    plat = args.platform or get_platform()
    print(f"Platform: {plat}")
    print(f"Project: {PROJECT_ROOT}")
    print()

    # Default to build if no action specified
    if not args.clean and not args.build and not args.run:
        args.build = True

    result = 0

    # Clean
    if args.clean:
        print("Cleaning...")
        result = clean_build(plat)
        if result != 0:
            print("Clean failed!")
            return result
        print()

    # Build
    if args.build or args.run:
        result = build(plat, args.config, args.jobs)
        if result != 0:
            print("\nBuild failed!")
            return result
        print("\nBuild successful!")

        # Show output location
        binary = get_output_binary(plat, args.config)
        if binary and binary.exists():
            print(f"Output: {binary}")

    # Run
    if args.run and result == 0:
        print()
        result = run_app(plat, args.config)

    return result


if __name__ == "__main__":
    sys.exit(main())
