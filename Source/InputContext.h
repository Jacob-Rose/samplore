/*
  ==============================================================================

    InputContext.h
    Created: 2025
    Author:  Samplore Team

    Layered input context system inspired by Unreal's Input Mapping Context.
    Contexts are checked in priority order (highest first). Each context can
    be enabled/disabled dynamically to layer input handling.

  ==============================================================================
*/

#ifndef INPUTCONTEXT_H
#define INPUTCONTEXT_H

#include "JuceHeader.h"
#include <map>
#include <functional>

namespace samplore
{
    /// Normalize a KeyPress to lowercase for consistent comparison
    inline juce::KeyPress normalizeKeyPress(const juce::KeyPress& key)
    {
        int keyCode = key.getKeyCode();
        if (keyCode >= 'A' && keyCode <= 'Z')
            keyCode += 32; // Convert to lowercase
        return juce::KeyPress(keyCode, key.getModifiers(), 0);
    }

    /// Comparator for KeyPress to use as map key
    struct KeyPressComparator
    {
        bool operator()(const juce::KeyPress& a, const juce::KeyPress& b) const
        {
            if (a.getKeyCode() != b.getKeyCode())
                return a.getKeyCode() < b.getKeyCode();
            return a.getModifiers().getRawFlags() < b.getModifiers().getRawFlags();
        }
    };

    /// A single input binding action
    struct InputBinding
    {
        juce::String mActionName;
        std::function<void()> mCallback;
        juce::String mDescription;  // For UI display
    };

    using InputBindingMap = std::map<juce::KeyPress, InputBinding, KeyPressComparator>;

    /// An input context (layer) containing key bindings
    class InputContext
    {
    public:
        InputContext(const juce::String& name, int priority = 0);

        /// Bind a key to an action
        void bind(const juce::KeyPress& key, const juce::String& actionName,
                  std::function<void()> callback, const juce::String& description = "");

        /// Unbind a key
        void unbind(const juce::KeyPress& key);

        /// Clear all bindings
        void clear();

        /// Try to handle a key press. Returns true if handled.
        bool tryHandle(const juce::KeyPress& key);

        /// Check if key is bound
        bool hasBinding(const juce::KeyPress& key) const;

        /// Get binding info for a key (nullptr if not bound)
        const InputBinding* getBinding(const juce::KeyPress& key) const;

        /// Enable/disable this context
        void setEnabled(bool enabled) { mEnabled = enabled; }
        bool isEnabled() const { return mEnabled; }

        /// Context info
        const juce::String& getName() const { return mName; }
        int getPriority() const { return mPriority; }

        /// Access all bindings (for UI)
        const InputBindingMap& getBindings() const { return mBindings; }

    private:
        juce::String mName;
        int mPriority;
        bool mEnabled = true;
        InputBindingMap mBindings;
    };

    /// Singleton managing all input contexts
    class InputContextManager : public ChangeBroadcaster
    {
    public:
        static void initInstance();
        static void cleanupInstance();
        static InputContextManager& getInstance();

        /// Create a new context with given priority (higher = checked first)
        std::shared_ptr<InputContext> createContext(const juce::String& name, int priority = 0);

        /// Get existing context by name
        std::shared_ptr<InputContext> getContext(const juce::String& name);

        /// Remove a context
        void removeContext(const juce::String& name);

        /// Handle a key press - iterates contexts by priority
        bool handleKeyPress(const juce::KeyPress& key);

        /// Get all contexts (sorted by priority, descending)
        const std::vector<std::shared_ptr<InputContext>>& getContexts() const { return mContexts; }

    public:
        InputContextManager();
        ~InputContextManager();

    private:
        void sortContexts();

        std::vector<std::shared_ptr<InputContext>> mContexts;
        static std::unique_ptr<InputContextManager> sInstance;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InputContextManager)
    };
}

#endif // INPUTCONTEXT_H
