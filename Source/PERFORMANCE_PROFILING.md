# Performance Profiling Guide

This document explains how to profile paint performance in Samplore using the built-in tools.

## Quick Start

### 1. Enable Profiling

Add to your main component's constructor or initialization:

```cpp
// In SamplifyMainComponent.cpp constructor
#if JUCE_DEBUG
    PerformanceProfiler::getInstance().setEnabled(true);
#endif
```

### 2. Profile a Component's Paint Method

Add a single line at the top of any `paint()` method:

```cpp
void SampleTile::paint(Graphics& g)
{
    PROFILE_PAINT("SampleTile::paint");
    
    // Your existing paint code...
}
```

### 3. View Results

Print statistics to the debug console:

```cpp
// Add a keyboard shortcut or menu item to print stats
void SamplifyMainComponent::keyPressed(const KeyPress& key)
{
    if (key.getKeyCode() == KeyPress::F5Key)
    {
        PerformanceProfiler::getInstance().printStatistics();
    }
}
```

Or use JUCE's built-in command:

```cpp
// In your menu bar or keyboard handler
PerformanceProfiler::getInstance().printStatistics();
```

## Built-in JUCE Tools

### Option 1: PerformanceCounter (for repeated operations)

Best for measuring operations that repeat many times:

```cpp
// As a member variable
PerformanceCounter mPaintCounter {"SampleTile::paint", 100};

// In paint method
void SampleTile::paint(Graphics& g)
{
    mPaintCounter.start();
    
    // Your paint code...
    
    mPaintCounter.stop(); // Prints stats every 100 calls
}
```

### Option 2: ScopedTimeMeasurement (for one-off measurements)

Best for measuring a single operation:

```cpp
void expensiveOperation()
{
    double timeTaken = 0.0;
    
    {
        ScopedTimeMeasurement timer(timeTaken);
        // Code to measure...
    }
    
    DBG("Operation took " + String(timeTaken * 1000.0, 2) + " ms");
}
```

### Option 3: Manual Timing (most flexible)

```cpp
void paint(Graphics& g)
{
    auto startTime = Time::getMillisecondCounterHiRes();
    
    // Your paint code...
    
    auto elapsed = Time::getMillisecondCounterHiRes() - startTime;
    if (elapsed > 16.0) // Flag slow paints (>16ms = <60fps)
    {
        DBG("SLOW PAINT: " + String(elapsed, 2) + " ms");
    }
}
```

## Custom PerformanceProfiler (Recommended)

The `PerformanceProfiler` class provides aggregated statistics across all components.

### Basic Usage

```cpp
#include "PerformanceProfiler.h"

void SampleTile::paint(Graphics& g)
{
    PROFILE_PAINT("SampleTile::paint");
    // Rest of paint code...
}

void SampleExplorer::paint(Graphics& g)
{
    PROFILE_PAINT("SampleExplorer::paint");
    // Rest of paint code...
}
```

### Profiling Specific Sections

```cpp
void ComplexComponent::paint(Graphics& g)
{
    {
        PROFILE_PAINT("ComplexComponent::drawBackground");
        drawBackground(g);
    }
    
    {
        PROFILE_PAINT("ComplexComponent::drawContent");
        drawContent(g);
    }
    
    {
        PROFILE_PAINT("ComplexComponent::drawOverlay");
        drawOverlay(g);
    }
}
```

### Viewing Statistics

Add a keyboard shortcut (e.g., F5) to print stats:

```cpp
bool SamplifyMainComponent::keyPressed(const KeyPress& key)
{
    if (key == KeyPress::F5Key)
    {
        PerformanceProfiler::getInstance().printStatistics();
        return true;
    }
    
    if (key == KeyPress::F6Key)
    {
        PerformanceProfiler::getInstance().reset();
        DBG("Performance stats reset");
        return true;
    }
    
    return false;
}
```

### Example Output

```
========================================
Paint Performance Statistics
========================================
SampleTile::paint:
  Count: 1523
  Total: 2847.32 ms
  Avg:   1.870 ms
  Min:   0.412 ms
  Max:   23.145 ms
  Last:  1.634 ms
SampleExplorer::paint:
  Count: 47
  Total: 1234.56 ms
  Avg:   26.268 ms
  Min:   18.234 ms
  Max:   45.123 ms
  Last:  22.341 ms
========================================
```

## Tips for Finding Performance Bottlenecks

### 1. Profile Everything First

Add `PROFILE_PAINT()` to all major components:
- `SampleTile`
- `SampleExplorer`
- `TagExplorer`
- `AudioPlayer`
- `SamplifyMainComponent`
- Any custom components with complex paint code

### 2. Look for High Totals

Components with high total time are being repainted too often or are too slow.

### 3. Look for High Averages

Components with high average times have expensive paint operations.

### 4. Check Max Times

Occasional spikes can indicate:
- First paint (loading resources)
- Resizing operations
- Theme changes
- Font rendering

### 5. Profile Subsections

If a component is slow, break down the paint method:

```cpp
void SampleTile::paint(Graphics& g)
{
    {
        PROFILE_PAINT("SampleTile::shadows");
        drawShadows(g);
    }
    
    {
        PROFILE_PAINT("SampleTile::background");
        g.fillRoundedRectangle(...);
    }
    
    {
        PROFILE_PAINT("SampleTile::text");
        g.drawText(...);
    }
    
    {
        PROFILE_PAINT("SampleTile::waveform");
        drawWaveform(g);
    }
}
```

## Common Performance Issues

### Issue: Many Small Repaints

**Symptom:** High count, low average time  
**Fix:** Reduce unnecessary `repaint()` calls, use `repaint(Rectangle)` for partial updates

### Issue: Slow Individual Paints

**Symptom:** Low count, high average time  
**Fix:** Optimize paint code, cache rendered content, use images for complex graphics

### Issue: Excessive Repaints During Scrolling

**Symptom:** Count increases rapidly when scrolling  
**Fix:** Use `setBufferedToImage(true)` for complex components

### Issue: Font Rendering Slowness

**Symptom:** High time in text drawing operations  
**Fix:** Cache font objects, reduce font switches, use simpler fonts

## Best Practices

1. **Only profile in Debug builds** - Check `JUCE_DEBUG` before enabling
2. **Profile representative workloads** - Load typical number of samples/tags
3. **Profile on target hardware** - Performance varies by platform
4. **Reset between tests** - Use F6 or `reset()` to clear stats
5. **Compare before/after** - Measure baseline before optimizing
6. **Focus on the worst offenders** - Optimize the slowest 20% first

## Integration with KeyBindings

Add profiling commands to `KeyBindingManager`:

```cpp
// In KeyBindingManager.h
enum CommandIDs
{
    // ... existing commands
    showPerformanceStats = 9000,
    resetPerformanceStats = 9001,
};

// In KeyBindingManager.cpp
void registerDefaultKeyBindings()
{
    addKeyPress(CommandIDs::showPerformanceStats, KeyPress::F5Key);
    addKeyPress(CommandIDs::resetPerformanceStats, KeyPress::F6Key);
}

// In command handler
void handleCommand(int commandID)
{
    switch (commandID)
    {
        case CommandIDs::showPerformanceStats:
            PerformanceProfiler::getInstance().printStatistics();
            break;
            
        case CommandIDs::resetPerformanceStats:
            PerformanceProfiler::getInstance().reset();
            break;
    }
}
```

## Advanced: Real-time Performance Overlay

For a visual overlay showing FPS and paint times:

```cpp
class PerformanceOverlay : public Component, public Timer
{
public:
    PerformanceOverlay()
    {
        setInterceptsMouseClicks(false, false);
        startTimer(500); // Update twice per second
    }
    
    void paint(Graphics& g) override
    {
        g.setFont(FontOptions(12.0f));
        g.setColour(Colours::white);
        g.fillRect(0, 0, 200, 60);
        g.setColour(Colours::black);
        g.drawText("FPS: " + String(mFps, 1), 5, 5, 190, 20, Justification::left);
        g.drawText("Paint: " + String(mLastPaintTime, 2) + " ms", 5, 25, 190, 20, Justification::left);
    }
    
    void timerCallback() override
    {
        // Calculate FPS from recent paints
        // Update display
        repaint();
    }
    
private:
    float mFps = 0.0f;
    float mLastPaintTime = 0.0f;
};
```
