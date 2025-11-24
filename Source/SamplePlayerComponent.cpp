/*
  ==============================================================================

    SamplePlayerComponent.cpp
    Created: 4 Apr 2020 7:43:10pm
    Author:  jacob

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "SamplePlayerComponent.h"
#include "SamplifyProperties.h"
#include "SamplifyLookAndFeel.h"

using namespace samplify;

//==============================================================================
SamplePlayerComponent::SamplePlayerComponent() : mSampleTagContainer(false)
{

    addAndMakeVisible(mSampleInfoEditor);
    addAndMakeVisible(mSampleColorSelectorButton);
    addAndMakeVisible(mSampleRemoveColorButton);
    addAndMakeVisible(mSampleDirectoryChainButton); //shows the parent folders of sample
    addAndMakeVisible(mSampleTagContainer);

    mSampleColorSelectorButton.setName("SetSampleColor");
    mSampleColorSelectorButton.addListener(this);

    mSampleRemoveColorButton.setName("RemoveSampleColor");
    mSampleColorSelectorButton.setColour(TextButton::textColourOffId, getLookAndFeel().findColour(TextEditor::backgroundColourId).getPerceivedBrightness() > 0.5f ? Colours::black : Colours::white);
    mSampleRemoveColorButton.addListener(this);

    mSampleDirectoryChainButton.setName("ParentFolders");
    mSampleDirectoryChainButton.setButtonText("Parent Folders");
    mSampleDirectoryChainButton.addListener(this);

    mSampleInfoEditor.addListener(this);
    mSampleInfoEditor.setTextToShowWhenEmpty("Add Comment about Sample", getLookAndFeel().findColour(TextEditor::textColourId).darker(0.4f));
    mSampleInfoEditor.setMultiLine(true, true);
    //addMouseListener(this, false);
}

SamplePlayerComponent::~SamplePlayerComponent()
{

}

void SamplePlayerComponent::changeListenerCallback(ChangeBroadcaster* source)
{
    // Handle colour selector changes
    if (auto* selector = dynamic_cast<ColourSelector*>(source))
    {
        onColourChanged(selector->getCurrentColour());
        return;
    }

    Sample::Reference samp = getCurrentSample();
    if (!samp.isNull())
    {
        mSampleTagContainer.setTags(samp.getTags());
        if (samp.getColor().getFloatAlpha() > 0.0f)
        {
            mSampleColorSelectorButton.setButtonText("Set Sample Color");
            mSampleColorSelectorButton.setColour(TextButton::buttonColourId, getLookAndFeel().findColour(backgroundColourId));
        }
        else
        {
            mSampleColorSelectorButton.setButtonText("Change Sample Color");
            mSampleColorSelectorButton.setColour(TextButton::buttonColourId, samp.getColor());
        }
    }
    else
    {
        mSampleTagContainer.setTags(StringArray());
    }
    resized();
    repaint();
}

void SamplePlayerComponent::textEditorTextChanged(TextEditor& e)
{
    getCurrentSample().setInfoText(e.getText());
}

void SamplePlayerComponent::onColourChanged(Colour newColour)
{
    Sample::Reference samp = getCurrentSample();
    mSampleColorSelectorButton.setColour(TextButton::buttonColourId, newColour);
    mSampleColorSelectorButton.setColour(TextButton::textColourOffId, newColour.getPerceivedBrightness() > 0.5f ? Colours::black : Colours::white);
    samp.setColor(newColour);
    resized();
}

void SamplePlayerComponent::buttonClicked(Button* b)
{
    Sample::Reference samp = getCurrentSample();
    if (b->getName() == "SetSampleColor")
    {
        mColourSelector = std::make_unique<ColourSelector>();
        mColourSelector->setSize(200, 200);
        mColourSelector->setCurrentColour(samp.getColor().withAlpha(1.0f));
        mColourSelector->addChangeListener(this);
        auto* selector = mColourSelector.get();
        CallOutBox::launchAsynchronously(std::move(mColourSelector), b->getScreenBounds(), nullptr);
    }
    else if (b->getName() == "RemoveSampleColor")
    {
        samp.setColor(Colours::transparentWhite);
        resized();
    }
    else if (b->getName() == "ParentFolders")
    {
        PopupMenu dirMenu;
        StringArray parentFolders = samp.getRelativeParentFolders();
        for (int i = 0; i < parentFolders.size(); i++)
        {
            dirMenu.addItem(i + 1, parentFolders[i]);
        }
        dirMenu.showMenuAsync(PopupMenu::Options());
    }
}

void SamplePlayerComponent::paint (Graphics& g)
{
    Sample::Reference samp = getCurrentSample();
    if (!samp.isNull())
    {
        mSampleInfoEditor.setText(samp.getInfoText());
        g.drawText(samp.getFile().getFileName(), m_TitleRect, Justification::left, true);

        //draw thumbnail
        if (samp.getThumbnail() != nullptr)
        {
            g.setColour(getLookAndFeel().findColour(waveformColourId));
            if (SampleAudioThumbnail* thumbnail = dynamic_cast<SampleAudioThumbnail*>(samp.getThumbnail().get()))
            {
                samp.getThumbnail()->drawChannels(g, m_ThumbnailRect, 0, samp.getLength(), 1.0f, 160);
            }
            else
            {
                samp.getThumbnail()->drawChannels(g, m_ThumbnailRect, 0, samp.getLength(), 1.0f);
            }
        }
        //Draw Audio Line if playing
        std::shared_ptr<AudioPlayer> auxPlayer = SamplifyProperties::getInstance()->getAudioPlayer();
        if (auxPlayer->getSampleReference() == samp)
        {
            float startT = auxPlayer->getStartCueRelative();
            float currentT = auxPlayer->getRelativeTime();
            float startX = m_ThumbnailRect.getTopLeft().x + ((m_ThumbnailRect.getTopRight().x - m_ThumbnailRect.getTopLeft().x) * startT);
            float currentX = m_ThumbnailRect.getTopLeft().x + ((m_ThumbnailRect.getTopRight().x - m_ThumbnailRect.getTopLeft().x) * currentT);
            float y1 = m_ThumbnailRect.getTopLeft().y;
            float y2 = m_ThumbnailRect.getBottomLeft().y;
            g.setColour(Colours::black);
            g.drawLine(startX, y1, startX, y2, 1.0f);
            if (auxPlayer->getState() == AudioPlayer::TransportState::Playing)
            {
                g.setColour(Colours::red);
                g.drawLine(currentX, y1, currentX, y2, 1.0f);
                repaint();
            }
        }
    }
}
const int padding = 10;
void SamplePlayerComponent::resized()
{
    Sample::Reference samp = getCurrentSample();
    if (!samp.isNull())
    {
        m_ThumbnailRect = Rectangle<int>(padding, 0, getWidth() - (padding * 2), getHeight() / 2);
        float cHeight = m_ThumbnailRect.getHeight();
        m_TitleRect = Rectangle<int>(0, m_ThumbnailRect.getHeight(), (getWidth()/3)*2, SAMPLE_TILE_TITLE_FONT.getHeight() + 2);
        mSampleDirectoryChainButton.setBounds(m_TitleRect.getWidth(), cHeight, (getWidth() / 3), m_TitleRect.getHeight());
        cHeight += m_TitleRect.getHeight();
        mSampleInfoEditor.setBounds(0, cHeight, getWidth() / 4, getHeight()- cHeight);
        float cWidth = mSampleInfoEditor.getWidth();
        if (samp.getColor().getFloatAlpha() > 0.0f)
        {
            float regionHeight = (getHeight() - cHeight);
            mSampleColorSelectorButton.setBounds(cWidth, cHeight, 100, regionHeight /2);
            mSampleRemoveColorButton.setBounds(cWidth, cHeight + (regionHeight / 2), 100, regionHeight / 2);
        }
        else
        {
            float regionHeight = (getHeight() - cHeight);
            mSampleColorSelectorButton.setBounds(cWidth, cHeight, 100, regionHeight);
            mSampleRemoveColorButton.setBounds(cWidth, cHeight, 100, 0);
        }
        
        cWidth += 100;
        mSampleTagContainer.setBounds(cWidth, cHeight, getWidth() - cWidth, getHeight() - cHeight);
    }
    else
    {
        m_ThumbnailRect = Rectangle<int>(0, 0, 0, 0);
        mSampleInfoEditor.setBounds(0, 0, 0, 0);
        mSampleColorSelectorButton.setBounds(0, 0, 0, 0);
        mSampleRemoveColorButton.setBounds(0, 0, 0, 0);
        mSampleDirectoryChainButton.setBounds(0, 0, 0, 0); 
        mSampleTagContainer.setBounds(0, 0, 0, 0);
    }
    
}

void SamplePlayerComponent::mouseDown(const MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        if (m_ThumbnailRect.contains(e.getMouseDownPosition()))
        {
            float rectWidth = m_ThumbnailRect.getWidth();
            float mouseDownX = e.getMouseDownX();
            SamplifyProperties::getInstance()->getAudioPlayer()->playSample(mouseDownX / rectWidth);
        }
    }
}



Sample::Reference SamplePlayerComponent::getCurrentSample()
{
    return SamplifyProperties::getInstance()->getAudioPlayer()->getSampleReference();
}
