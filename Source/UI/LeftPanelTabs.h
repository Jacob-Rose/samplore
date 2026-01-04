/*
  ==============================================================================

    LeftPanelTabs.h
    Created: 2025
    Author:  Samplore Team

    Tabbed container for the left panel (Folders / Cues)

  ==============================================================================
*/

#ifndef LEFTPANELTABS_H
#define LEFTPANELTABS_H

#include "JuceHeader.h"
#include "../DirectoryExplorer.h"
#include "../CueManager.h"
#include "../ThemeManager.h"

namespace samplore
{
    /// Simplified cue panel for embedding in tabs (no window chrome)
    class CuePanel : public Component,
                     public ChangeListener,
                     public Button::Listener,
                     public ComboBox::Listener,
                     public ThemeManager::Listener
    {
    public:
        CuePanel();
        ~CuePanel() override;

        void paint(Graphics& g) override;
        void resized() override;

        void changeListenerCallback(ChangeBroadcaster* source) override;
        void buttonClicked(Button* button) override;
        void comboBoxChanged(ComboBox* comboBox) override;

        void themeChanged(ThemeManager::Theme newTheme) override;
        void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;

    private:
        class CueRowComponent : public Component, public Button::Listener
        {
        public:
            CueRowComponent(const juce::KeyPress& key, const CueBinding& binding);
            void paint(Graphics& g) override;
            void resized() override;
            void buttonClicked(Button* button) override;

        private:
            juce::KeyPress mKey;
            CueBinding mBinding;
            TextButton mPlayButton{">"};
            TextButton mDeleteButton{"X"};
        };

        void refreshBindings();
        void refreshRackList();
        void showNewRackDialog();
        void showDeleteRackConfirmation();

        // Rack selector row
        ComboBox mRackSelector;
        TextButton mNewRackButton{"+"};
        TextButton mDeleteRackButton{"-"};

        ToggleButton mCueModeToggle;
        Label mCueModeLabel;
        Viewport mBindingsViewport;
        Component mBindingsContainer;
        TextButton mClearAllButton{"Clear All"};

        std::vector<std::unique_ptr<CueRowComponent>> mRows;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CuePanel)
    };

    /// Tabbed container with Folders and Cues tabs
    class LeftPanelTabs : public Component, public ThemeManager::Listener
    {
    public:
        LeftPanelTabs();
        ~LeftPanelTabs() override;

        void paint(Graphics& g) override;
        void resized() override;

        void themeChanged(ThemeManager::Theme newTheme) override;
        void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;

        DirectoryExplorer& getDirectoryExplorer() { return mDirectoryExplorer; }

    private:
        TabbedComponent mTabs{TabbedButtonBar::TabsAtTop};
        DirectoryExplorer mDirectoryExplorer;
        CuePanel mCuePanel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LeftPanelTabs)
    };
}

#endif // LEFTPANELTABS_H
