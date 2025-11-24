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
        void closeButtonPressed() override { exitModalState(0); }

        class View : public Component, public Button::Listener, public TextEditor::Listener, public ChangeListener
        {
        public:
            View();

            void buttonClicked(Button*) override;
            void textEditorTextChanged(TextEditor&) override;
            void changeListenerCallback(ChangeBroadcaster* source) override;

            void paint(Graphics& g) override;

            void resized() override;
            TextButton mPrimaryColorButton;
            TextButton mAccentColorButton;
            TextEditor mSampleMinSizeValue;
            TextEditor mThumbnailLineCount;
        private:
            std::unique_ptr<ColourSelector> mColourSelector;
            bool mEditingPrimaryColor = false;
        };
    private:
        View mView;

    };
}
#endif