/*
  ==============================================================================

    KeyBindingManager.cpp
    Created: 2025
    Author:  Samplore Team

  ==============================================================================
*/

#include "KeyBindingManager.h"
#include "SamplifyProperties.h"

using namespace samplore;

std::unique_ptr<KeyBindingManager> KeyBindingManager::sInstance = nullptr;

KeyBindingManager::KeyBindingManager()
{
    initializeActions();
}

void KeyBindingManager::initInstance()
{
    sInstance = std::make_unique<KeyBindingManager>();

    // Create our input context (priority 0 = global/default)
    sInstance->mContext = InputContextManager::getInstance().createContext("Global", 0);

    // Load any saved bindings
    sInstance->loadBindings();
}

void KeyBindingManager::cleanupInstance()
{
    if (sInstance)
    {
        sInstance->saveBindings();

        // Remove our context from the manager
        InputContextManager::getInstance().removeContext("Global");
        sInstance.reset();
    }
}

KeyBindingManager& KeyBindingManager::getInstance()
{
    jassert(sInstance != nullptr);
    return *sInstance;
}

void KeyBindingManager::initializeActions()
{
    // Audio playback
    mActions[Action::PlayAudio] = {
        juce::KeyPress('g'),  // default
        juce::KeyPress('g'),  // current (same initially)
        "Play",
        "Play the current sample"
    };

    mActions[Action::StopAudio] = {
        juce::KeyPress('h'),
        juce::KeyPress('h'),
        "Stop",
        "Stop audio playback"
    };

    // Window toggles
    mActions[Action::TogglePlayerWindow] = {
        juce::KeyPress('p', juce::ModifierKeys::ctrlModifier, 0),
        juce::KeyPress('p', juce::ModifierKeys::ctrlModifier, 0),
        "Toggle Player",
        "Show/hide player window"
    };

    mActions[Action::ToggleFilterWindow] = {
        juce::KeyPress('f', juce::ModifierKeys::ctrlModifier, 0),
        juce::KeyPress('f', juce::ModifierKeys::ctrlModifier, 0),
        "Toggle Filter",
        "Show/hide filter window"
    };

    mActions[Action::ToggleDirectoryWindow] = {
        juce::KeyPress('d', juce::ModifierKeys::ctrlModifier, 0),
        juce::KeyPress('d', juce::ModifierKeys::ctrlModifier, 0),
        "Toggle Directory",
        "Show/hide directory window"
    };

    // Application
#if JUCE_MAC
    mActions[Action::OpenPreferences] = {
        juce::KeyPress(',', juce::ModifierKeys::commandModifier, 0),
        juce::KeyPress(',', juce::ModifierKeys::commandModifier, 0),
        "Preferences",
        "Open preferences window"
    };
#else
    mActions[Action::OpenPreferences] = {
        juce::KeyPress('p', juce::ModifierKeys::ctrlModifier | juce::ModifierKeys::shiftModifier, 0),
        juce::KeyPress('p', juce::ModifierKeys::ctrlModifier | juce::ModifierKeys::shiftModifier, 0),
        "Preferences",
        "Open preferences window"
    };
#endif

    mActions[Action::ExitApplication] = {
        juce::KeyPress('q', juce::ModifierKeys::ctrlModifier, 0),
        juce::KeyPress('q', juce::ModifierKeys::ctrlModifier, 0),
        "Exit",
        "Exit application"
    };

    mActions[Action::ToggleCueBindings] = {
        juce::KeyPress('k', juce::ModifierKeys::ctrlModifier, 0),
        juce::KeyPress('k', juce::ModifierKeys::ctrlModifier, 0),
        "Cue Bindings",
        "Open cue bindings window"
    };
}

void KeyBindingManager::setCallback(Action action, std::function<void()> callback)
{
    mCallbacks[action] = callback;
    rebuildContext();
}

void KeyBindingManager::clearCallback(Action action)
{
    mCallbacks.erase(action);
    rebuildContext();
}

void KeyBindingManager::rebuildContext()
{
    if (!mContext)
        return;

    mContext->clear();

    for (const auto& [action, info] : mActions)
    {
        auto callbackIt = mCallbacks.find(action);
        if (callbackIt != mCallbacks.end() && callbackIt->second)
        {
            mContext->bind(info.currentKey,
                           info.displayName,
                           callbackIt->second,
                           info.description);
        }
    }
}

const KeyBindingManager::ActionInfo* KeyBindingManager::getActionInfo(Action action) const
{
    auto it = mActions.find(action);
    return (it != mActions.end()) ? &it->second : nullptr;
}

juce::KeyPress KeyBindingManager::getKey(Action action) const
{
    auto it = mActions.find(action);
    if (it != mActions.end())
        return it->second.currentKey;
    return {};
}

bool KeyBindingManager::setKey(Action action, const juce::KeyPress& newKey)
{
    // Check for conflicts with other bindings
    for (const auto& [otherAction, info] : mActions)
    {
        if (otherAction != action && info.currentKey == newKey)
            return false;  // Conflict
    }

    auto it = mActions.find(action);
    if (it != mActions.end())
    {
        it->second.currentKey = newKey;
        rebuildContext();
        saveBindings();
        return true;
    }

    return false;
}

void KeyBindingManager::resetKey(Action action)
{
    auto it = mActions.find(action);
    if (it != mActions.end())
    {
        it->second.currentKey = it->second.defaultKey;
        rebuildContext();
        saveBindings();
    }
}

void KeyBindingManager::resetAllKeys()
{
    for (auto& [action, info] : mActions)
    {
        info.currentKey = info.defaultKey;
    }
    rebuildContext();
    saveBindings();
}

juce::String KeyBindingManager::getActionName(Action action)
{
    switch (action)
    {
        case Action::PlayAudio:             return "Play Audio";
        case Action::StopAudio:             return "Stop Audio";
        case Action::TogglePlayerWindow:    return "Toggle Player Window";
        case Action::ToggleFilterWindow:    return "Toggle Filter Window";
        case Action::ToggleDirectoryWindow: return "Toggle Directory Window";
        case Action::OpenPreferences:       return "Open Preferences";
        case Action::ExitApplication:       return "Exit Application";
        case Action::ToggleCueBindings:     return "Toggle Cue Bindings";
        default:                            return "Unknown";
    }
}

juce::String KeyBindingManager::getKeyString(Action action) const
{
    return getKey(action).getTextDescription();
}

void KeyBindingManager::saveBindings()
{
    auto* props = SamplifyProperties::getInstance();
    if (props == nullptr)
        return;

    auto* settings = props->getUserSettings();
    if (settings == nullptr)
        return;

    for (const auto& [action, info] : mActions)
    {
        juce::String key = "keybind_" + juce::String(static_cast<int>(action));
        juce::String value = info.currentKey.getTextDescription();
        settings->setValue(key, value);
    }

    props->savePropertiesFile();
}

void KeyBindingManager::loadBindings()
{
    auto* props = SamplifyProperties::getInstance();
    if (props == nullptr)
        return;

    auto* settings = props->getUserSettings();
    if (settings == nullptr)
        return;

    for (auto& [action, info] : mActions)
    {
        juce::String key = "keybind_" + juce::String(static_cast<int>(action));
        juce::String value = settings->getValue(key, "");

        if (value.isNotEmpty())
        {
            auto keyPress = juce::KeyPress::createFromDescription(value);
            if (keyPress.isValid())
            {
                info.currentKey = keyPress;
            }
        }
    }

    // Rebuild context with loaded bindings (callbacks may not be set yet)
    rebuildContext();
}
