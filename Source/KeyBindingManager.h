/*
  ==============================================================================

    KeyBindingManager.h
    Created: 2025
    Author:  Samplore Team

    Centralized key binding management system with persistence and rebinding

  ==============================================================================
*/

#ifndef KEYBINDINGMANAGER_H
#define KEYBINDINGMANAGER_H

#include "JuceHeader.h"
#include <map>

namespace samplore
{
    class KeyBindingManager
    {
    public:
        /// Identifiers for all bindable actions
        enum class Action
        {
            PlayAudio,
            StopAudio,
            TogglePlayerWindow,
            ToggleFilterWindow,
            ToggleDirectoryWindow,
            OpenPreferences,
            ExitApplication
        };

        /// Represents a key binding with modifiers
        struct KeyBinding
        {
            juce::KeyPress keyPress;
            juce::String displayName;
            juce::String description;

            KeyBinding() = default;
            KeyBinding(const juce::KeyPress& kp, const juce::String& name, const juce::String& desc)
                : keyPress(kp), displayName(name), description(desc) {}
        };

        //======================================================================
        // Singleton pattern
        static void initInstance();
        static void cleanupInstance();
        static KeyBindingManager& getInstance();

        //======================================================================
        /// Get the key binding for an action
        KeyBinding getBinding(Action action) const;
        
        /// Set/rebind a key for an action
        bool setBinding(Action action, const juce::KeyPress& newKey);
        
        /// Reset a specific binding to default
        void resetBinding(Action action);
        
        /// Reset all bindings to defaults
        void resetAllBindings();
        
        /// Check if a key press matches an action
        bool matchesAction(const juce::KeyPress& key, Action action) const;
        
        /// Get all bindings (for UI display)
        std::map<Action, KeyBinding> getAllBindings() const { return mBindings; }
        
        /// Get action name for display
        static juce::String getActionName(Action action);
        
        /// Get key string for display
        juce::String getKeyString(Action action) const;
        
        /// Reset all bindings to defaults
        void resetToDefaults();

        //======================================================================
        // Persistence
        void saveBindings();
        void loadBindings();

    public:
        KeyBindingManager();
    private:
        void initializeDefaultBindings();

        std::map<Action, KeyBinding> mBindings;
        static std::unique_ptr<KeyBindingManager> instance;
    };
}

#endif // KEYBINDINGMANAGER_H
