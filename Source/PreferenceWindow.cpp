/*
  ==============================================================================

    PreferenceWindow.cpp
    Created: 13 Jun 2020 3:51:18pm
    Author:  Jake Rose

  ==============================================================================
*/

#include "PreferenceWindow.h"
#include "SamplifyLookAndFeel.h"
#include "SamplifyMainComponent.h"

using namespace samplify;

PreferenceWindow::PreferenceWindow() : DialogWindow("Preferences", AppValues::getInstance().MAIN_BACKGROUND_COLOR, true)
{
    setContentComponent(&mView);
    setSize(600, 800);
}

PreferenceWindow::View::View() : 
    mPrimaryColorButton("Set Primary Color"),
    mAccentColorButton("Set Accent Color"),
    mSampleMinSizeValue("Set Sample Tile Minimum Size"),
    mThumbnailLineCount("Set Thumbnail Draw Line Count")
{
    mThumbnailLineCount.setInputRestrictions(3, "0123456789");
    mThumbnailLineCount.setText(String(AppValues::getInstance().AUDIO_THUMBNAIL_LINE_COUNT));
    mSampleMinSizeValue.setInputRestrictions(3, "0123456789");
    mSampleMinSizeValue.setText(String(AppValues::getInstance().SAMPLE_TILE_MIN_WIDTH));
    mThumbnailLineCount.addListener(this);
    mSampleMinSizeValue.addListener(this);
    mPrimaryColorButton.setColour(TextButton::buttonColourId, AppValues::getInstance().MAIN_BACKGROUND_COLOR);
    mPrimaryColorButton.setColour(TextButton::textColourOffId, AppValues::getInstance().MAIN_BACKGROUND_COLOR.getPerceivedBrightness() > 0.5f ? Colours::black : Colours::white);
    mAccentColorButton.setColour(TextButton::buttonColourId, AppValues::getInstance().MAIN_FOREGROUND_COLOR);
    mAccentColorButton.setColour(TextButton::textColourOffId, AppValues::getInstance().MAIN_FOREGROUND_COLOR.getPerceivedBrightness() > 0.5f ? Colours::black : Colours::white);
    mPrimaryColorButton.addListener(this);
    mAccentColorButton.addListener(this);

    addAndMakeVisible(mPrimaryColorButton);
    addAndMakeVisible(mAccentColorButton);
    addAndMakeVisible(mSampleMinSizeValue);
    addAndMakeVisible(mThumbnailLineCount);
}

void PreferenceWindow::View::buttonClicked(Button* button)
{
    if (button->getName() == "Set Primary Color")
    {
        mEditingPrimaryColor = true;
        mColourSelector = std::make_unique<ColourSelector>();
        mColourSelector->setSize(200, 200);
        mColourSelector->setCurrentColour(AppValues::getInstance().MAIN_BACKGROUND_COLOR);
        mColourSelector->addChangeListener(this);
        CallOutBox::launchAsynchronously(std::move(mColourSelector), button->getScreenBounds(), nullptr);
    }
    else if (button->getName() == "Set Accent Color")
    {
        mEditingPrimaryColor = false;
        mColourSelector = std::make_unique<ColourSelector>();
        mColourSelector->setSize(200, 200);
        mColourSelector->setCurrentColour(AppValues::getInstance().MAIN_FOREGROUND_COLOR);
        mColourSelector->addChangeListener(this);
        CallOutBox::launchAsynchronously(std::move(mColourSelector), button->getScreenBounds(), nullptr);
    }
}

void PreferenceWindow::View::changeListenerCallback(ChangeBroadcaster* source)
{
    if (auto* selector = dynamic_cast<ColourSelector*>(source))
    {
        Colour newColour = selector->getCurrentColour();
        if (mEditingPrimaryColor)
        {
            mPrimaryColorButton.setColour(TextButton::buttonColourId, newColour);
            mPrimaryColorButton.setColour(TextButton::textColourOffId, newColour.getPerceivedBrightness() > 0.5f ? Colours::black : Colours::white);
            AppValues::getInstance().MAIN_BACKGROUND_COLOR = newColour;
            SamplifyMainComponent::setupLookAndFeel(getLookAndFeel());
        }
        else
        {
            mAccentColorButton.setColour(TextButton::buttonColourId, newColour);
            mAccentColorButton.setColour(TextButton::textColourOffId, newColour.getPerceivedBrightness() > 0.5f ? Colours::black : Colours::white);
            AppValues::getInstance().MAIN_FOREGROUND_COLOR = newColour;
        }
    }
}

void PreferenceWindow::View::textEditorTextChanged(TextEditor& editor)
{
    if (editor.getName() == "Set Sample Tile Minimum Size")
    {
        if (editor.getText().length() > 0)
        {
            AppValues::getInstance().SAMPLE_TILE_MIN_WIDTH = std::stoi(editor.getText().toStdString());
        }
        
    }
    else if (editor.getName() == "Set Thumbnail Draw Line Count")
    {
        if (editor.getText().length() > 0)
        {
            AppValues::getInstance().AUDIO_THUMBNAIL_LINE_COUNT = std::stoi(editor.getText().toStdString());
        }
    }
}

void PreferenceWindow::View::paint(Graphics& g)
{
    g.drawText(mThumbnailLineCount.getName(), Rectangle<float>(0, 200, getWidth() / 2, 30), Justification::centred);
    g.drawText(mSampleMinSizeValue.getName(), Rectangle<float>(0, 230, getWidth() / 2, 30), Justification::centred);
}

void PreferenceWindow::View::resized()
{
    mPrimaryColorButton.setBounds(0, 0, getWidth(), 100);
    mAccentColorButton.setBounds(0, 100, getWidth(), 100);
    mThumbnailLineCount.setBounds(getWidth() / 2, 200, getWidth() / 2, 30);
    mSampleMinSizeValue.setBounds(getWidth() / 2, 230, getWidth() / 2, 30);
}
