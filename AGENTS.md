# AGENTS.md - Samplore

Quick reference for AI coding agents working in this repository. See CLAUDE.md for detailed project documentation.

## Build Commands

```bash
# Setup (first time only)
python3 scripts/setup.py              # Interactive setup with .env configuration
# OR
./scripts/setup.sh                    # Shell wrapper for setup.py

# Configure (generate build files from .jucer)
python3 scripts/configure.py          # Configures JUCE paths and generates build files
                                      # Will auto-build Projucer if not found

# Build
python3 scripts/build.py              # Build for current platform (auto-detect)
python3 scripts/build.py --clean      # Clean build artifacts
make -C Builds/LinuxMakefile CONFIG=Debug -j4    # Linux debug build
make -C Builds/LinuxMakefile CONFIG=Release -j4  # Linux release build

# Run
python3 scripts/run.py                # Launch the built application
./Builds/LinuxMakefile/build/Samplore # Direct launch (Linux)

# Testing: No formal test suite yet - manual testing required
```

## Code Style

**Language**: C++17 (JUCE framework)  
**Namespace**: All code in `namespace samplore { }`  
**Includes**: Always `#include "JuceHeader.h"` first, then project headers, then standard library  
**Naming**:
- Classes: `PascalCase` (e.g., `SampleLibrary`, `AudioPlayer`)
- Member variables: `mCamelCase` with `m` prefix (e.g., `mCurrentSample`, `mTags`)
- Functions: `camelCase` (e.g., `getCurrentSamples()`, `updateCurrentSamples()`)
- Constants: `UPPER_SNAKE_CASE` (e.g., `SAMPLE_TILE_MIN_WIDTH`)

**Smart Pointers**:
- Use `std::shared_ptr<T>` for ownership (e.g., Sample objects)
- Use `std::weak_ptr<T>` for non-owning references (e.g., `Sample::Reference` wrapper)
- Use `std::unique_ptr<T>` for singletons (e.g., `ThemeManager::instance`)
- Always check `weak_ptr.expired()` or `Reference.isNull()` before dereferencing

**Singletons**: Use static `initInstance()`, `cleanupInstance()`, `getInstance()` pattern (see `ThemeManager`, `SamploreProperties`, `AppValues`)

**JUCE Patterns**:
- Use `ChangeBroadcaster`/`ChangeListener` for decoupled event communication
- Prefer async operations with `std::future<T>` for non-blocking I/O (see `SampleLibrary::mUpdateSampleFuture`)
- Modal dialogs: Use async methods (`launchAsync()`, `showMenuAsync()`) instead of `runModalLoop()` for Linux compatibility
- Colors: Use JUCE's `LookAndFeel_V4::getColourScheme()` for system defaults, avoid hardcoding hex values

**Error Handling**: Informal style with DBG() macros, nullptr checks, and AlertWindow for user-facing errors

**Comments**: Use `///` for Doxygen-style summaries, `//` for inline comments. Comment intent, not implementation.

## Architecture Notes

- **Entry**: `Main.cpp` → `SamploreApplication` → `SamploreMainComponent`
- **Core singletons**: `SamploreProperties` (owns `SampleLibrary`, `AudioPlayer`), `AppValues`, `ThemeManager`
- **Data flow**: `SampleLibrary` manages all samples/tags/directories → broadcasts changes → UI components update
- **Smart pointer arch**: `Sample` owned by `SampleDirectory` as `shared_ptr`, UI uses `Sample::Reference` (weak_ptr wrapper)

## Platform Notes

Target: JUCE 7+/8 with Linux/macOS/Windows support. Avoid deprecated APIs (modal loops, old Font constructors, DirectoryIterator).
