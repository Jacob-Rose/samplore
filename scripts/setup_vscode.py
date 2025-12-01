#!/usr/bin/env python3
"""
VSCode Configuration Generator for Samplore
Automatically generates c_cpp_properties.json from .env and .jucer file
"""

import argparse
import json
import os
import platform
import sys
import xml.etree.ElementTree as ET
from pathlib import Path


SCRIPT_DIR = Path(__file__).parent.resolve()
PROJECT_ROOT = SCRIPT_DIR.parent
JUCER_FILE = PROJECT_ROOT / "Samplore.jucer"
ENV_FILE = PROJECT_ROOT / ".env"
VSCODE_DIR = PROJECT_ROOT / ".vscode"
CPP_PROPERTIES_FILE = VSCODE_DIR / "c_cpp_properties.json"


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


def get_juce_modules_from_jucer():
    """Extract JUCE modules from the .jucer file."""
    if not JUCER_FILE.exists():
        print(f"Error: .jucer file not found: {JUCER_FILE}")
        return []
    
    try:
        tree = ET.parse(JUCER_FILE)
        root = tree.getroot()
        
        # Find MODULES section
        modules_elem = root.find('.//MODULES')
        if modules_elem is None:
            print("Warning: No MODULES section found in .jucer file")
            return []
        
        modules = []
        for module in modules_elem.findall('MODULE'):
            module_id = module.get('id')
            if module_id:
                modules.append(module_id)
        
        return sorted(modules)
    
    except Exception as e:
        print(f"Error parsing .jucer file: {e}")
        return []


def get_defines_from_jucer():
    """Extract preprocessor defines from the .jucer file."""
    if not JUCER_FILE.exists():
        return []
    
    try:
        tree = ET.parse(JUCER_FILE)
        root = tree.getroot()
        
        defines = []
        
        # Get project type
        project_type = root.get('projectType', '')
        if 'GUI' in project_type or 'Standalone' in project_type:
            defines.append('JUCE_STANDALONE_APPLICATION=1')
        
        # Get common settings
        juce_settings = root.find('.//JUCEOPTIONS')
        if juce_settings is not None:
            for key, value in juce_settings.attrib.items():
                if value == '1':
                    defines.append(f'{key}=1')
                elif value == '0':
                    defines.append(f'{key}=0')
        
        return defines
    
    except Exception as e:
        print(f"Error parsing defines from .jucer: {e}")
        return []


def normalize_path_for_vscode(path):
    """Normalize path for VSCode configuration (forward slashes, absolute)."""
    path_obj = Path(path).expanduser().resolve()
    # Convert to string with forward slashes (works on all platforms in VSCode)
    return str(path_obj).replace('\\', '/')


def generate_linux_config(juce_path, modules):
    """Generate Linux configuration."""
    juce_path_obj = Path(juce_path).expanduser().resolve()
    modules_path = juce_path_obj / "modules"
    
    include_paths = [
        "${workspaceFolder}/**",
        "${workspaceFolder}/Source",
        "${workspaceFolder}/JuceLibraryCode",
        normalize_path_for_vscode(modules_path),
    ]
    
    # Add each JUCE module directory
    for module in modules:
        include_paths.append(normalize_path_for_vscode(modules_path / module))
    
    defines = [
        "LINUX=1",
        "DEBUG=1",
        "JUCE_DISPLAY_SPLASH_SCREEN=0",
        "JUCE_USE_DARK_SPLASH_SCREEN=1",
        "JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1",
        "JUCE_STRICT_REFCOUNTEDPOINTER=1",
    ]
    
    # Add module availability defines
    for module in modules:
        defines.append(f"JUCE_MODULE_AVAILABLE_{module}=1")
    
    # Add custom defines from .jucer
    custom_defines = get_defines_from_jucer()
    defines.extend(custom_defines)
    
    return {
        "name": "Linux",
        "includePath": include_paths,
        "defines": defines,
        "compilerPath": "/usr/bin/g++",
        "cStandard": "c17",
        "cppStandard": "c++17",
        "intelliSenseMode": "linux-gcc-x64",
        "compileCommands": "${workspaceFolder}/Builds/LinuxMakefile/compile_commands.json"
    }


def generate_macos_config(juce_path, modules):
    """Generate macOS configuration."""
    juce_path_obj = Path(juce_path).expanduser().resolve()
    modules_path = juce_path_obj / "modules"
    
    include_paths = [
        "${workspaceFolder}/**",
        "${workspaceFolder}/Source",
        "${workspaceFolder}/JuceLibraryCode",
        normalize_path_for_vscode(modules_path),
    ]
    
    # Add each JUCE module directory
    for module in modules:
        include_paths.append(normalize_path_for_vscode(modules_path / module))
    
    defines = [
        "MACOS=1",
        "DEBUG=1",
        "JUCE_DISPLAY_SPLASH_SCREEN=0",
        "JUCE_USE_DARK_SPLASH_SCREEN=1",
        "JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1",
        "JUCE_STRICT_REFCOUNTEDPOINTER=1",
    ]
    
    # Add module availability defines
    for module in modules:
        defines.append(f"JUCE_MODULE_AVAILABLE_{module}=1")
    
    # Add custom defines from .jucer
    custom_defines = get_defines_from_jucer()
    defines.extend(custom_defines)
    
    return {
        "name": "Mac",
        "includePath": include_paths,
        "defines": defines,
        "macFrameworkPath": [
            "/System/Library/Frameworks",
            "/Library/Frameworks"
        ],
        "compilerPath": "/usr/bin/clang++",
        "cStandard": "c17",
        "cppStandard": "c++17",
        "intelliSenseMode": "macos-clang-x64"
    }


def generate_windows_config(juce_path, modules):
    """Generate Windows configuration."""
    juce_path_obj = Path(juce_path).expanduser().resolve()
    modules_path = juce_path_obj / "modules"
    
    include_paths = [
        "${workspaceFolder}/**",
        "${workspaceFolder}/Source",
        "${workspaceFolder}/JuceLibraryCode",
        normalize_path_for_vscode(modules_path),
    ]
    
    # Add each JUCE module directory
    for module in modules:
        include_paths.append(normalize_path_for_vscode(modules_path / module))
    
    defines = [
        "WINDOWS=1",
        "WIN32",
        "_WINDOWS",
        "DEBUG=1",
        "_DEBUG",
        "JUCE_DISPLAY_SPLASH_SCREEN=0",
        "JUCE_USE_DARK_SPLASH_SCREEN=1",
        "JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1",
        "JUCE_STRICT_REFCOUNTEDPOINTER=1",
    ]
    
    # Add module availability defines
    for module in modules:
        defines.append(f"JUCE_MODULE_AVAILABLE_{module}=1")
    
    # Add custom defines from .jucer
    custom_defines = get_defines_from_jucer()
    defines.extend(custom_defines)
    
    return {
        "name": "Win32",
        "includePath": include_paths,
        "defines": defines,
        "windowsSdkVersion": "10.0.22000.0",
        "compilerPath": "cl.exe",
        "cStandard": "c17",
        "cppStandard": "c++17",
        "intelliSenseMode": "windows-msvc-x64"
    }


def generate_cpp_properties(juce_path, modules):
    """Generate c_cpp_properties.json content."""
    configurations = [
        generate_linux_config(juce_path, modules),
        generate_macos_config(juce_path, modules),
        generate_windows_config(juce_path, modules),
    ]
    
    return {
        "configurations": configurations,
        "version": 4
    }


def main():
    parser = argparse.ArgumentParser(
        description="VSCode Configuration Generator for Samplore",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 scripts/setup_vscode.py           # Generate VSCode config from .env
  python3 scripts/setup_vscode.py --force   # Overwrite existing config
  python3 scripts/setup_vscode.py --check   # Check current config status

This script reads:
  - JUCE path from .env
  - Module list from Samplore.jucer
  - Generates .vscode/c_cpp_properties.json for IntelliSense
        """
    )
    
    parser.add_argument(
        "--force",
        action="store_true",
        help="Overwrite existing c_cpp_properties.json"
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Check configuration status without making changes"
    )
    
    args = parser.parse_args()
    
    print("=" * 70)
    print("VSCode Configuration Generator")
    print("=" * 70)
    print()
    
    # Check for .env file
    if not ENV_FILE.exists():
        print(f"✗ Error: .env file not found: {ENV_FILE}")
        print()
        print("Please run setup first:")
        print("  python3 scripts/setup.py")
        return 1
    
    # Load JUCE path
    env_vars = load_env_file()
    juce_path = env_vars.get('JUCE_PATH')
    
    if not juce_path or juce_path == '/path/to/juce':
        print("✗ Error: JUCE_PATH not configured in .env")
        print()
        print("Please run setup first:")
        print("  python3 scripts/setup.py")
        return 1
    
    # Validate JUCE path
    juce_path_obj = Path(juce_path).expanduser()
    if not juce_path_obj.exists():
        print(f"✗ Warning: JUCE path does not exist: {juce_path_obj}")
        print()
    
    modules_path = juce_path_obj / "modules"
    if not modules_path.exists():
        print(f"✗ Warning: JUCE modules directory not found: {modules_path}")
        print()
    
    print(f"JUCE Path: {juce_path}")
    print()
    
    # Get modules from .jucer
    print("Reading modules from Samplore.jucer...")
    modules = get_juce_modules_from_jucer()
    
    if not modules:
        print("✗ Warning: No JUCE modules found in .jucer file")
        print("Continuing with default module set...")
        # Fallback to common modules
        modules = [
            "juce_audio_basics",
            "juce_audio_devices",
            "juce_audio_formats",
            "juce_audio_processors",
            "juce_audio_utils",
            "juce_core",
            "juce_data_structures",
            "juce_events",
            "juce_graphics",
            "juce_gui_basics",
            "juce_gui_extra",
        ]
    
    print(f"Found {len(modules)} JUCE modules:")
    for module in modules:
        print(f"  - {module}")
    print()
    
    # Check mode
    if args.check:
        if CPP_PROPERTIES_FILE.exists():
            print(f"✓ VSCode config exists: {CPP_PROPERTIES_FILE}")
            print()
            print("To regenerate, run with --force")
        else:
            print(f"✗ VSCode config not found: {CPP_PROPERTIES_FILE}")
            print()
            print("Run without --check to generate")
        return 0
    
    # Check if file exists
    if CPP_PROPERTIES_FILE.exists() and not args.force:
        print(f"⚠ VSCode config already exists: {CPP_PROPERTIES_FILE}")
        print()
        print("Use --force to overwrite, or delete the file manually")
        return 0
    
    # Generate configuration
    print("Generating c_cpp_properties.json...")
    config = generate_cpp_properties(juce_path, modules)
    
    # Create .vscode directory if needed
    VSCODE_DIR.mkdir(exist_ok=True)
    
    # Write configuration
    with open(CPP_PROPERTIES_FILE, 'w') as f:
        json.dump(config, f, indent=4)
    
    print(f"✓ Generated: {CPP_PROPERTIES_FILE}")
    print()
    print("=" * 70)
    print("Configuration Complete!")
    print("=" * 70)
    print()
    print("VSCode IntelliSense should now work correctly.")
    print()
    print("If you still see errors:")
    print("  1. Reload VSCode (Ctrl+Shift+P → 'Reload Window')")
    print("  2. Open a C++ file and check the configuration")
    print("  3. Select the correct configuration (Linux/Mac/Win32)")
    print("     - Click the configuration name in the status bar")
    print("     - Or Ctrl+Shift+P → 'C/C++: Select a Configuration'")
    print()
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
