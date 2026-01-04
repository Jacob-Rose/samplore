# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Samplore is a cross-platform sample library manager for music producers, built with JUCE (C++17). It provides fast sample preview, tagging, waveform visualization, and drag-and-drop integration with DAWs.

## Build Commands

**IMPORTANT**: Always use the Python scripts in `scripts/` for build operations. Do not use raw `make` commands directly.

```bash
# First-time setup
python3 scripts/setup.py              # Interactive wizard (installs JUCE, configures paths)

# Configure (after .jucer changes or fresh clone)
python3 scripts/configure.py          # Generates build files from .jucer

# Build
python3 scripts/build.py              # Release build (default)
python3 scripts/build.py --debug      # Debug build with symbols
python3 scripts/build.py --clean      # Clean build

# Run
python3 scripts/run.py                # Run release build
python3 scripts/run.py --debug        # Run debug build

# Debug (Linux)
python3 scripts/debug.py              # GDB debugger
python3 scripts/debug.py --cgdb       # Best TUI debugger
python3 scripts/debug.py -b main      # Set breakpoint

# No formal test suite - manual testing required
```

## Code Style

**Language**: C++17 with JUCE framework
**Namespace**: All code in `namespace samplore { }`
**Includes**: `#include "JuceHeader.h"` first, then project headers, then stdlib

**Naming**:
- Classes: `PascalCase` (`SampleLibrary`, `AudioPlayer`)
- Members: `mCamelCase` with `m` prefix (`mCurrentSample`, `mTags`)
- Functions: `camelCase` (`getCurrentSamples()`)
- Constants: `UPPER_SNAKE_CASE` (`SAMPLE_TILE_MIN_WIDTH`)

**Smart Pointers**:
- `std::shared_ptr<T>` for ownership (Sample objects in SampleDirectory)
- `std::weak_ptr<T>` via `Sample::Reference` wrapper for UI references
- `std::unique_ptr<T>` for singletons
- Always check `weak_ptr.expired()` or `Reference.isNull()` before use

**Singletons**: Use `initInstance()`, `cleanupInstance()`, `getInstance()` pattern (see ThemeManager, SamploreProperties, AppValues)

**JUCE Patterns**:
- `ChangeBroadcaster`/`ChangeListener` for decoupled events
- Async operations with `std::future<T>` for non-blocking I/O
- Use `launchAsync()`, `showMenuAsync()` - never `runModalLoop()` (breaks Linux)
- Colors via `ThemeManager` - avoid hardcoded hex values

**Comments**: `///` for Doxygen summaries, `//` for inline. Comment intent, not implementation.

## Architecture

**Entry Point**: `Main.cpp` → `SamploreApplication` → `SamploreMainComponent`

**Core Singletons**:
- `SamploreProperties` - owns `SampleLibrary` and `AudioPlayer`
- `ThemeManager` - colors and theme state
- `AppValues` - application-wide settings

**Data Flow**:
```
SampleLibrary (owns directories/samples/tags)
    ↓ broadcasts changes
UI Components (SampleExplorer, FilterExplorer, etc.)
    ↓ reference samples via
Sample::Reference (weak_ptr wrapper)
```

**Key Components**:
- `SampleDirectory` - owns `Sample` objects as `shared_ptr`
- `SampleExplorer` - grid view of samples with tiles
- `SampleTile` - individual sample display with waveform
- `AudioPlayer` - playback engine
- `OverlayPanel` - modal overlay system with `IOverlayPanelContent` interface

**Import System** (new):
- `SpliceOrganizer` - reads Splice SQLite database
- `SpliceImportTask` - background import with progress/rollback
- `ImportWizard` - extensible import menu

## Platform Notes

- Target: JUCE 7+/8, Linux/macOS/Windows
- Avoid deprecated APIs: modal loops, old Font constructors, DirectoryIterator (use RangedDirectoryIterator)
- SQLite bundled in `Source/ThirdParty/` for Splice integration
