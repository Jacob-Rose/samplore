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

## Building from Source

### Prerequisites

- JUCE framework (version 7+)
- C++17 compatible compiler
- Platform-specific dependencies:

**Linux:**
```bash
sudo apt-get install libfreetype6-dev libwebkit2gtk-4.1-dev libgtk-3-dev libasound2-dev libcurl4-openssl-dev
```

### Quick Start

**Automated Setup (Recommended):**
```bash
git clone https://github.com/Jacob-Rose/samplore.git
cd samplore
python3 scripts/setup.py    # Interactive setup wizard
python3 scripts/build.py    # Build the project
python3 scripts/run.py      # Run the application
```

**Using Shell Scripts (Alternative):**
```bash
./scripts/setup.sh    # Setup
./scripts/build.sh    # Build
./scripts/run.sh      # Run
```

**Manual Setup:**
```bash
# 1. Clone and configure
git clone https://github.com/Jacob-Rose/samplore.git
cd samplore
cp .env.example .env
# Edit .env and set JUCE_PATH=/path/to/your/juce

# 2. Generate build files
python3 scripts/configure.py

# 3. Build and run
python3 scripts/build.py
python3 scripts/run.py
```

### Build Toolchain

The project includes a comprehensive cross-platform build toolchain with **Python scripts** (recommended) and shell script alternatives:

| Script | Python | Shell | Description |
|--------|--------|-------|-------------|
| **Setup** | `setup.py` | `setup.sh` | Interactive first-time configuration wizard |
| **Configure** | `configure.py` | - | Updates `.jucer` file, generates build files |
| **Build** | `build.py` | `build.sh` | Compiles the project |
| **Clean** | `clean.py` | `clean.sh` | Removes build artifacts |
| **Run** | `run.py` | `run.sh` | Launches the built application |

**Why Python?** Python scripts work identically across all platforms (Linux, macOS, Windows) without requiring Git Bash or MSYS2 on Windows.

**Note**: The `.jucer` file uses placeholder paths that are configured per-machine. See [JUCER_PATHS_README.md](JUCER_PATHS_README.md) for details.

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
