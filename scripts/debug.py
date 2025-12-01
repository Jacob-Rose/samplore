#!/usr/bin/env python3
"""
Samplore Debugger Launcher
Launches Samplore under GDB with an optional TUI interface.
"""

import argparse
import os
import platform
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


def get_binary_path(plat, config):
    """Get the path to the Samplore binary."""
    if plat == "linux":
        return BUILDS_DIR / "LinuxMakefile" / "build" / "Samplore"
    elif plat == "macos":
        return BUILDS_DIR / "MacOSX" / "build" / config / "Samplore.app" / "Contents" / "MacOS" / "Samplore"
    elif plat == "windows":
        build_dir = BUILDS_DIR / "VisualStudio2022"
        if not build_dir.exists():
            build_dir = BUILDS_DIR / "VisualStudio2019"
        return build_dir / f"x64/{config}/Samplore.exe"
    return None


def check_debugger(debugger):
    """Check if a debugger is installed."""
    import shutil
    return shutil.which(debugger) is not None


def get_installed_debuggers():
    """Get a list of installed debuggers."""
    debuggers = {
        'gdb': check_debugger('gdb'),
        'cgdb': check_debugger('cgdb'),
    }
    return {k: v for k, v in debuggers.items() if v}


def install_debugger(debugger):
    """Install a debugger using apt."""
    print()
    print(f"Installing {debugger}...")
    print()
    
    try:
        # Update package list
        print("Updating package list...")
        result = subprocess.run(
            ["sudo", "apt-get", "update"],
            check=False
        )
        
        if result.returncode != 0:
            print("✗ Failed to update package list")
            return False
        
        # Install package
        print(f"Installing {debugger}...")
        result = subprocess.run(
            ["sudo", "apt-get", "install", "-y", debugger],
            check=False
        )
        
        if result.returncode == 0:
            print()
            print(f"✓ {debugger} installed successfully!")
            return True
        else:
            print()
            print(f"✗ Failed to install {debugger}")
            return False
            
    except Exception as e:
        print(f"✗ Error: {e}")
        return False


def show_install_menu():
    """Show installation menu for debuggers."""
    print()
    print("=" * 70)
    print("Install Debuggers")
    print("=" * 70)
    print()
    
    # Check what's installed
    installed = get_installed_debuggers()
    
    print("Current status:")
    print(f"  GDB:  {'✓ Installed' if installed.get('gdb') else '✗ Not installed'}")
    print(f"  CGDB: {'✓ Installed' if installed.get('cgdb') else '✗ Not installed (recommended)'}")
    print()
    
    if installed.get('gdb') and installed.get('cgdb'):
        print("All recommended debuggers are installed!")
        input("\nPress Enter to continue...")
        return True
    
    print("Installation options:")
    options = []
    option_num = 1
    
    if not installed.get('gdb'):
        print(f"  {option_num}) Install GDB (required)")
        options.append(('gdb', option_num))
        option_num += 1
    
    if not installed.get('cgdb'):
        print(f"  {option_num}) Install CGDB (recommended - better TUI)")
        options.append(('cgdb', option_num))
        option_num += 1
    
    print(f"  {option_num}) Install all missing")
    options.append(('all', option_num))
    option_num += 1
    
    print(f"  {option_num}) Back to main menu")
    print()
    
    choice = input(f"Select option [1-{option_num}]: ").strip()
    
    try:
        choice_num = int(choice)
        
        # Back to main menu
        if choice_num == option_num:
            return True
        
        # Find what to install
        for pkg, num in options:
            if num == choice_num:
                if pkg == 'all':
                    # Install all missing
                    success = True
                    if not installed.get('gdb'):
                        success = install_debugger('gdb') and success
                    if not installed.get('cgdb'):
                        success = install_debugger('cgdb') and success
                    
                    if success:
                        print()
                        print("✓ All debuggers installed!")
                    else:
                        print()
                        print("⚠ Some installations failed")
                    
                    input("\nPress Enter to continue...")
                    return success
                else:
                    # Install specific package
                    success = install_debugger(pkg)
                    input("\nPress Enter to continue...")
                    return success
        
        print("✗ Invalid option")
        input("\nPress Enter to continue...")
        return False
        
    except ValueError:
        print("✗ Invalid input")
        input("\nPress Enter to continue...")
        return False


def show_debug_menu(config):
    """Show debug options menu."""
    print()
    print("=" * 70)
    print("Debug Samplore")
    print("=" * 70)
    print()
    
    # Check what's installed
    installed = get_installed_debuggers()
    
    if not installed:
        print("✗ No debuggers installed!")
        print()
        print("Please install debuggers first (Option 1 in main menu)")
        input("\nPress Enter to continue...")
        return None
    
    print(f"Configuration: {config}")
    print()
    print("Available debuggers:")
    
    options = []
    option_num = 1
    
    if installed.get('cgdb'):
        print(f"  {option_num}) CGDB (recommended - full TUI with syntax highlighting)")
        options.append(('cgdb', option_num))
        option_num += 1
    
    if installed.get('gdb'):
        print(f"  {option_num}) GDB TUI mode")
        options.append(('gdb-tui', option_num))
        option_num += 1
        print(f"  {option_num}) GDB (command line only)")
        options.append(('gdb', option_num))
        option_num += 1
    
    print()
    print("Special modes:")
    print(f"  {option_num}) Attach to running process")
    options.append(('attach', option_num))
    option_num += 1
    
    print(f"  {option_num}) Back to main menu")
    print()
    
    choice = input(f"Select option [1-{option_num}]: ").strip()
    
    try:
        choice_num = int(choice)
        
        # Back to main menu
        if choice_num == option_num:
            return None
        
        # Find selected option
        for mode, num in options:
            if num == choice_num:
                # Ask for breakpoint
                print()
                set_breakpoint = input("Set a breakpoint? (leave empty to skip): ").strip()
                
                return {
                    'mode': mode,
                    'breakpoint': set_breakpoint if set_breakpoint else None
                }
        
        print("✗ Invalid option")
        input("\nPress Enter to continue...")
        return None
        
    except ValueError:
        print("✗ Invalid input")
        input("\nPress Enter to continue...")
        return None


def show_main_menu():
    """Show main interactive menu."""
    while True:
        print("\033[2J\033[H")  # Clear screen
        print("=" * 70)
        print("Samplore Debugger")
        print("=" * 70)
        print()
        print("Platform: Linux")
        print()
        
        # Check installation status
        installed = get_installed_debuggers()
        print("Debugger status:")
        print(f"  GDB:  {'✓ Installed' if installed.get('gdb') else '✗ Not installed'}")
        print(f"  CGDB: {'✓ Installed' if installed.get('cgdb') else '✗ Not installed'}")
        print()
        
        print("Options:")
        print("  1) Install/Check Debuggers")
        print("  2) Debug Samplore (Debug build)")
        print("  3) Debug Samplore (Release build)")
        print("  4) Exit")
        print()
        
        choice = input("Select option [1-4]: ").strip()
        
        if choice == '1':
            show_install_menu()
        elif choice == '2':
            result = show_debug_menu('Debug')
            if result:
                return ('Debug', result)
        elif choice == '3':
            result = show_debug_menu('Release')
            if result:
                return ('Release', result)
        elif choice == '4':
            print()
            print("Exiting...")
            return None
        else:
            print()
            print("✗ Invalid option")
            input("\nPress Enter to continue...")


def run_debugger_from_menu(config, debug_opts):
    """Run debugger based on menu selection."""
    plat = get_platform()
    mode = debug_opts['mode']
    breakpoint = debug_opts['breakpoint']
    
    # Handle attach mode
    if mode == 'attach':
        return attach_to_process(plat, config)
    
    # Get binary
    binary = get_binary_path(plat, config)
    
    if not binary or not binary.exists():
        print()
        print(f"✗ Binary not found: {binary}")
        print()
        print(f"Build the project first with:")
        print(f"  python3 scripts/build.py --config {config}")
        print()
        input("Press Enter to continue...")
        return 1
    
    print()
    print(f"Binary: {binary}")
    if breakpoint:
        print(f"Breakpoint: {breakpoint}")
    print()
    input("Press Enter to start debugging...")
    
    # Launch appropriate debugger
    if mode == 'cgdb':
        return launch_cgdb(binary, [], breakpoint)
    elif mode == 'gdb-tui':
        return launch_gdb(binary, [], tui=True, breakpoint=breakpoint)
    elif mode == 'gdb':
        return launch_gdb(binary, [], tui=False, breakpoint=breakpoint)
    
    return 1


def launch_gdb(binary, args, tui=False, breakpoint=None):
    """Launch with GDB."""
    if not check_debugger("gdb"):
        print("✗ GDB not installed")
        print()
        print("Install GDB:")
        print("  Ubuntu/Debian: sudo apt-get install gdb")
        print("  Fedora/RHEL:   sudo dnf install gdb")
        print("  Arch:          sudo pacman -S gdb")
        return 1
    
    gdb_args = ["gdb"]
    
    # TUI mode for a nicer interface
    if tui:
        gdb_args.append("-tui")
    
    # Set breakpoint if specified
    if breakpoint:
        gdb_args.extend(["-ex", f"break {breakpoint}"])
    
    # Auto-run the program
    gdb_args.extend(["-ex", "run"])
    
    # Add the binary
    gdb_args.append("--args")
    gdb_args.append(str(binary))
    
    # Add program arguments
    if args:
        gdb_args.extend(args)
    
    print(f"Launching GDB...")
    print(f"Command: {' '.join(gdb_args)}")
    print()
    print("GDB TUI Quick Reference:")
    print("  Ctrl+X A    - Toggle TUI mode")
    print("  Ctrl+X 1    - Source window only")
    print("  Ctrl+X 2    - Source + assembly")
    print("  Ctrl+L      - Refresh screen")
    print("  run         - Start/restart program")
    print("  continue    - Continue execution")
    print("  next        - Step over")
    print("  step        - Step into")
    print("  finish      - Step out")
    print("  break FILE:LINE - Set breakpoint")
    print("  print VAR   - Print variable")
    print("  backtrace   - Show call stack")
    print("  quit        - Exit GDB")
    print()
    print("-" * 60)
    
    return subprocess.run(gdb_args).returncode


def launch_lldb(binary, args, tui=False, breakpoint=None):
    """Launch with LLDB (macOS)."""
    if not check_debugger("lldb"):
        print("✗ LLDB not installed")
        print("Install Xcode Command Line Tools:")
        print("  xcode-select --install")
        return 1
    
    lldb_args = ["lldb"]
    
    # Add the binary
    lldb_args.append(str(binary))
    
    # Build initial commands
    commands = []
    if breakpoint:
        commands.append(f"breakpoint set --name {breakpoint}")
    commands.append("run " + " ".join(args) if args else "run")
    
    # Create command file if we have commands
    cmd_file = PROJECT_ROOT / ".lldb_init"
    if commands:
        with open(cmd_file, 'w') as f:
            for cmd in commands:
                f.write(cmd + "\n")
        lldb_args.extend(["-s", str(cmd_file)])
    
    print(f"Launching LLDB...")
    print(f"Command: {' '.join(lldb_args)}")
    print()
    print("LLDB Quick Reference:")
    print("  run (r)               - Start/restart program")
    print("  continue (c)          - Continue execution")
    print("  next (n)              - Step over")
    print("  step (s)              - Step into")
    print("  finish                - Step out")
    print("  breakpoint set (b)    - Set breakpoint")
    print("  frame variable (v)    - Print variables")
    print("  bt                    - Show call stack")
    print("  quit (q)              - Exit LLDB")
    print()
    print("-" * 60)
    
    result = subprocess.run(lldb_args).returncode
    
    # Cleanup
    if cmd_file.exists():
        cmd_file.unlink()
    
    return result


def launch_cgdb(binary, args, breakpoint=None):
    """Launch with CGDB (nicer TUI than gdb -tui)."""
    if not check_debugger("cgdb"):
        print("✗ CGDB not installed")
        print()
        print("CGDB is a curses-based interface to GDB with syntax highlighting.")
        print()
        print("Install CGDB:")
        print("  Ubuntu/Debian: sudo apt-get install cgdb")
        print("  Fedora/RHEL:   sudo dnf install cgdb")
        print("  Arch:          sudo pacman -S cgdb")
        print("  macOS:         brew install cgdb")
        print()
        print("Falling back to gdb -tui...")
        return launch_gdb(binary, args, tui=True, breakpoint=breakpoint)
    
    cgdb_args = ["cgdb"]
    
    # Set breakpoint if specified
    if breakpoint:
        cgdb_args.extend(["-ex", f"break {breakpoint}"])
    
    # Auto-run
    cgdb_args.extend(["-ex", "run"])
    
    # Add binary
    cgdb_args.append("--args")
    cgdb_args.append(str(binary))
    
    # Add program arguments
    if args:
        cgdb_args.extend(args)
    
    print(f"Launching CGDB...")
    print(f"Command: {' '.join(cgdb_args)}")
    print()
    print("CGDB Quick Reference:")
    print("  ESC         - Switch to source window")
    print("  i           - Switch to GDB window")
    print("  Space       - Set/unset breakpoint")
    print("  o           - Open file dialog")
    print("  /           - Search in source")
    print("  run         - Start/restart program")
    print("  continue    - Continue execution")
    print("  next        - Step over")
    print("  step        - Step into")
    print("  quit        - Exit")
    print()
    print("-" * 60)
    
    return subprocess.run(cgdb_args).returncode


def attach_to_process(plat, config):
    """Attach debugger to a running Samplore process."""
    binary = get_binary_path(plat, config)
    
    if not binary or not binary.exists():
        print(f"✗ Binary not found: {binary}")
        print("Build the project first with:")
        print(f"  python3 scripts/build.py --config {config}")
        return 1
    
    # Find running Samplore processes
    try:
        if plat == "windows":
            result = subprocess.run(
                ["tasklist", "/FI", "IMAGENAME eq Samplore.exe"],
                capture_output=True,
                text=True
            )
        else:
            result = subprocess.run(
                ["pgrep", "-l", "Samplore"],
                capture_output=True,
                text=True
            )
        
        if result.returncode != 0 or not result.stdout.strip():
            print("✗ No running Samplore process found")
            print()
            print("Start Samplore first with:")
            print("  python3 scripts/run.py")
            return 1
        
        # Parse PIDs
        pids = []
        for line in result.stdout.strip().split('\n'):
            if plat == "windows":
                parts = line.split()
                if len(parts) >= 2 and parts[0] == "Samplore.exe":
                    pids.append(parts[1])
            else:
                parts = line.split()
                if parts:
                    pids.append(parts[0])
        
        if not pids:
            print("✗ Could not parse process IDs")
            return 1
        
        # Select PID if multiple
        if len(pids) > 1:
            print("Multiple Samplore processes found:")
            for i, pid in enumerate(pids, 1):
                print(f"  {i}) PID {pid}")
            choice = input("Select process [1]: ").strip() or "1"
            try:
                pid = pids[int(choice) - 1]
            except (ValueError, IndexError):
                print("✗ Invalid selection")
                return 1
        else:
            pid = pids[0]
        
        print(f"Attaching to PID {pid}...")
        print()
        
        # Attach with debugger
        if plat == "macos":
            return subprocess.run(["lldb", "-p", pid]).returncode
        else:
            return subprocess.run(["gdb", "-p", pid]).returncode
            
    except Exception as e:
        print(f"✗ Error: {e}")
        return 1


def main():
    # Check if we're on a supported platform
    plat = get_platform()
    
    # For non-Linux, show a warning
    if plat != "linux":
        print("=" * 70)
        print("Samplore Debugger")
        print("=" * 70)
        print()
        print(f"⚠ Platform: {plat}")
        print()
        print("This interactive debugger menu is currently Linux-only.")
        print()
        if plat == "macos":
            print("For macOS debugging:")
            print("  - Use Xcode debugger")
            print("  - Or run: lldb Builds/MacOSX/build/Debug/Samplore.app/Contents/MacOS/Samplore")
        elif plat == "windows":
            print("For Windows debugging:")
            print("  - Use Visual Studio debugger (recommended)")
            print("  - Or VSCode with C++ extension")
        print()
        return 1
    
    parser = argparse.ArgumentParser(
        description="Samplore Debugger Launcher (Linux)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 scripts/debug.py                    # Interactive menu (Linux only)
  python3 scripts/debug.py --cgdb             # Launch with CGDB (best TUI)
  python3 scripts/debug.py --tui              # Launch with GDB TUI mode
  python3 scripts/debug.py --attach           # Attach to running process
  python3 scripts/debug.py -b main            # Set breakpoint at main()
  python3 scripts/debug.py -b Sample::load    # Set breakpoint at Sample::load

Interactive Menu:
  Run with no arguments to get an interactive menu for installing
  and launching debuggers with various options.

TUI Debuggers:
  CGDB - Best TUI debugger with syntax highlighting and vim-like controls
  GDB TUI - Built-in GDB text UI (simpler than CGDB)
        """
    )
    
    parser.add_argument(
        "--config", "-c",
        choices=["Debug", "Release"],
        default="Debug",
        help="Build configuration to debug (default: Debug)"
    )
    parser.add_argument(
        "--cgdb",
        action="store_true",
        help="Use CGDB (curses-based GDB with syntax highlighting)"
    )
    parser.add_argument(
        "--tui",
        action="store_true",
        help="Use GDB TUI mode (text user interface)"
    )
    parser.add_argument(
        "--attach",
        action="store_true",
        help="Attach to a running Samplore process"
    )
    parser.add_argument(
        "-b", "--breakpoint",
        help="Set a breakpoint (e.g., 'main', 'Sample::load', 'SampleLibrary.cpp:123')"
    )
    parser.add_argument(
        "args",
        nargs="*",
        help="Arguments to pass to Samplore"
    )
    
    args = parser.parse_args()
    
    # If no debugger flags and no args, show interactive menu
    if not args.cgdb and not args.tui and not args.attach and not args.breakpoint and not args.args:
        menu_result = show_main_menu()
        if menu_result is None:
            return 0
        
        config, debug_opts = menu_result
        return run_debugger_from_menu(config, debug_opts)
    
    # Command-line mode
    print("=" * 70)
    print("Samplore Debugger")
    print("=" * 70)
    print()
    
    print(f"Platform: {plat}")
    print(f"Config:   {args.config}")
    print()
    
    # Attach mode
    if args.attach:
        return attach_to_process(plat, args.config)
    
    # Get binary path
    binary = get_binary_path(plat, args.config)
    
    if not binary or not binary.exists():
        print(f"✗ Binary not found: {binary}")
        print()
        print(f"Build the project first with:")
        print(f"  python3 scripts/build.py --config {args.config}")
        return 1
    
    print(f"Binary: {binary}")
    if args.breakpoint:
        print(f"Breakpoint: {args.breakpoint}")
    print()
    
    # Launch appropriate debugger (Linux only at this point)
    if args.cgdb:
        return launch_cgdb(binary, args.args, args.breakpoint)
    else:
        return launch_gdb(binary, args.args, args.tui or True, args.breakpoint)
    
    return 1


if __name__ == "__main__":
    sys.exit(main())
