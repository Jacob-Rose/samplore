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

### Setup

1. Clone the repository:
```bash
git clone https://github.com/Jacob-Rose/samplore.git
cd samplore
```

2. Configure environment:
```bash
cp .env.example .env
# Edit .env and set your JUCE_PATH
```

3. Build:
```bash
./scripts/build.sh
```

4. Run:
```bash
./scripts/run.sh
```

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
