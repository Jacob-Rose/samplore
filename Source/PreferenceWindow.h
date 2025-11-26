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

namespace samplify
{
    class PreferenceWindow : public DialogWindow
    {
    public:
        //==============================================================================

        PreferenceWindow();
        void closeButtonPressed() override;
        bool keyPressed(const KeyPress& key) override;

        class View : public Component, public Button::Listener, public TextEditor::Listener, public ChangeListener, public ComboBox::Listener
        {
        public:
            View();

            void buttonClicked(Button*) override;
            void textEditorTextChanged(TextEditor&) override;
            void changeListenerCallback(ChangeBroadcaster* source) override;
            void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

            void paint(Graphics& g) override;
            void resized() override;

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
            TextButton mCloseButton;

        private:
            void updateColorButtons();
            void applyColorPreset(const String& presetName);

            std::unique_ptr<ColourSelector> mColourSelector;
            enum class ColorEditMode { Primary, Accent };
            ColorEditMode mColorEditMode = ColorEditMode::Primary;
        };
    private:
        View mView;

    };
}
#endif