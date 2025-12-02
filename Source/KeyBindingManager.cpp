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

std::unique_ptr<KeyBindingManager> KeyBindingManager::instance = nullptr;

KeyBindingManager::KeyBindingManager()
{
    initializeDefaultBindings();
}

void KeyBindingManager::initInstance()
{
    instance = std::make_unique<KeyBindingManager>();
    instance->loadBindings();
}

void KeyBindingManager::cleanupInstance()
{
    if (instance)
    {
        instance->saveBindings();
        instance.reset();
    }
}

KeyBindingManager& KeyBindingManager::getInstance()
{
    return *instance;
}

void KeyBindingManager::initializeDefaultBindings()
{
    // Audio playback
    mBindings[Action::PlayAudio] = KeyBinding(
        juce::KeyPress('g'),
        "Play",
        "Play the current sample"
    );
    
    mBindings[Action::StopAudio] = KeyBinding(
        juce::KeyPress('h'),
        "Stop",
        "Stop audio playback"
    );
    
    // Window toggles (not yet implemented, but reserved)
    mBindings[Action::TogglePlayerWindow] = KeyBinding(
        juce::KeyPress('p', juce::ModifierKeys::ctrlModifier, 0),
        "Toggle Player",
        "Show/hide player window"
    );
    
    mBindings[Action::ToggleFilterWindow] = KeyBinding(
        juce::KeyPress('f', juce::ModifierKeys::ctrlModifier, 0),
        "Toggle Filter",
        "Show/hide filter window"
    );
    
    mBindings[Action::ToggleDirectoryWindow] = KeyBinding(
        juce::KeyPress('d', juce::ModifierKeys::ctrlModifier, 0),
        "Toggle Directory",
        "Show/hide directory window"
    );
    
    // Application
#if JUCE_MAC
    mBindings[Action::OpenPreferences] = KeyBinding(
        juce::KeyPress(',', juce::ModifierKeys::commandModifier, 0),
        "Preferences",
        "Open preferences window"
    );
#else
    mBindings[Action::OpenPreferences] = KeyBinding(
        juce::KeyPress('p', juce::ModifierKeys::ctrlModifier | juce::ModifierKeys::shiftModifier, 0),
        "Preferences",
        "Open preferences window"
    );
#endif
    
    mBindings[Action::ExitApplication] = KeyBinding(
        juce::KeyPress('q', juce::ModifierKeys::ctrlModifier, 0),
        "Exit",
        "Exit application"
    );
}

KeyBindingManager::KeyBinding KeyBindingManager::getBinding(Action action) const
{
    auto it = mBindings.find(action);
    if (it != mBindings.end())
        return it->second;
    
    return KeyBinding();
}

bool KeyBindingManager::setBinding(Action action, const juce::KeyPress& newKey)
{
    // Check for conflicts with other bindings
    for (const auto& pair : mBindings)
    {
        if (pair.first != action && pair.second.keyPress == newKey)
        {
            // Conflict detected
            return false;
        }
    }
    
    // Update binding
    auto it = mBindings.find(action);
    if (it != mBindings.end())
    {
        it->second.keyPress = newKey;
        saveBindings();
        return true;
    }
    
    return false;
}

void KeyBindingManager::resetBinding(Action action)
{
    // Temporarily save current bindings
    auto currentBindings = mBindings;
    
    // Reinitialize defaults
    initializeDefaultBindings();
    
    // Get the default for this action
    auto defaultBinding = mBindings[action];
    
    // Restore current bindings
    mBindings = currentBindings;
    
    // Apply the default
    mBindings[action] = defaultBinding;
    
    saveBindings();
}

void KeyBindingManager::resetAllBindings()
{
    initializeDefaultBindings();
    saveBindings();
}

bool KeyBindingManager::matchesAction(const juce::KeyPress& key, Action action) const
{
    auto binding = getBinding(action);
    return binding.keyPress == key;
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
        default:                            return "Unknown";
    }
}

juce::String KeyBindingManager::getKeyString(Action action) const
{
    auto binding = getBinding(action);
    return binding.keyPress.getTextDescription();
}

void KeyBindingManager::resetToDefaults()
{
    initializeDefaultBindings();
    saveBindings();
}

void KeyBindingManager::saveBindings()
{
    auto* props = SamplifyProperties::getInstance();
    if (props == nullptr)
        return;

    auto* settings = props->getUserSettings();
    if (settings == nullptr)
        return;

    // Save each binding
    for (const auto& pair : mBindings)
    {
        juce::String key = "keybind_" + juce::String((int)pair.first);
        juce::String value = pair.second.keyPress.getTextDescription();
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

    // Load each binding
    for (auto& pair : mBindings)
    {
        juce::String key = "keybind_" + juce::String((int)pair.first);
        juce::String value = settings->getValue(key, "");
        
        if (value.isNotEmpty())
        {
            auto keyPress = juce::KeyPress::createFromDescription(value);
            if (keyPress.isValid())
            {
                pair.second.keyPress = keyPress;
            }
        }
    }
}
