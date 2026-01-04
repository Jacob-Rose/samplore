/*
  ==============================================================================

    PreferenceWindow.h
    Created: 13 Jun 2020 3:51:18pm
    Author:  jacob

  ==============================================================================
*/

#ifndef PREFERENCEWINDOW_H
#define PREFERENCEWINDOW_H
#include "JuceHeader.h"
#include "ThemeManager.h"
#include "KeyBindingEditor.h"
#include "SampleLibrary.h"
#include "UI/IOverlayPanelContent.h"

namespace samplore
{
    // Forward declaration
    class OverlayPanel;
    
    /// Content panel for application preferences (used with OverlayPanel)
    /// Implements IOverlayPanelContent to provide title
    class PreferencePanel : public Component, 
                            public Button::Listener, 
                            public TextEditor::Listener, 
                            public ChangeListener, 
                            public ComboBox::Listener, 
                            public ThemeManager::Listener,
                            public IOverlayPanelContent
    {
    public:
        PreferencePanel();
        ~PreferencePanel() override;

            void         buttonClicked(Button*) override;
        void textEditorTextChanged(TextEditor&) override;
        void changeListenerCallback(ChangeBroadcaster* source) override;
        void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

        void paint(Graphics& g) override;
        void resized() override;

        //==================================================================
        // ThemeManager::Listener interface
        void themeChanged(ThemeManager::Theme newTheme) override;
        void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;
        
        //==================================================================
        // IOverlayPanelContent interface
        String getOverlayTitle() const override { return "Preferences"; }
        bool shouldShowBackButton() const override { return false; }
        void onOverlayBackButton() override {}
        void setParentOverlay(OverlayPanel* parent) override { mParentOverlay = parent; }

    private:
        // Parent overlay panel (for requesting chrome refresh if needed)
        OverlayPanel* mParentOverlay = nullptr;
        
        // Theme section
        Label mThemeLabel;
        ComboBox mThemeSelector;

        // Color customization section
        Label mColorCustomizationLabel;
        Label mPrimaryColorLabel;
        TextButton mPrimaryColorButton;
        Label mAccentColorLabel;
        TextButton mAccentColorButton;
        TextButton mResetColorsButton;

        // Color presets section
        Label mColorPresetsLabel;
        TextButton mPresetStudioDark;
        TextButton mPresetStudioLight;
        TextButton mPresetAbleton;
        TextButton mPresetProTools;
        TextButton mPresetHighContrast;

        // Appearance section
        Label mAppearanceLabel;
        Label mTileSizeLabel;
        TextEditor mSampleMinSizeValue;
        Label mThumbnailLinesLabel;
        Label mThumbnailLinesPlayerLabel;
        TextEditor mThumbnailLineCountPlayer;
        TextEditor mThumbnailLineCount;
        Label mPlaybackIndicatorLabel;
        ComboBox mPlaybackIndicatorModeSelector;
        TextButton mPlaybackIndicatorColorButton;

        // Key bindings section
        Label mKeyBindingsLabel;
        TextButton mEditKeyBindingsButton;

        // Directory management section
        Label mDirectoryManagementLabel;
        TextButton mAddDirectoryButton;
        Viewport mDirectoryViewport;
        Component mDirectoryListContainer;

    private:
        void updateColorButtons();
        void applyColorPreset(const String& presetName);
        void updateAllComponentColors();
        void updateDirectoryList();
        
        // Directory list item component
        struct DirectoryListItem : public Component, public Button::Listener
        {
            DirectoryListItem(std::shared_ptr<SampleDirectory> dir, bool isActive, PreferencePanel* parent);
            ~DirectoryListItem() override;
            
            void paint(Graphics& g) override;
            void resized() override;
            void buttonClicked(Button* button) override;
            
            std::shared_ptr<SampleDirectory> mDirectory;
            ToggleButton mActiveCheckbox;
            TextButton mDeleteButton;
            PreferencePanel* mParentView;
        };

        std::unique_ptr<ColourSelector> mColourSelector;
        enum class ColorEditMode { Primary, Accent, PlaybackIndicator };
        ColorEditMode mColorEditMode = ColorEditMode::Primary;

        void updatePlaybackIndicatorColorButton();
    };
}
#endif