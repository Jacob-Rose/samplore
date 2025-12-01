# Samplore

![Samplore Screenshot](https://i.imgur.com/TEacSr6.png)

A modern, open-source sample library manager for music producers built with JUCE.

## Overview

Music production has hundreds of great tools, but searching for the right sample when you have libraries of thousands of files is difficult in modern DAWs like Ableton Live and FL Studio.

**Samplore** (formerly Samplify) is a dedicated sample browser that makes discovering and organizing your sound library effortless.

## Features

- **Fast Sample Preview** - Near-zero latency playback, start from any point in the sample
- **Directory Explorer** - Navigate your sample folders with a hierarchical tree view
- **Waveform Visualization** - See your samples with customizable waveform detail
- **Tag System** - Organize samples with custom tags and colors
- **Search & Filter** - Quickly find samples by name, tags, or directory
- **Drag & Drop** - Seamlessly drag samples into your DAW
- **Sample Notes** - Leave notes and ideas on individual samples
- **Customizable Themes** - Dark/Light themes with custom color schemes
- **Cross-Platform** - Runs on Windows, macOS, and Linux

![Samplore Interface](https://i.imgur.com/yw0G0ml.png)

## Quick Setup Guide

### 🚀 One-Command Setup

```bash
git clone https://github.com/Jacob-Rose/samplore.git
cd samplore
python3 scripts/setup.py    # Interactive setup wizard
```

The setup wizard will:
- ✅ **Install JUCE** (if you don't have it)
- ✅ **Configure build paths** automatically  
- ✅ **Install Linux dependencies** (if needed)
- ✅ **Set up VSCode IntelliSense** (optional)
- ✅ **Generate build files** automatically

### 🏗️ Build & Run

```bash
python3 scripts/build.py    # Build the project
python3 scripts/run.py      # Run the application
```

### 🔧 Advanced Options

**Manual Configuration:**
```bash
# 1. Configure manually
cp .env.example .env
# Edit .env and set JUCE_PATH=/path/to/your/juce

# 2. Generate build files  
python3 scripts/configure.py

# 3. Build and run
python3 scripts/build.py
python3 scripts/run.py
```

**Debugging (Linux):**
```bash
# Interactive debugger menu
python3 scripts/debug.py

# Or use specific debugger
python3 scripts/debug.py --cgdb    # Best TUI experience
```

**VSCode IntelliSense:**
```bash
# Generate VSCode config (auto-syncs with .env + .jucer)
python3 scripts/setup_vscode.py
```

### 📋 Build Scripts

| Script | Description |
|--------|-------------|
| **`setup.py`** | Interactive first-time setup wizard |
| **`configure.py`** | Updates `.jucer`, generates build files |
| **`build.py`** | Compiles project for current platform |
| **`debug.py`** | Interactive debugging menu (Linux) |
| **`setup_vscode.py`** | VSCode IntelliSense configuration |
| **`clean.py`** | Removes build artifacts |
| **`run.py`** | Launches the built application |

### 🐧 Linux Dependencies

The setup script automatically installs these on Linux:
```bash
sudo apt-get install libfreetype6-dev libwebkit2gtk-4.1-dev libgtk-3-dev libasound2-dev libcurl4-openssl-dev
```

### 💻 Platform Support

- ✅ **Linux** - Full support with GCC
- ✅ **macOS** - Full support with Clang  
- ✅ **Windows** - Full support with MSVC
- ✅ **VSCode** - Cross-platform IntelliSense

### 🔄 Why Python Scripts?

All build scripts use **Python standard library only** - no external dependencies required! This provides:
- **Cross-platform consistency** (Linux/macOS/Windows)
- **No shell compatibility issues**
- **Rich error handling and user feedback**
- **Interactive menus and progress indicators**

### 📝 Configuration Files

- **`.env`** - Local configuration (JUCE path, build settings)
- **`Samplore.jucer`** - JUCE project configuration
- **`.vscode/c_cpp_properties.json`** - VSCode IntelliSense (auto-generated)

**Note**: `.jucer` uses placeholder paths that are configured per-machine by the setup wizard.

### Building for Windows

**On Windows:**
- Use Visual Studio 2019 or 2022 to open `Builds/VisualStudio2022/Samplore.sln`
- Or use the Python build scripts as shown above

**From Linux/macOS:**
- Cross-compilation is not officially supported by JUCE
- Use a Windows VM or dual-boot setup for Windows builds
- Alternatively, use GitHub Actions or CI/CD for automated Windows builds

See [CLAUDE.md](CLAUDE.md) for detailed build instructions and architecture documentation.

## Technology

Built with the [JUCE framework](https://juce.com/), Samplore provides:
- Professional audio processing
- Cross-platform compatibility
- Modern C++ architecture
- Efficient memory management for large libraries

## Contributing

Contributions are welcome! Please feel free to:
- Report bugs
- Suggest features
- Submit pull requests
- Improve documentation

## History

Originally developed as "Samplify" as a personal project and learning experience with JUCE. Now open source and renamed to "Samplore" to avoid trademark conflicts.

## License

See repository for license details.

## Acknowledgments

- Built with [JUCE](https://juce.com/)
- Icons sourced legally (see CLAUDE.md for details)
- Font resources included under their respective licenses
