/*
  ==============================================================================

    KeyBindingManager.h
    Created: 2025
    Author:  Samplore Team

    Centralized key binding management system with persistence and rebinding.
    Uses InputContextManager internally for key handling.

  ==============================================================================
*/

#ifndef KEYBINDINGMANAGER_H
#define KEYBINDINGMANAGER_H

#include "JuceHeader.h"
#include "InputContext.h"
#include <map>
#include <functional>

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
            ExitApplication,
            ToggleCueBindings
        };

        /// Metadata for an action (does not include callback)
        struct ActionInfo
        {
            juce::KeyPress defaultKey;
            juce::KeyPress currentKey;
            juce::String displayName;
            juce::String description;
        };

        //======================================================================
        // Singleton pattern
        static void initInstance();
        static void cleanupInstance();
        static KeyBindingManager& getInstance();

        //======================================================================
        /// Set callback for an action - this is how components register their handlers
        void setCallback(Action action, std::function<void()> callback);

        /// Remove callback for an action
        void clearCallback(Action action);

        /// Get info about an action
        const ActionInfo* getActionInfo(Action action) const;

        /// Get current key for an action
        juce::KeyPress getKey(Action action) const;

        /// Set/rebind a key for an action (returns false if conflict)
        bool setKey(Action action, const juce::KeyPress& newKey);

        /// Reset a specific binding to default
        void resetKey(Action action);

        /// Reset all bindings to defaults
        void resetAllKeys();

        /// Get all actions (for UI display)
        const std::map<Action, ActionInfo>& getAllActions() const { return mActions; }

        /// Get action name for display
        static juce::String getActionName(Action action);

        /// Get key string for display
        juce::String getKeyString(Action action) const;

        //======================================================================
        // Persistence
        void saveBindings();
        void loadBindings();

    public:
        KeyBindingManager();

    private:
        void initializeActions();
        void rebuildContext();

        std::map<Action, ActionInfo> mActions;
        std::map<Action, std::function<void()>> mCallbacks;
        std::shared_ptr<InputContext> mContext;

        static std::unique_ptr<KeyBindingManager> sInstance;
    };
}

#endif // KEYBINDINGMANAGER_H
