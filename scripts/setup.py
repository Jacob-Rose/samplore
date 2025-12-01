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


def clone_juce(install_path, branch="master"):
    """Clone JUCE from GitHub to the specified path."""
    print()
    print(f"Cloning JUCE from GitHub to {install_path}...")
    print(f"Branch: {branch}")
    print()
    print("This may take a few minutes...")
    print()
    
    # Check if git is installed
    if not shutil.which("git"):
        print("✗ Git is not installed. Please install git first.")
        return False
    
    # Create parent directory if it doesn't exist
    install_path.parent.mkdir(parents=True, exist_ok=True)
    
    # Clone the repository
    try:
        result = subprocess.run(
            [
                "git", "clone",
                "--branch", branch,
                "--depth", "1",  # Shallow clone for faster download
                "https://github.com/juce-framework/JUCE.git",
                str(install_path)
            ],
            check=True
        )
        
        if result.returncode == 0:
            print()
            print("✓ JUCE cloned successfully!")
            return True
        else:
            print()
            print("✗ Failed to clone JUCE")
            return False
            
    except subprocess.CalledProcessError as e:
        print()
        print(f"✗ Git clone failed: {e}")
        return False
    except Exception as e:
        print()
        print(f"✗ Error: {e}")
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
    
    # Find existing JUCE installations
    suggestions = find_juce_installations()
    
    # Ask user how they want to provide JUCE
    print("JUCE Setup Options:")
    print("  1) Use existing JUCE installation")
    print("  2) Clone JUCE from GitHub (recommended if you don't have JUCE)")
    print()
    
    if suggestions:
        print("Found existing JUCE installations:")
        for suggestion in suggestions:
            print(f"  - {suggestion}")
        print()
    
    setup_choice = get_input("Choice", default="1" if suggestions else "2")
    print()
    
    juce_path = None
    
    if setup_choice == "2":
        # Clone JUCE from GitHub
        print("Where would you like to install JUCE?")
        print("Examples:")
        print("  Linux:   /home/username/JUCE")
        print("  macOS:   /Users/username/JUCE")
        print("  Windows: C:/JUCE")
        print()
        
        default_path = str(Path.home() / "JUCE")
        juce_path_str = get_input("Install path", default=default_path)
        temp_clone_path = Path(os.path.expanduser(juce_path_str))
        
        # Check if path already exists
        if temp_clone_path.exists():
            print()
            print(f"⚠ Warning: Directory already exists: {temp_clone_path}")
            if not get_yes_no("Remove and re-clone?", default_yes=False):
                print()
                print("Using existing directory...")
                juce_path = temp_clone_path
            else:
                print()
                print(f"Removing {temp_clone_path}...")
                shutil.rmtree(temp_clone_path)
                juce_path = None
        else:
            juce_path = None
        
        # Ask for branch/version
        print()
        print("Which JUCE version would you like to clone?")
        print("  1) master (latest stable)")
        print("  2) develop (cutting edge)")
        print("  3) Specify a version tag (e.g., 7.0.9, 8.0.0)")
        print()
        
        version_choice = get_input("Choice", default="1")
        
        if version_choice == "2":
            branch = "develop"
        elif version_choice == "3":
            branch = get_input("Version tag (e.g., 7.0.9)")
            if not branch:
                print("✗ Version tag cannot be empty")
                return 1
        else:
            branch = "master"
        
        # Clone JUCE if we don't have it yet
        if juce_path is None:
            # First clone to temporary location
            clone_temp_path = temp_clone_path.parent / "JUCE_temp_clone"
            
            if not clone_juce(clone_temp_path, branch):
                print()
                print("✗ Failed to clone JUCE")
                print()
                print("You can manually download JUCE from https://juce.com/")
                print("Then re-run this setup script.")
                return 1
            
            # Ask if user wants to rename
            print()
            suggested_name = temp_clone_path.name
            print(f"The repository will be named: {suggested_name}")
            
            if get_yes_no("Would you like to use a different folder name?", default_yes=False):
                print()
                print("Enter the desired folder name (just the name, not the full path):")
                print("Examples: JUCE, juce, juce-8.0, my-juce")
                print()
                
                new_name = get_input("Folder name", default=suggested_name)
                if new_name and new_name != suggested_name:
                    juce_path = temp_clone_path.parent / new_name
                else:
                    juce_path = temp_clone_path
            else:
                juce_path = temp_clone_path
            
            # Rename if necessary
            if clone_temp_path != juce_path:
                print()
                print(f"Renaming {clone_temp_path.name} → {juce_path.name}...")
                
                # Check if target already exists
                if juce_path.exists():
                    print(f"✗ Target directory already exists: {juce_path}")
                    print(f"Cleaning up temporary clone...")
                    shutil.rmtree(clone_temp_path)
                    return 1
                
                try:
                    shutil.move(str(clone_temp_path), str(juce_path))
                    print("✓ Renamed successfully")
                except Exception as e:
                    print(f"✗ Failed to rename: {e}")
                    print("Using original name...")
                    juce_path = clone_temp_path
        
        # Verify clone
        if not (juce_path / "modules").exists():
            print()
            print("✗ JUCE clone appears incomplete - modules directory not found")
            return 1
        
        print()
        print(f"✓ JUCE installed at: {juce_path}")
    else:
        # Use existing JUCE installation
        print("Please enter the path to your JUCE installation:")
        print("Examples:")
        print("  Linux:   /home/username/JUCE")
        print("  macOS:   /Users/username/JUCE")
        print("  Windows: C:/JUCE")
        print()
        
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
            print(f"✗ Error: Directory does not exist: {juce_path}")
            print()
            print("Please provide a valid JUCE installation path,")
            print("or choose option 2 to clone JUCE from GitHub.")
            return 1
        
        if not (juce_path / "modules").exists():
            print()
            print("✗ Error: No 'modules' directory found in JUCE path")
            print(f"   Expected: {juce_path / 'modules'}")
            print()
            print("This doesn't appear to be a valid JUCE installation.")
            return 1
    
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
