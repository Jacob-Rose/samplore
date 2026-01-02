# AGENTS.md

Quick reference for AI coding agents. See [CLAUDE.md](CLAUDE.md) for full documentation.

## Quick Commands

```bash
python3 scripts/setup.py       # First-time setup
python3 scripts/configure.py   # Generate build files
python3 scripts/build.py       # Build
python3 scripts/run.py         # Run
python3 scripts/debug.py       # Debug (Linux)
```

## Style Cheatsheet

```cpp
namespace samplore {

class MyComponent : public Component {  // PascalCase classes
public:
    void doSomething();                 // camelCase functions

private:
    String mName;                       // mCamelCase members
    static const int MAX_SIZE = 100;    // UPPER_SNAKE constants
};

}
```

## Key Patterns

- **Includes**: `JuceHeader.h` first, then project, then stdlib
- **Ownership**: `shared_ptr` owns, `Sample::Reference` (weak_ptr) for UI
- **Singletons**: `initInstance()`, `cleanupInstance()`, `getInstance()`
- **Events**: `ChangeBroadcaster`/`ChangeListener`
- **Dialogs**: Always async (`launchAsync()`, never `runModalLoop()`)
- **Colors**: Use `ThemeManager`, not hex values

## Architecture (one-liner)

`Main.cpp` → `SamploreApplication` → `SamploreMainComponent`
`SamploreProperties` owns `SampleLibrary` + `AudioPlayer`
`SampleLibrary` broadcasts → UI listens via `ChangeListener`
