/*
  ==============================================================================

    CueManager.h
    Created: 2025
    Author:  Samplore Team

    Manages cue bindings using the InputContext system.
    When cue mode is enabled, the cue context takes priority.

  ==============================================================================
*/

#ifndef CUEMANAGER_H
#define CUEMANAGER_H

#include "JuceHeader.h"
#include "Sample.h"
#include "InputContext.h"

namespace samplore
{
    /// Represents cue binding data (key is stored in InputContext)
    struct CueBinding
    {
        Sample::Reference mSample;
        double mStartTime;          // Relative position 0.0 to 1.0
        juce::String mDisplayName;  // Optional display name
        float mHue;                 // 0.0-1.0 for display color

        CueBinding() : mSample(nullptr), mStartTime(0.0), mHue(0.0f) {}

        CueBinding(Sample::Reference sample, double startTime = 0.0,
                   const juce::String& name = "", float hue = -1.0f)
            : mSample(sample), mStartTime(startTime), mDisplayName(name)
        {
            // Generate random hue if not specified
            mHue = (hue < 0.0f) ? juce::Random::getSystemRandom().nextFloat() : hue;
        }

        bool isValid() const { return !mSample.isNull(); }

        /// Convert hue to display color
        juce::Colour getColor() const
        {
            const float saturation = 0.55f;  // Slightly higher for cue visibility
            const float brightness = 0.85f;
            return juce::Colour::fromHSV(mHue, saturation, brightness, 1.0f);
        }
    };

    /// Map type for cue bindings: KeyPress -> CueBinding
    using CueBindingMap = std::map<juce::KeyPress, CueBinding, KeyPressComparator>;

    /// A named collection of cue bindings
    struct CueRack
    {
        juce::String mName;
        CueBindingMap mBindings;

        CueRack() = default;
        CueRack(const juce::String& name) : mName(name) {}
    };

    /// CueManager singleton - manages cue bindings via InputContext
    class CueManager : public ChangeBroadcaster
    {
    public:
        static constexpr int CUE_CONTEXT_PRIORITY = 100;  // Higher than default (0)

        //======================================================================
        // Singleton pattern
        static void initInstance();
        static void cleanupInstance();
        static CueManager& getInstance();

        //======================================================================
        // Cue mode control (enables/disables the cue input context)
        bool isCueModeEnabled() const;
        void setCueModeEnabled(bool enabled);
        void toggleCueMode();

        //======================================================================
        // Binding management
        bool addBinding(const juce::KeyPress& key, Sample::Reference sample,
                        double startTime = 0.0, const juce::String& displayName = "");
        bool removeBinding(const juce::KeyPress& key);
        void removeAllBindingsForSample(Sample::Reference sample);
        void clearAllBindings();

        /// Get binding for a specific key (returns nullptr if not found)
        const CueBinding* getBinding(const juce::KeyPress& key) const;

        /// Get all bindings map (for UI display)
        const CueBindingMap& getBindings() const { return mBindings; }

        /// Check if a key has a binding
        bool hasBinding(const juce::KeyPress& key) const;

        //======================================================================
        // Direct trigger (for UI play buttons)
        void triggerBinding(const CueBinding& binding);

        //======================================================================
        // Available keys for binding
        static std::vector<juce::KeyPress> getAvailableKeys();
        static juce::String getKeyDisplayString(const juce::KeyPress& key);

        //======================================================================
        // Rack management
        juce::StringArray getRackNames() const;
        juce::String getCurrentRackName() const { return mCurrentRackName; }
        bool loadRack(const juce::String& name);
        bool saveCurrentRack();
        bool saveRackAs(const juce::String& name);
        bool createNewRack(const juce::String& name);
        bool deleteRack(const juce::String& name);
        bool renameRack(const juce::String& oldName, const juce::String& newName);

        //======================================================================
        // Persistence
        void saveAllRacks();
        void loadAllRacks();

        /// Get the cue input context
        std::shared_ptr<InputContext> getInputContext() { return mInputContext; }

    public:
        CueManager();
        ~CueManager();

    private:
        void rebuildInputContext();
        void triggerCue(const juce::KeyPress& key);
        juce::File getRacksDirectory() const;
        juce::File getRackFile(const juce::String& rackName) const;
        bool saveRackToFile(const CueRack& rack);
        bool loadRackFromFile(const juce::String& name, CueRack& outRack);

        CueBindingMap mBindings;
        juce::String mCurrentRackName;
        std::shared_ptr<InputContext> mInputContext;

        static std::unique_ptr<CueManager> sInstance;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CueManager)
    };
}

#endif // CUEMANAGER_H
