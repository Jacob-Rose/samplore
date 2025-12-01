#!/usr/bin/env python3
"""
run_tests.py - Run Samplore unit tests

Builds and runs the test suite for Samplore.
"""

import sys
import subprocess
import argparse
from pathlib import Path

# Project paths
SCRIPT_DIR = Path(__file__).parent
PROJECT_ROOT = SCRIPT_DIR.parent
TEST_BUILD_DIR = PROJECT_ROOT / "Source" / "Tests" / "build"


def get_platform():
    """Detect the current platform."""
    if sys.platform.startswith("linux"):
        return "linux"
    elif sys.platform == "darwin":
        return "macos"
    elif sys.platform == "win32":
        return "windows"
    else:
        return "unknown"


def get_test_binary(plat):
    """Get the path to the test binary for the platform."""
    if plat == "linux" or plat == "macos":
        return TEST_BUILD_DIR / "SamploreTests_artefacts" / "Debug" / "SamploreTests"
    elif plat == "windows":
        return TEST_BUILD_DIR / "SamploreTests_artefacts" / "Debug" / "SamploreTests.exe"
    return None


def build_tests():
    """Build the tests using build.py."""
    print("Building tests...")
    print("=" * 60)
    
    build_script = SCRIPT_DIR / "build.py"
    result = subprocess.run(
        [sys.executable, str(build_script), "--build-tests"],
        cwd=PROJECT_ROOT
    )
    
    if result.returncode != 0:
        print("\n✗ Test build failed!")
        return False
    
    print("\n✓ Test build successful!")
    return True


def run_tests(plat, args):
    """Run the test binary."""
    binary = get_test_binary(plat)
    
    if not binary or not binary.exists():
        print(f"✗ Test binary not found: {binary}")
        print("Build the tests first with: python3 scripts/build.py --build-tests")
        return 1
    
    print("\nRunning tests...")
    print("=" * 60)
    
    # Build command with optional Catch2 arguments
    cmd = [str(binary)]
    if args:
        cmd.extend(args)
    
    result = subprocess.run(cmd, cwd=PROJECT_ROOT)
    
    print("=" * 60)
    
    if result.returncode == 0:
        print("✓ All tests passed!")
    else:
        print("✗ Some tests failed!")
    
    return result.returncode


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="Run Samplore unit tests",
        epilog="""
Examples:
  python3 scripts/run_tests.py              # Build and run all tests
  python3 scripts/run_tests.py --no-build   # Run without building
  python3 scripts/run_tests.py --list       # List all tests
  python3 scripts/run_tests.py "[basic]"    # Run tests matching tag
  python3 scripts/run_tests.py -s           # Show successful tests too

Catch2 Options (pass after --):
  --list-tests                  List all test cases
  --list-tags                   List all tags
  -s, --success                 Show successful tests
  -b, --break                   Break into debugger on failure
  [test-spec]                   Run specific tests (e.g., "[basic]")
        """,
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    
    parser.add_argument(
        "--no-build",
        action="store_true",
        help="Skip building, just run tests"
    )
    
    parser.add_argument(
        "--platform", "-p",
        choices=["linux", "macos", "windows"],
        default=None,
        help="Target platform (default: auto-detect)"
    )
    
    # Capture remaining args for Catch2
    parser.add_argument(
        "catch_args",
        nargs="*",
        help="Arguments to pass to Catch2 test runner"
    )
    
    args = parser.parse_args()
    
    # Detect platform
    plat = args.platform or get_platform()
    print(f"Platform: {plat}")
    print(f"Project: {PROJECT_ROOT}")
    print()
    
    # Build tests if needed
    if not args.no_build:
        if not build_tests():
            return 1
        print()
    
    # Run tests
    return run_tests(plat, args.catch_args)


if __name__ == "__main__":
    sys.exit(main())
