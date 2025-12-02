/*
  ==============================================================================

    KeyBindingEditor.h
    Author:  Jake Rose

    UI component for editing key bindings

  ==============================================================================
*/

#ifndef KEYBINDINGEDITOR_H
#define KEYBINDINGEDITOR_H

#include "JuceHeader.h"
#include "KeyBindingManager.h"
#include "ThemeManager.h"

namespace samplore
{

class KeyBindingEditor : public Component,
                        public ThemeManager::Listener,
                        private Button::Listener,
                        public KeyListener
{
public:
    KeyBindingEditor();
    ~KeyBindingEditor() override;

    void paint(Graphics& g) override;
    void resized() override;
    
    // ThemeManager::Listener interface
    void themeChanged(ThemeManager::Theme newTheme) override;
    void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;
    
    // KeyListener interface
    bool keyPressed(const KeyPress& key, Component* originatingComponent) override;

private:
    void buttonClicked(Button* button) override;
    void rebuildKeyBindingList();
    void startKeyCapture(KeyBindingManager::Action action);
    void updateAllComponentColors();
    
    struct KeyBindingRow
    {
        KeyBindingManager::Action action;
        Label* actionLabel;
        Label* keyLabel;
        TextButton* rebindButton;
    };
    
    Array<KeyBindingRow> mKeyRows;
    Viewport mViewport;
    Component mContentComponent;
    TextButton mResetButton;
    TextButton mCloseButton;
    
    // For key capture
    bool mIsCapturingKey = false;
    KeyBindingManager::Action mCapturingAction;
    Label mCaptureLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KeyBindingEditor)
};

}

#endif