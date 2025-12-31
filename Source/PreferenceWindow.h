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

namespace samplore
{
    /// Overlay panel for application preferences
    class PreferenceWindow : public Component, public ThemeManager::Listener
    {
    public:
        //==============================================================================

        PreferenceWindow();
        ~PreferenceWindow() override;
        
        void paint(Graphics& g) override;
        void resized() override;
        
        /// Shows the preferences overlay
        void show();
        
        /// Hides the preferences overlay
        void hide();
        
        /// Callback when close is requested
        std::function<void()> onClose;
        
        //==================================================================
        // ThemeManager::Listener interface
        void themeChanged(ThemeManager::Theme newTheme) override;
        void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;

        class View : public Component, public Button::Listener, public TextEditor::Listener, public ChangeListener, public ComboBox::Listener, public ThemeManager::Listener
        {
        public:
            View();
            ~View() override;

            void buttonClicked(Button*) override;
            void textEditorTextChanged(TextEditor&) override;
            void changeListenerCallback(ChangeBroadcaster* source) override;
            void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

            void paint(Graphics& g) override;
            void resized() override;

            //==================================================================
            // ThemeManager::Listener interface
            void themeChanged(ThemeManager::Theme newTheme) override;
            void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;

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
                DirectoryListItem(std::shared_ptr<SampleDirectory> dir, bool isActive, View* parent);
                ~DirectoryListItem() override;
                
                void paint(Graphics& g) override;
                void resized() override;
                void buttonClicked(Button* button) override;
                
                std::shared_ptr<SampleDirectory> mDirectory;
                ToggleButton mActiveCheckbox;
                TextButton mDeleteButton;
                View* mParentView;
            };

            std::unique_ptr<ColourSelector> mColourSelector;
            enum class ColorEditMode { Primary, Accent };
            ColorEditMode mColorEditMode = ColorEditMode::Primary;
        };
    private:
        Label mTitleLabel;
        TextButton mCloseButton;
        Viewport mViewport;
        View mView;
        
        void updateColors();

    };
}
#endif