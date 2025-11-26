# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

SamplifyPlus is a sample management and browsing application for music producers built with JUCE. It provides:
- Directory browsing with hierarchical folder views
- Waveform visualization for audio files
- Custom tagging system for organizing samples
- Audio playback and preview
- Drag-and-drop support into DAWs
- Customizable color themes

**Current Status**: The codebase is being modernized from JUCE 5.x/6.x to JUCE 7+/8 with new Linux support added alongside existing Windows and macOS builds.

## Build System

### Dependencies

The project requires JUCE framework. The JUCE path is configured via `.env` file.

**Linux Build Dependencies**:
```bash
sudo apt-get install libfreetype6-dev libwebkit2gtk-4.1-dev libgtk-3-dev libasound2-dev libcurl4-openssl-dev
```

### Environment Setup

1. Copy `.env.example` to `.env`:
   ```bash
   cp .env.example .env
   ```

2. Edit `.env` and set your JUCE path:
   ```
   JUCE_PATH=/path/to/your/juce
   BUILD_CONFIG=Release
   BUILD_JOBS=4
   ```

### Building

**Using Build Script (Recommended)**:
```bash
# Build for current platform
./scripts/build.sh

# Clean build artifacts
./scripts/clean.sh
```

**Manual Build Commands**:

**Linux (Makefile)**:
```bash
# Debug build
make -C Builds/LinuxMakefile CONFIG=Debug -j4

# Release build
make -C Builds/LinuxMakefile CONFIG=Release -j4

# Output location
./Builds/LinuxMakefile/build/SamplifyPlus
```

**Windows (Visual Studio)**:
Open `Builds/VisualStudio2022/SamplifyPlus.sln` or `Builds/VisualStudio2019/SamplifyPlus.sln`

**macOS (Xcode)**:
Open `Builds/MacOSX/SamplifyPlus.xcodeproj` or use:
```bash
xcodebuild -project Builds/MacOSX/SamplifyPlus.xcodeproj -configuration Release
```

### Projucer

The project uses a `.jucer` file (`SamplifyPlus.jucer`) for build configuration. After modifying the `.jucer` file in Projucer:
1. Save the project in Projucer to regenerate platform-specific build files
2. JUCE module path is set to `/home/jakee/Documents/juce/modules` (Linux)
3. C++17 standard is required

## Architecture

### Core Singleton System

The application uses several singleton patterns for global state:

**SamplifyProperties** (`SamplifyProperties.h/cpp`)
- Singleton wrapper around JUCE's ApplicationProperties
- Manages application settings and persistence
- Owns the `SampleLibrary` and `AudioPlayer` instances
- Access via `SamplifyProperties::getInstance()`
- Lifecycle: `initInstance()` in Main.cpp, `cleanupInstance()` on shutdown

**AppValues** (referenced in Main.cpp)
- Global application values singleton
- Similar lifecycle pattern to SamplifyProperties

### Main Application Structure

**Entry Point**: `Source/Main.cpp`
- `SamplifyPlusApplication` class inherits from `JUCEApplication`
- Creates `MainWindow` → `SamplifyMainComponent`
- Initializes singletons: `AppValues`, `SamplifyProperties`, `AudioPlayer`

**Main Window**: `SamplifyMainComponent` (`SamplifyMainComponent.h/cpp`)
- Singleton accessible via `SamplifyMainComponent::getInstance()`
- Inherits from `AudioAppComponent` for audio I/O
- Layout: Three main explorer panels with resizable edges
  - `DirectoryExplorer` (left) - folder tree view
  - `SampleExplorer` (center) - grid of sample tiles with waveforms
  - `FilterExplorer` (right) - tag management interface
  - `SamplePlayerComponent` (bottom) - audio playback controls

### Sample Management System

**SampleLibrary** (`SampleLibrary.h/cpp`)
- Core data structure for all samples and tags
- Owns collections of `SampleDirectory` objects (user-added root folders)
- Manages tag library (name + color)
- Handles async sample loading with `std::future` for performance
- Query system: text search + tag filtering
- Broadcasts changes when sample lists update

**Sample** (`Sample.h/cpp`)
- Represents a single audio file
- Uses smart pointer architecture: `Sample::Reference` (weak_ptr wrapper) and `Sample::List`
- Properties: tags (StringArray), color, info text, thumbnail
- Change broadcaster for UI updates
- Thumbnail cached via `SampleAudioThumbnail`

**SampleDirectory** (`SampleDirectory.h/cpp`)
- Recursive directory structure
- Each directory has `CheckStatus` (Enabled/Disabled/Mixed) for filtering
- Lazy loading: scans child directories and audio files
- Static `mWildcard` field set by AudioPlayer's format manager (all supported audio formats)

**SampleTag** (`SampleTag.h`)
- Tag metadata structure

### Audio System

**AudioPlayer** (`AudioPlayer.h/cpp`)
- Singleton-like pattern (owned by SamplifyProperties)
- Wraps JUCE's `AudioFormatManager` and `AudioTransportSource`
- States: Playing, Stopped, Stopping, Starting
- Loads and plays `Sample::Reference` objects
- Supports cue points for playback from specific positions

**SampleAudioThumbnail** (`SampleAudioThumbnail.h/cpp`)
- Generates and caches waveform data for visual display
- Uses JUCE's `AudioThumbnailCache`

### UI Components

**Explorer Components**:
- `DirectoryExplorer`: TreeView of `DirectoryExplorerTreeViewItem` nodes
- `SampleExplorer`: Viewport containing `SampleContainer` (grid of `SampleTile`)
- `FilterExplorer`: Contains `TagExplorer_V2` for tag management
- `SamplePlayerComponent`: Audio transport controls, waveform scrubbing

**Tile Components**:
- `SampleTile` (`SampleTile.h/cpp`): Individual sample display with waveform, drag-and-drop source
- `TagTile` (`TagTile.h/cpp`): Tag display/editing tile

**Windows**:
- `PreferenceWindow`: Settings dialog (color customization, etc.)
- `InfoWindow`: Sample information/notes editor

### Look and Feel

**SamplifyLookAndFeel** (`SamplifyLookAndFeel.h/cpp`)
- Custom JUCE LookAndFeel implementation
- Loaded via `SamplifyMainComponent::setupLookAndFeel()`

**LookAndFeel_VJake** (`LookAndFeel_VJake.h/cpp`)
- Alternative/base LookAndFeel
- Applied to MainWindow

**Custom Fonts/Icons**:
- `Fonts.h/cpp`: Binary font resources (Abel, Nevis, CentraleSans)
- `Icons.h/cpp`: SVG icon loading (close, info, correct, minus)

## Data Flow

1. **Startup**:
   - `Main.cpp` initializes singletons: `AppValues`, `SamplifyProperties`
   - `SamplifyProperties::init()` creates `SampleLibrary`
   - `SamplifyProperties::loadPropertiesFile()` restores directories and tags
   - `SampleLibrary` spawns async loading of samples from directories

2. **Sample Loading**:
   - User adds directory → `SampleLibrary::addDirectory()`
   - Creates `SampleDirectory` which recursively scans for audio files
   - Filters by `SampleDirectory::mWildcard` (supported audio formats)
   - `SampleLibrary::getAllSamplesInDirectories_Async()` runs query
   - Results broadcast to `SampleExplorer` which updates `SampleContainer`

3. **Tag Filtering**:
   - User selects tags in `FilterExplorer` → updates query string
   - Query passed to `SampleLibrary::updateCurrentSamples(query)`
   - Filters samples by tag intersection
   - Change broadcast triggers UI refresh

4. **Sample Playback**:
   - User clicks `SampleTile` → calls `AudioPlayer::setSample()`
   - `AudioPlayer::loadFile()` uses `AudioFormatManager` to decode
   - Transport controls in `SamplePlayerComponent` control playback

## Important Patterns

### Async Loading
The `SampleLibrary` uses `std::future<Sample::List>` for non-blocking sample loading. Check `mUpdateSampleFuture.valid()` before polling results. Timer-based polling in `SampleLibrary::timerCallback()`.

### Smart Pointer Architecture
- `Sample` objects owned by `SampleDirectory` as `shared_ptr<Sample>`
- UI and other systems use `Sample::Reference` (weak_ptr wrapper) to avoid ownership issues
- Always check `Sample::Reference::isNull()` before dereferencing

### Change Broadcasting
Heavy use of JUCE's `ChangeBroadcaster`/`ChangeListener` for decoupled communication:
- `SampleLibrary` → `SampleExplorer` (sample list updates)
- `SampleDirectory` → `SampleLibrary` (check status changes)
- `Sample` → UI tiles (tag/color changes)

## JUCE 7+ Migration Notes

The codebase is being migrated from JUCE 5.x/6.x to JUCE 7+/8. Key API changes already addressed:

**Modal Operations (Linux)**:
- `FileChooser::browseForDirectory()` → `launchAsync()` with callbacks
- `CallOutBox::runModalLoop()` → `launchAsynchronously()`
- `DialogWindow::showModalDialog()` → `enterModalState()` with callbacks
- `PopupMenu::show()` → `showMenuAsync()` with callbacks
- `NativeMessageBox` → `MessageBoxOptions` with `showAsync()`

**Known Deprecation Warnings** (non-blocking):
- Font constructors (should migrate to FontOptions)
- DirectoryIterator (should migrate to RangedDirectoryIterator)
- setContentComponent (should use setContentNonOwned)
- DragAndDropContainer::startDragging (newer overload available)

**Compatibility Note**: Modal operations work on Windows/macOS but are removed from Linux builds in JUCE 7+. All async patterns are cross-platform compatible.

## File Organization

```
Source/
├── Main.cpp                          # Application entry point
├── Samplify*.{h,cpp}                # Main components and global systems
├── Structures/                       # Core data structures (logical grouping)
│   ├── AudioPlayer.*
│   ├── Sample.*
│   ├── SampleDirectory.*
│   ├── SampleLibrary.*
│   ├── SampleAudioThumbnail.*
│   ├── SampleTag.*
│   ├── SearchFilter.h
│   ├── SortingMethod.h
│   ├── Icons.*
│   ├── Fonts.*
│   └── SamplifyProperties.*
├── Components/                       # UI components (logical grouping)
│   ├── SamplePlayerComponent.*
│   ├── DirectoryExplorer/
│   │   ├── DirectoryExplorer.*
│   │   └── DirectoryExplorerTreeViewItem.*
│   ├── SampleExplorer/
│   │   ├── SampleExplorer.*
│   │   ├── SampleContainer.*
│   │   └── SampleTile.*
│   └── FilterExplorer/
│       ├── FilterExplorer.*
│       └── TagExplorer/
│           ├── TagExplorer.*
│           ├── TagExplorer_V2.*
│           ├── TagContainer.*
│           └── TagTile.*
└── Windows/                          # Dialog windows
    ├── PreferenceWindow.*
    └── InfoWindow.*
```

## Common Tasks

### Adding a New JUCE Module

1. Open `SamplifyPlus.jucer` in Projucer
2. Add module in Modules section
3. Configure module path: `/home/jakee/Documents/juce/modules`
4. Disable "Use global path"
5. Save project to regenerate build files

### Modifying Sample Properties

Sample properties (tags, color, info text) are stored in:
- Tags: `Sample::mTags` (StringArray)
- Color: `Sample::mColor` (Colour)
- Info: `Sample::mInfoText` (String)

Access through `Sample::Reference` wrapper methods. Changes broadcast to listeners automatically.

### Adding New Color Themes

Colors managed in `SamplifyLookAndFeel` and `PreferenceWindow`. Use JUCE's `Colour` and `LookAndFeel::setColour()` system with custom `ColourIds`.
