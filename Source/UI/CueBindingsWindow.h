/*
  ==============================================================================

    CueBindingsWindow.h
    Created: 2025
    Author:  Samplore Team

    Floating window for managing keyboard cue bindings

  ==============================================================================
*/

#ifndef CUEBINDINGSWINDOW_H
#define CUEBINDINGSWINDOW_H

#include "JuceHeader.h"
#include "../CueManager.h"
#include "../ThemeManager.h"

namespace samplore
{
    /// Row component for displaying a single cue binding
    class CueBindingRowComponent : public Component, public Button::Listener
    {
    public:
        CueBindingRowComponent(const juce::KeyPress& key, const CueBinding& binding);

        void paint(Graphics& g) override;
        void resized() override;
        void buttonClicked(Button* button) override;

    private:
        juce::KeyPress mKey;
        CueBinding mBinding;
        TextButton mPlayButton;
        TextButton mDeleteButton;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CueBindingRowComponent)
    };

    /// Content component showing all cue bindings with controls
    class CueBindingsListComponent : public Component,
                                      public ChangeListener,
                                      public Button::Listener,
                                      public ThemeManager::Listener
    {
    public:
        CueBindingsListComponent();
        ~CueBindingsListComponent() override;

        void paint(Graphics& g) override;
        void resized() override;

        void changeListenerCallback(ChangeBroadcaster* source) override;
        void buttonClicked(Button* button) override;

        // ThemeManager::Listener
        void themeChanged(ThemeManager::Theme newTheme) override;
        void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;

        void refreshBindingsList();

    private:
        ToggleButton mCueModeToggle;
        Label mCueModeLabel;
        Label mHeaderLabel;
        Viewport mBindingsViewport;
        Component mBindingsContainer;
        TextButton mClearAllButton;

        std::vector<std::unique_ptr<CueBindingRowComponent>> mRowComponents;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CueBindingsListComponent)
    };

    /// Floating window containing the cue bindings list
    class CueBindingsWindow : public DocumentWindow,
                               public ChangeListener,
                               public ThemeManager::Listener
    {
    public:
        CueBindingsWindow();
        ~CueBindingsWindow() override;

        void closeButtonPressed() override;
        void changeListenerCallback(ChangeBroadcaster* source) override;

        // ThemeManager::Listener
        void themeChanged(ThemeManager::Theme newTheme) override;
        void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;

    private:
        std::unique_ptr<CueBindingsListComponent> mContent;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CueBindingsWindow)
    };
}

#endif // CUEBINDINGSWINDOW_H
