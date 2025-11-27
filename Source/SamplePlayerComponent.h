/*
  ==============================================================================

    SamplePlayerComponent.h
    Created: 4 Apr 2020 7:43:10pm
    Author:  jacob

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Sample.h"
#include "TagContainer.h"

namespace samplore
{
    class SamplePlayerComponent : public Component, public ChangeListener, public TextEditor::Listener, public Button::Listener
    {
    public:

        enum ColourIds
        {
            waveformColourId = 0x1245,
            backgroundColourId,
            textColourId
        };
        SamplePlayerComponent();
        ~SamplePlayerComponent();

        void changeListenerCallback(ChangeBroadcaster* source) override;

        void textEditorTextChanged(TextEditor&) override;

        void buttonClicked(Button*) override;

        void paint(Graphics&) override;
        void resized() override;

        void mouseDown(const MouseEvent& event) override;

        Sample::Reference getCurrentSample();

        void updateThemeColors();

    private:
        TextEditor mSampleInfoEditor;
        TextButton mSampleColorSelectorButton;
        TextButton mSampleRemoveColorButton;
        TextButton mSampleDirectoryChainButton;
        TagContainer mSampleTagContainer;

        Rectangle<int> m_ThumbnailRect;
        Rectangle<int> m_TitleRect;

        std::unique_ptr<ColourSelector> mColourSelector;
        void onColourChanged(Colour newColour);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplePlayerComponent)
    };
}
