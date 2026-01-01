# Scrolling Performance Analysis - Root Cause Found

## Problem Statement

Playback works fine when NOT scrolling, but hitches/stutters when scrolling. This indicates scrolling is performing synchronous/blocking operations that interfere with the audio thread.

## Root Cause: Synchronous File I/O During Scroll

### **Critical Issue Found:**

**`Sample.cpp:270-276` - `generateThumbnailAndCache()`**

```cpp
void Sample::Reference::generateThumbnailAndCache()
{
    std::shared_ptr<Sample> sample = mSample.lock();
    if (!isNull() && sample->mThumbnail == nullptr)
    {
        sample->mThumbnailCache = std::make_shared<AudioThumbnailCache>(1);
        AudioFormatManager* afm = SamplifyProperties::getInstance()->getAudioPlayer()->getFormatManager();
        sample->mThumbnail = std::make_shared<SampleAudioThumbnail>(512, *afm, *sample->mThumbnailCache);
        sample->mThumbnail->addChangeListener(sample->getChangeListener());
        AudioFormatReader* reader = afm->createReaderFor(sample->mFile);  // ⚠️ SYNCHRONOUS FILE I/O!
        if (reader != nullptr)
        {
            sample->mThumbnail->setSource(new FileInputSource(sample->mFile));  // ⚠️ MORE FILE I/O!
            sample->mLength = (float)reader->lengthInSamples / reader->sampleRate;
        }
        delete reader;
        
        // Save properties file after generating thumbnail (creates file if it doesn't exist)
        sample->savePropertiesFile();  // ⚠️ EVEN MORE FILE I/O!
    }
}
```

### **Call Chain During Scroll:**

1. **User scrolls** → Viewport position changes
2. **`SampleExplorer::visibleAreaChanged()`** (`SampleExplorer.cpp:216-228`)
   - Calls `mSampleContainer->updateVisibleItems()`
3. **`SampleContainer::updateVisibleItems()`** (`SampleContainer.cpp:37-101`)
   - Lines 78-90: For each newly visible tile:
     - Calls `tile->setSample(mCurrentSamples[sampleIndex])`
4. **`SampleTile::setSample()`** (`SampleTile.cpp:388-418`)
   - Line 407: Calls `sample.generateThumbnailAndCache()`
5. **`Sample::generateThumbnailAndCache()`** (`Sample.cpp:261-282`)
   - **Line 270**: `afm->createReaderFor(sample->mFile)` - **BLOCKS ON FILE I/O**
   - **Line 273**: `setSource(new FileInputSource(...))` - **BLOCKS ON FILE I/O**
   - **Line 279**: `savePropertiesFile()` - **BLOCKS ON FILE I/O**

### **Why This Causes Audio Hitches:**

1. **MessageManager Thread Blocking**: 
   - All JUCE UI operations (including scrolling) run on the MessageManager thread
   - File I/O blocks this thread, delaying UI updates and event processing
   - Audio thread may need to communicate with UI thread for state updates

2. **Shared AudioFormatManager**:
   - Both UI and audio threads likely access the same `AudioFormatManager`
   - Creating readers may involve mutex locks that block the audio thread

3. **Multiple Files Per Scroll**:
   - Scrolling exposes ~3-5 new tiles at once (visible range ±1 buffer)
   - Each tile can trigger file I/O if thumbnail not cached
   - Total blocking time = 3-5 × (file open + read + properties save)

4. **Cascading Effect**:
   - While blocked on file I/O, more scroll events queue up
   - Each new visible tile adds more file I/O to the queue
   - Audio callbacks get starved of CPU time

---

## Secondary Performance Issues

### **1. Dynamic Buffering Toggle During Playback**

**`SampleTile.cpp:150-161`**

```cpp
// Dynamic buffering: toggle after paint completes (can't change during paint)
if (isCurrentlyPlaying != mIsPlaying)
{
    mIsPlaying = isCurrentlyPlaying;
    // Defer buffering change until after paint() completes to avoid crash
    Component::SafePointer<SampleTile> safeThis(this);
    MessageManager::callAsync([safeThis, shouldBuffer = !mIsPlaying]() {
        if (safeThis != nullptr)
            safeThis->setBufferedToImage(shouldBuffer);  // ⚠️ Image buffer reallocation
    });
```

**Issue**: `setBufferedToImage(false)` and `setBufferedToImage(true)` may cause:
- Image buffer allocation/deallocation
- Component invalidation across large areas
- Potential memory allocations on MessageManager thread

### **2. Tile Pool Expansion During Scroll**

**`SampleContainer.cpp:70-75`**

```cpp
// Ensure we have enough tiles in the pool
while ((int)mTilePool.size() < visibleCount)
{
    auto tile = std::make_unique<SampleTile>(nullptr);  // ⚠️ Heavy allocation
    addAndMakeVisible(tile.get());
    mTilePool.push_back(std::move(tile));
}
```

**Issue**: Creating new `SampleTile` objects during scroll:
- Memory allocation
- Component tree modification (`addAndMakeVisible`)
- Font cache initialization (static but still accessed)
- Theme listener registration

**Impact**: Minor compared to file I/O, but adds latency

### **3. setBounds() Calls on Every Scroll**

**`SampleContainer.cpp:86-89`**

```cpp
tile->setBounds((column * tileWidth) + padding,
                (row * tileHeight) + padding,
                tileWidth - (padding * 2),
                tileHeight - (padding * 2));
```

**Issue**: Even if tile already has correct bounds, `setBounds()` is called
- Triggers `resized()` callback
- Invalidates component layout
- May trigger unnecessary repaints

**Impact**: Negligible, but could be optimized with bounds checking

### **4. repaint() Called for Every Tile Update**

**`SampleTile.cpp:417`**

```cpp
mSample = sample;
repaint();  // ⚠️ Even if sample didn't change
```

**Issue**: `repaint()` called even when tile is reusing same sample

**Impact**: Minor, but adds to repaint queue

---

## Performance Metrics

### **Estimated Blocking Times** (per file):

| Operation | Estimated Time | Severity |
|-----------|---------------|----------|
| `createReaderFor()` | 1-10ms (HDD) / 0.1-1ms (SSD) | **Critical** |
| `setSource()` | 1-5ms (initial read) | **Critical** |
| `savePropertiesFile()` | 1-10ms (write to disk) | **High** |
| Tile allocation | 0.1-0.5ms | Low |
| `setBufferedToImage()` | 0.5-2ms | Medium |

### **Audio Thread Impact**:

- **Audio buffer size**: Typically 512-1024 samples at 48kHz = ~10-20ms
- **Single file I/O**: 3-25ms (can exceed audio buffer!)
- **3-5 tiles during scroll**: 9-125ms total blocking
- **Result**: Audio underruns/stutters

---

## Solutions

### **Critical Fix #1: Async Thumbnail Generation**

Move file I/O off the MessageManager thread:

```cpp
void Sample::Reference::generateThumbnailAndCache()
{
    std::shared_ptr<Sample> sample = mSample.lock();
    if (!isNull() && sample->mThumbnail == nullptr)
    {
        // Mark as "loading" to prevent duplicate requests
        sample->mThumbnail = std::make_shared<SampleAudioThumbnail>(512, *afm, *cache);
        
        // Launch async task
        std::thread([sample]() {
            AudioFormatManager afm;
            afm.registerBasicFormats();
            
            AudioFormatReader* reader = afm.createReaderFor(sample->mFile);
            if (reader != nullptr)
            {
                sample->mThumbnail->setSource(new FileInputSource(sample->mFile));
                sample->mLength = (float)reader->lengthInSamples / reader->sampleRate;
                delete reader;
                
                // Trigger repaint on MessageManager thread
                MessageManager::callAsync([sample]() {
                    sample->sendChangeMessage();
                });
            }
        }).detach();
    }
}
```

**Alternatives**:
- Use `juce::Thread` or `juce::ThreadPool`
- Pre-generate thumbnails during library scan
- Use thumbnail cache more aggressively

### **Critical Fix #2: Thumbnail Pre-Loading**

Generate thumbnails when library loads, not during scroll:

**`SampleLibrary.cpp` - in background update task**
```cpp
for (auto& sample : samples)
{
    sample.generateThumbnailAndCache();  // Do this BEFORE adding to UI
}
```

### **High Priority Fix #3: Check Before setBounds()**

**`SampleContainer.cpp:86`**
```cpp
Rectangle<int> newBounds((column * tileWidth) + padding,
                        (row * tileHeight) + padding,
                        tileWidth - (padding * 2),
                        tileHeight - (padding * 2));

if (tile->getBounds() != newBounds)
    tile->setBounds(newBounds);
```

### **Medium Priority Fix #4: Check Before setSample()**

**`SampleContainer.cpp:90`**
```cpp
if (tile->getSample() != mCurrentSamples[sampleIndex])
    tile->setSample(mCurrentSamples[sampleIndex]);
```

### **Medium Priority Fix #5: Skip repaint() if Sample Unchanged**

**`SampleTile.cpp:417`**
```cpp
if (mSample != sample)
{
    mSample = sample;
    repaint();
}
else
{
    mSample = sample;
}
```

### **Low Priority Fix #6: Pre-allocate Tile Pool**

**`SampleContainer.cpp` - in constructor or setSampleItems()**
```cpp
// Pre-allocate enough tiles for maximum visible area
int maxVisibleTiles = (getHeight() / getTileHeight() + 2) * getColumnCount();
while (mTilePool.size() < maxVisibleTiles)
{
    auto tile = std::make_unique<SampleTile>(nullptr);
    addAndMakeVisible(tile.get());
    mTilePool.push_back(std::move(tile));
}
```

---

## Testing Strategy

### **1. Confirm Root Cause**

Add timing instrumentation:

```cpp
// In Sample::generateThumbnailAndCache()
auto start = Time::getHighResolutionTicks();
AudioFormatReader* reader = afm->createReaderFor(sample->mFile);
auto duration = Time::getHighResolutionTicksPerSecond() / 
                (Time::getHighResolutionTicks() - start);
DBG("createReaderFor took " << duration << "ms");
```

### **2. Measure Improvement**

- Use existing `PerformanceProfiler` (F6/F7/F8 keys)
- Monitor audio dropouts in DAW/system audio meter
- Profile with `perf` on Linux or Instruments on macOS

### **3. Verify No Deadlocks**

- Ensure async thumbnail loading doesn't cause race conditions
- Check that audio thread never waits on UI thread
- Test with slow HDDs to expose timing issues

---

## Implementation Priority

1. **CRITICAL**: Async thumbnail generation OR pre-loading during library scan
2. **HIGH**: Check before `setSample()` to avoid redundant file I/O
3. **HIGH**: Check before `setBounds()` to avoid redundant layout
4. **MEDIUM**: Skip `repaint()` if sample unchanged
5. **MEDIUM**: Pre-allocate tile pool
6. **LOW**: Optimize `setBufferedToImage()` toggling

---

## Architecture Recommendations

### **Long-term Solution: Thumbnail Cache Database**

Instead of generating thumbnails on-demand:

1. **Library scan phase**: Generate ALL thumbnails asynchronously
2. **Store in SQLite database**: Already using `sqlite3.c` in `Source/ThirdParty/`
3. **Load thumbnails from cache**: No file I/O during scroll
4. **Invalidate on file change**: Check mtime/hash

### **Audio Thread Protection**

Ensure audio thread NEVER blocks on UI operations:

1. **Separate AudioFormatManager**: One for UI, one for audio thread
2. **Lock-free state sharing**: Use atomics for play state
3. **Audio thread priority**: Boost audio callback thread priority

### **Profiling Infrastructure**

Enhance existing `PerformanceProfiler`:

1. **Track file I/O separately**: Measure `createReaderFor()` time
2. **Audio dropout detection**: Count underruns
3. **Flame graph support**: Export to trace format

---

## Related Files

| File | Issue | Line Numbers |
|------|-------|--------------|
| `Source/Sample.cpp` | **CRITICAL: Sync file I/O** | 270-276, 279 |
| `Source/SampleTile.cpp` | Calls sync file I/O | 407 |
| `Source/SampleTile.cpp` | Redundant repaint | 417 |
| `Source/SampleTile.cpp` | Dynamic buffering toggle | 150-161 |
| `Source/SampleContainer.cpp` | Tile pool expansion | 70-75 |
| `Source/SampleContainer.cpp` | Redundant setBounds | 86-89 |
| `Source/SampleContainer.cpp` | Redundant setSample | 90 |

---

## Conclusion

The **root cause** of audio hitching during scroll is **synchronous file I/O in `Sample::generateThumbnailAndCache()`**. Every newly visible tile can trigger 3-25ms of blocking disk access, which exceeds typical audio buffer sizes and causes underruns.

**Fix priority**:
1. Move thumbnail generation to async thread OR pre-generate during library scan
2. Add guards to prevent redundant setSample() calls
3. Optimize tile pool allocation

The other fixes we implemented (timer-based playback indicators) are still valuable for general performance, but they don't address the core scrolling issue.
