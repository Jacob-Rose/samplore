#!/usr/bin/env python3
"""
Samplore Native Build Script
Builds Samplore for the current platform (Linux, macOS, or Windows)
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
        return build_dir / "build" / "Samplore"
    elif plat == "macos":
        return build_dir / "build" / config / "Samplore.app"
    elif plat == "windows":
        return build_dir / f"x64/{config}/Samplore.exe"
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


def build_linux(config, jobs, use_cmake=False, build_dir=None):
    """Build on Linux using make or CMake."""
    if use_cmake:
        return build_linux_cmake(config, jobs, build_dir)
    else:
        return build_linux_projucer(config, jobs, build_dir)


def build_linux_cmake(config, jobs, build_dir):
    """Build on Linux using CMake."""
    if not build_dir:
        build_dir = PROJECT_ROOT / "build"
    else:
        build_dir = Path(build_dir)
    build_dir.mkdir(exist_ok=True)
    
    # Configure with CMake (use project root as source)
    configure_result = run_command(
        ["cmake", "-S", str(PROJECT_ROOT), "-B", str(build_dir), "-DCMAKE_BUILD_TYPE=" + config],
        cwd=PROJECT_ROOT
    )
    if configure_result != 0:
        print(f"[ERROR] CMake configuration failed")
        return configure_result
    
    # Build with CMake
    build_result = run_command(
        ["cmake", "--build", str(build_dir), "--parallel", str(os.cpu_count() or 4)],
        cwd=PROJECT_ROOT
    )
    
    if build_result != 0:
        print(f"[ERROR] CMake build failed")
        return build_result
    
    print("[OK] CMake build successful")
    return 0


def build_linux_projucer(config, jobs, build_dir):
    """Build on Linux using Projucer (original method)."""
    projucer_build_dir = get_build_dir("linux")

    if not projucer_build_dir or not projucer_build_dir.exists() or not (projucer_build_dir / "Makefile").exists():
        print(f"[WARNING] Linux build files not found")
        print("Generating build files...")
        result = subprocess.run(
            [sys.executable, str(SCRIPT_DIR / "configure.py")],
            cwd=PROJECT_ROOT
        )
        if result.returncode != 0:
            print("[ERROR] Failed to generate build files")
            return 1

        # Re-check for Makefile
        projucer_build_dir = get_build_dir("linux")

    cmd = ["make", f"CONFIG={config}", f"-j{jobs}"]
    result = run_command(cmd, cwd=projucer_build_dir)

    # If using custom build directory, copy results there
    if result == 0 and build_dir:
        custom_build_dir = Path(build_dir)
        custom_build_dir.mkdir(exist_ok=True)

        # Copy the built executable
        output_binary = get_output_binary("linux", config)
        if output_binary and output_binary.exists():
            custom_binary = custom_build_dir / output_binary.name
            import shutil
            shutil.copy2(str(output_binary), str(custom_binary))
            print(f"[OK] Copied to: {custom_binary}")

    return result


def build_macos(config):
    """Build on macOS using xcodebuild."""
    build_dir = get_build_dir("macos")
    
    # Find the xcodeproj
    xcodeproj = None
    if build_dir and build_dir.exists():
        for item in build_dir.iterdir():
            if item.suffix == ".xcodeproj":
                xcodeproj = item
                break
    
    if not xcodeproj:
        print(f"[WARNING] macOS build files not found")
        print("Generating build files...")
        result = subprocess.run(
            [sys.executable, str(SCRIPT_DIR / "configure.py")],
            cwd=PROJECT_ROOT
        )
        if result.returncode != 0:
            print("[ERROR] Failed to generate build files")
            return 1
        
        # Re-check for xcodeproj
        for item in build_dir.iterdir():
            if item.suffix == ".xcodeproj":
                xcodeproj = item
                break
        
        if not xcodeproj:
            print("[ERROR] No .xcodeproj found after generation")
            return 1

    cmd = [
        "xcodebuild",
        "-project", str(xcodeproj),
        "-configuration", config,
        "-jobs", str(os.cpu_count() or 4),
        "build",
    ]
    return run_command(cmd, cwd=build_dir)


def build_windows(config, jobs, use_cmake=False, build_dir=None):
    """Build on Windows using MSBuild or CMake."""
    if use_cmake:
        return build_windows_cmake(config, jobs, build_dir)
    else:
        return build_windows_projucer(config, jobs, build_dir)


def build_windows_cmake(config, jobs, build_dir):
    """Build on Windows using CMake."""
    if not build_dir:
        build_dir = PROJECT_ROOT / "build"
    else:
        build_dir = Path(build_dir)
    build_dir.mkdir(exist_ok=True)
    
    # Configure with CMake (use project root as source)
    configure_result = run_command(
        ["cmake", "-S", str(PROJECT_ROOT), "-B", str(build_dir), "-DCMAKE_BUILD_TYPE=" + config],
        cwd=PROJECT_ROOT
    )
    if configure_result != 0:
        print(f"[ERROR] CMake configuration failed")
        return configure_result
    
    # Build with CMake
    build_result = run_command(
        ["cmake", "--build", str(build_dir), "--parallel", str(os.cpu_count() or 4)],
        cwd=PROJECT_ROOT
    )
    
    if build_result != 0:
        print(f"[ERROR] CMake build failed")
        return build_result
    
    print("[OK] CMake build successful")
    return 0


def find_msbuild():
    """Find MSBuild executable on Windows."""
    msbuild_paths = [
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
        r"C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
        r"C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
        r"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe",
        r"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe",
        "msbuild",  # Try PATH
    ]

    for path in msbuild_paths:
        if Path(path).exists() or shutil.which(path):
            return path

    return None


def build_windows_projucer(config, jobs, build_dir):
    """Build on Windows using MSBuild."""
    if build_dir:
        projucer_build_dir = Path(build_dir) if isinstance(build_dir, str) else build_dir
    else:
        projucer_build_dir = get_build_dir("windows")

    # Find the .sln file
    sln_file = None
    if projucer_build_dir and projucer_build_dir.exists():
        for item in projucer_build_dir.iterdir():
            if item.suffix == ".sln":
                sln_file = item
                break

    if not sln_file:
        print(f"[WARNING] Windows build files not found")
        print("Generating build files...")
        result = subprocess.run(
            [sys.executable, str(SCRIPT_DIR / "configure.py")],
            cwd=PROJECT_ROOT
        )
        if result.returncode != 0:
            print("[ERROR] Failed to generate build files")
            return 1

        # Re-check for solution file
        if projucer_build_dir and projucer_build_dir.exists():
            for item in projucer_build_dir.iterdir():
                if item.suffix == ".sln":
                    sln_file = item
                    break

        if not sln_file:
            print("[ERROR] No .sln file found after generation")
            return 1

    # Find MSBuild
    msbuild = find_msbuild()
    if not msbuild:
        print("[ERROR] MSBuild not found. Please run from a Visual Studio Developer Command Prompt,")
        print("        or install Visual Studio with C++ development tools.")
        return 1

    cmd = [
        msbuild, str(sln_file),
        f"/p:Configuration={config}",
        f"/m:{os.cpu_count() or 4}",
        "/nologo",
        "/verbosity:minimal"
    ]
    return run_command(cmd, cwd=projucer_build_dir)


def build(plat, config, jobs, build_tests=False, use_cmake=False, build_dir=None):
    """Run the build for the specified platform."""
    build_method = "CMake" if use_cmake else "Projucer"
    print(f"Building Samplore for {plat} ({config}) using {build_method}")
    print("=" * 60)
    
    if plat == "linux":
        result = build_linux(config, jobs, use_cmake, build_dir)
        if build_tests:
            build_linux_tests(build_dir)
        return result
    elif plat == "macos":
        result = build_macos(config)
        if build_tests:
            build_macos_tests()
        return result
    elif plat == "windows":
        result = build_windows(config, jobs, use_cmake, build_dir)
        if build_tests:
            build_windows_tests()
        return result
    else:
        print(f"Unsupported platform: {plat}")
        return 1


def build_linux_tests(build_dir=None):
    """Build tests on Linux using CMake."""
    print("Building tests...")
    
    test_dir = PROJECT_ROOT / "Source" / "Tests"
    test_build_dir = build_dir or test_dir / "build"
    if isinstance(test_build_dir, str):
        test_build_dir = Path(test_build_dir)
    test_build_dir.mkdir(exist_ok=True)
    
    # Configure with CMake
    configure_result = run_command(
        ["cmake", "-S", str(test_dir), "-B", str(test_build_dir), "-DCMAKE_BUILD_TYPE=Debug"],
        cwd=test_dir
    )
    if configure_result != 0:
        print(f"[ERROR] Test CMake configuration failed")
        return configure_result
    
    # Build with CMake
    build_result = run_command(
        ["cmake", "--build", str(test_build_dir), "--parallel", str(os.cpu_count() or 4)],
        cwd=test_dir
    )
    
    if build_result != 0:
        print(f"[ERROR] Test build failed")
        return build_result
    
    print("[OK] Tests built successfully")
    return 0


def build_macos_tests():
    """Build tests on macOS using Makefile."""
    print("Building tests...")
    
    test_dir = PROJECT_ROOT / "Source" / "Tests"
    
    # Build with make
    cmd = ["make", f"-j{os.cpu_count() or 4}"]
    result = run_command(cmd, cwd=test_dir)
    
    if result != 0:
        print(f"[ERROR] Test build failed")
        return result
    
    print("[OK] Tests built successfully")
    return 0


def build_windows_tests():
    """Build tests on Windows using Makefile."""
    print("Building tests...")
    
    test_dir = PROJECT_ROOT / "Source" / "Tests"
    
    # Build with make (requires make on Windows, e.g., via MinGW)
    cmd = ["make", f"-j{os.cpu_count() or 4}"]
    result = run_command(cmd, cwd=test_dir)
    
    if result != 0:
        print(f"[ERROR] Test build failed")
        return result
    
    print("[OK] Tests built successfully")
    return 0


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
        description="Samplore Native Build Script",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 scripts/build.py                    # Build Release
  python3 scripts/build.py --debug            # Build Debug (with DBG output)
  python3 scripts/build.py --clean            # Clean build artifacts
  python3 scripts/build.py --clean --build    # Clean then build
  python3 scripts/build.py --run              # Build and run
  python3 scripts/build.py -j8                # Build with 8 parallel jobs
        """
    )

    parser.add_argument(
        "--config", "-c",
        choices=["Debug", "Release"],
        default="Release",
        help="Build configuration (default: Release)"
    )
    parser.add_argument(
        "--debug", "-d",
        action="store_true",
        help="Build in Debug mode (shorthand for --config Debug)"
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
    parser.add_argument(
        "--build-tests",
        action="store_true",
        help="Build and run unit tests"
    )
    
    parser.add_argument(
        "--build-with-cmake",
        action="store_true",
        help="Build using CMake (default: use Projucer)"
    )
    
    parser.add_argument(
        "--build-with-projucer",
        action="store_true",
        help="Build using Projucer (default)"
    )
    
    parser.add_argument(
        "--build-dir",
        type=str,
        default="build",
        help="Build directory (isolates .o files, default: build)"
    )

    args = parser.parse_args()

    # Auto-detect platform if not specified
    plat = args.platform or get_platform()
    print(f"Platform: {plat}")
    print(f"Project: {PROJECT_ROOT}")
    print()

    # --debug flag overrides --config
    if args.debug:
        args.config = "Debug"

    # Default to build if no action specified
    if not args.clean and not args.build and not args.run and not args.build_tests:
        args.build = True

    result = 0

    # Clean
    if args.clean:
        result = clean_build(plat)
        if result != 0:
            print("Clean failed!")
            return result

    # Build tests (always uses Debug config)
    if args.build_tests:
        # Only use build_dir for CMake builds, not Projucer
        effective_build_dir = args.build_dir if args.build_with_cmake else None
        result = build(plat, "Debug", args.jobs, build_tests=True,
                         use_cmake=args.build_with_cmake, build_dir=effective_build_dir)
        if result != 0:
            print("\nTest build failed!")
            return result

    # Build main project
    if args.build or args.run:
        # Only use build_dir for CMake builds, not Projucer
        effective_build_dir = args.build_dir if args.build_with_cmake else None
        result = build(plat, args.config, args.jobs,
                         use_cmake=args.build_with_cmake, build_dir=effective_build_dir)
        if result != 0:
            print("\nBuild failed!")
            return result
        print("\nBuild successful!")

        # Show output location
        if args.build_dir and args.build_dir != "build":
            # Show the isolated build location
            binary = Path(args.build_dir) / "Samplore"
            print(f"Output: {binary}")
        else:
            binary = get_output_binary(plat, args.config)
            if binary and binary.exists():
                print(f"Output: {binary}")

    # Run
    if args.run and result == 0:
        print()
        result = run_app(plat, args.config)
        if result != 0:
            print("\nRun failed!")
            return result

    return result


if __name__ == "__main__":
    sys.exit(main())
