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
#include "ThemeManager.h"

using namespace samplify;

//==============================================================================
SamplePlayerComponent::SamplePlayerComponent() : mSampleTagContainer(false)
{
    auto& theme = ThemeManager::getInstance();

    addAndMakeVisible(mSampleInfoEditor);
    addAndMakeVisible(mSampleColorSelectorButton);
    addAndMakeVisible(mSampleRemoveColorButton);
    addAndMakeVisible(mSampleDirectoryChainButton);
    addAndMakeVisible(mSampleTagContainer);

    // Color selector button
    mSampleColorSelectorButton.setName("SetSampleColor");
    mSampleColorSelectorButton.addListener(this);

    // Remove color button
    mSampleRemoveColorButton.setName("RemoveSampleColor");
    mSampleRemoveColorButton.setButtonText("Remove Color");
    mSampleRemoveColorButton.addListener(this);

    // Parent folders button
    mSampleDirectoryChainButton.setName("ParentFolders");
    mSampleDirectoryChainButton.setButtonText("Parent Folders");
    mSampleDirectoryChainButton.addListener(this);

    // Info editor
    mSampleInfoEditor.addListener(this);
    mSampleInfoEditor.setTextToShowWhenEmpty("Add notes about this sample...",
        theme.get(ThemeManager::ColorRole::TextSecondary));
    mSampleInfoEditor.setMultiLine(true, true);
    mSampleInfoEditor.setReturnKeyStartsNewLine(true);

    updateThemeColors();
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
    auto& theme = ThemeManager::getInstance();
    Sample::Reference samp = getCurrentSample();

    // Draw background
    g.fillAll(theme.get(ThemeManager::ColorRole::BackgroundSecondary));

    if (!samp.isNull())
    {
        mSampleInfoEditor.setText(samp.getInfoText());

        // Draw title with modern typography
        g.setColour(theme.get(ThemeManager::ColorRole::TextPrimary));
        g.setFont(Font(20.0f, Font::bold));
        g.drawText(samp.getFile().getFileName(), m_TitleRect, Justification::left, true);

        // Draw waveform with modern styling
        if (samp.getThumbnail() != nullptr)
        {
            // Draw waveform background
            g.setColour(theme.get(ThemeManager::ColorRole::BackgroundTertiary));
            g.fillRoundedRectangle(m_ThumbnailRect.toFloat(), 8.0f);

            // Draw waveform
            g.setColour(theme.get(ThemeManager::ColorRole::WaveformPrimary));
            if (SampleAudioThumbnail* thumbnail = dynamic_cast<SampleAudioThumbnail*>(samp.getThumbnail().get()))
            {
                samp.getThumbnail()->drawChannels(g, m_ThumbnailRect, 0, samp.getLength(), 1.0f, AppValues::getInstance().AUDIO_THUMBNAIL_LINE_COUNT_PLAYER);
            }
            else
            {
                samp.getThumbnail()->drawChannels(g, m_ThumbnailRect, 0, samp.getLength(), 1.0f);
            }

            // Draw subtle border around waveform
            g.setColour(theme.get(ThemeManager::ColorRole::Border));
            g.drawRoundedRectangle(m_ThumbnailRect.toFloat(), 8.0f, 1.0f);
        }

        // Draw playback position indicators
        std::shared_ptr<AudioPlayer> auxPlayer = SamplifyProperties::getInstance()->getAudioPlayer();
        if (auxPlayer->getSampleReference() == samp)
        {
            float startT = auxPlayer->getStartCueRelative();
            float currentT = auxPlayer->getRelativeTime();
            float startX = m_ThumbnailRect.getX() + (m_ThumbnailRect.getWidth() * startT);
            float currentX = m_ThumbnailRect.getX() + (m_ThumbnailRect.getWidth() * currentT);
            float y1 = m_ThumbnailRect.getY();
            float y2 = m_ThumbnailRect.getBottom();

            // Draw start position with subtle color
            g.setColour(theme.get(ThemeManager::ColorRole::TextSecondary).withAlpha(0.5f));
            g.drawLine(startX, y1, startX, y2, 1.5f);

            // Draw current position with accent color
            if (auxPlayer->getState() == AudioPlayer::TransportState::Playing)
            {
                g.setColour(theme.get(ThemeManager::ColorRole::AccentSecondary));
                g.drawLine(currentX, y1, currentX, y2, 2.0f);
                repaint();
            }
        }
    }
}
void SamplePlayerComponent::resized()
{
    const int padding = 16;
    const int itemSpacing = 8;
    const int buttonWidth = 120;
    const int titleHeight = 32;

    Sample::Reference samp = getCurrentSample();
    if (!samp.isNull())
    {
        int y = padding;

        // Waveform area (top half with padding)
        m_ThumbnailRect = Rectangle<int>(padding, y, getWidth() - (padding * 2),
                                         (getHeight() / 2) - padding);
        y = m_ThumbnailRect.getBottom() + padding;

        // Title and parent folders button row
        m_TitleRect = Rectangle<int>(padding, y, getWidth() - buttonWidth - (padding * 3),
                                      titleHeight);
        mSampleDirectoryChainButton.setBounds(getWidth() - buttonWidth - padding, y,
                                               buttonWidth, titleHeight);
        y += titleHeight + itemSpacing;

        // Bottom section: info editor, color buttons, and tags
        int remainingHeight = getHeight() - y - padding;
        int infoWidth = getWidth() / 3;

        mSampleInfoEditor.setBounds(padding, y, infoWidth, remainingHeight);

        int colorButtonX = padding + infoWidth + itemSpacing;
        if (samp.getColor().getFloatAlpha() > 0.0f)
        {
            int buttonHeight = remainingHeight / 2 - (itemSpacing / 2);
            mSampleColorSelectorButton.setBounds(colorButtonX, y, buttonWidth, buttonHeight);
            mSampleRemoveColorButton.setBounds(colorButtonX, y + buttonHeight + itemSpacing,
                                                buttonWidth, buttonHeight);
        }
        else
        {
            mSampleColorSelectorButton.setBounds(colorButtonX, y, buttonWidth, remainingHeight);
            mSampleRemoveColorButton.setBounds(colorButtonX, y, buttonWidth, 0);
        }

        // Tags area
        int tagsX = colorButtonX + buttonWidth + itemSpacing;
        mSampleTagContainer.setBounds(tagsX, y, getWidth() - tagsX - padding, remainingHeight);
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

void SamplePlayerComponent::updateThemeColors()
{
    auto& theme = ThemeManager::getInstance();

    // Update text editor colors
    mSampleInfoEditor.setColour(TextEditor::backgroundColourId,
        theme.get(ThemeManager::ColorRole::Surface));
    mSampleInfoEditor.setColour(TextEditor::textColourId,
        theme.get(ThemeManager::ColorRole::TextPrimary));
    mSampleInfoEditor.setColour(TextEditor::outlineColourId,
        theme.get(ThemeManager::ColorRole::Border));
    mSampleInfoEditor.setColour(TextEditor::focusedOutlineColourId,
        theme.get(ThemeManager::ColorRole::BorderFocus));

    // Update button colors
    mSampleDirectoryChainButton.setColour(TextButton::buttonColourId,
        theme.get(ThemeManager::ColorRole::Surface));
    mSampleDirectoryChainButton.setColour(TextButton::textColourOffId,
        theme.get(ThemeManager::ColorRole::TextPrimary));

    mSampleRemoveColorButton.setColour(TextButton::buttonColourId,
        theme.get(ThemeManager::ColorRole::Surface));
    mSampleRemoveColorButton.setColour(TextButton::textColourOffId,
        theme.get(ThemeManager::ColorRole::TextPrimary));

    repaint();
}
