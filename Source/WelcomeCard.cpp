/*
  ==============================================================================

    WelcomeCard.cpp
    Author:  Jake Rose

  ==============================================================================
*/

#include "WelcomeCard.h"
#include "UI/OverlayPanel.h"
#include "SamplifyLookAndFeel.h"

using namespace samplore;

WelcomeCard::WelcomeCard()
{
    setupPages();

    mNextButton.setButtonText("Next");
    mNextButton.onClick = [this]() { nextPage(); };
    addAndMakeVisible(mNextButton);

    mPrevButton.setButtonText("Back");
    mPrevButton.onClick = [this]() { prevPage(); };
    addAndMakeVisible(mPrevButton);

    mGetStartedButton.setButtonText("Get Started");
    mGetStartedButton.onClick = [this]() { finish(); };
    addAndMakeVisible(mGetStartedButton);

    ThemeManager::getInstance().addListener(this);
    updateButtonVisibility();

    setSize(500, 400);
}

WelcomeCard::~WelcomeCard()
{
    ThemeManager::getInstance().removeListener(this);
}

void WelcomeCard::setupPages()
{
    mPages.clear();

    mPages.push_back({
        "Welcome to Samplore!",
        "Samplore is a fast, intuitive sample library manager designed for music producers.\n\n"
        "This quick guide will walk you through the main features to help you get started."
    });

    mPages.push_back({
        "Sample Preview",
        "Left-click on any sample tile to preview it from the beginning.\n\n"
        "Right-click on a sample tile to preview from that specific point in the waveform - "
        "great for quickly jumping to the part you want to hear.\n\n"
        "The playback indicator shows the current position as it plays."
    });

    mPages.push_back({
        "Tags",
        "Tags help you organize and filter your samples.\n\n"
        "Right-click on a sample tile to add or remove tags. "
        "Use the Tags panel on the right to filter your library - "
        "tags are color-coded for easy visual identification.\n\n"
        "Group related tags into Tag Collections (like 'Drums', 'Synths', 'FX') "
        "to keep things tidy as your tag library grows."
    });

    mPages.push_back({
        "Cue System",
        "Bind samples to keyboard keys for instant triggering!\n\n"
        "Press Ctrl+K (or View -> Cue Bindings) to open the cue window. "
        "With a sample selected, press any key to bind it.\n\n"
        "Cues let you audition multiple samples rapidly or build quick "
        "performance setups. Bindings are saved with your session."
    });

    mPages.push_back({
        "Getting Started",
        "To import your sample library:\n\n"
        "1. Go to File -> Import Wizard\n"
        "2. Choose your import method\n"
        "3. Select your sample directories\n\n"
        "Samplore will scan your folders and build your library.\n\n"
        "Happy sampling!"
    });
}

void WelcomeCard::paint(Graphics& g)
{
    auto& theme = ThemeManager::getInstance();
    using CR = ThemeManager::ColorRole;

    auto bounds = getLocalBounds().reduced(20);

    // Draw page title
    g.setColour(theme.getColorForRole(CR::TextPrimary));
    g.setFont(FontOptions(24.0f).withStyle("Bold"));

    auto titleBounds = bounds.removeFromTop(40);
    g.drawText(mPages[mCurrentPage].title, titleBounds, Justification::centred);

    bounds.removeFromTop(20); // spacing

    // Draw page indicator dots
    auto dotsArea = bounds.removeFromTop(20);
    int dotSize = 8;
    int dotSpacing = 16;
    int totalDotsWidth = (int)mPages.size() * dotSpacing;
    int dotsStartX = (dotsArea.getWidth() - totalDotsWidth) / 2 + dotsArea.getX();

    for (int i = 0; i < (int)mPages.size(); i++)
    {
        auto dotBounds = Rectangle<float>(
            (float)(dotsStartX + i * dotSpacing),
            (float)(dotsArea.getCentreY() - dotSize / 2),
            (float)dotSize,
            (float)dotSize
        );

        if (i == mCurrentPage)
        {
            g.setColour(theme.getColorForRole(CR::AccentPrimary));
            g.fillEllipse(dotBounds);
        }
        else
        {
            g.setColour(theme.getColorForRole(CR::TextSecondary).withAlpha(0.5f));
            g.fillEllipse(dotBounds);
        }
    }

    bounds.removeFromTop(20); // spacing

    // Draw content
    g.setColour(theme.getColorForRole(CR::TextPrimary));
    g.setFont(FontOptions(16.0f));

    auto contentBounds = bounds.removeFromTop(bounds.getHeight() - 60);
    g.drawFittedText(mPages[mCurrentPage].content, contentBounds,
                     Justification::topLeft, 20);
}

void WelcomeCard::resized()
{
    auto bounds = getLocalBounds().reduced(20);

    // Reserve space for buttons at the bottom
    auto buttonArea = bounds.removeFromBottom(40);

    int buttonWidth = 100;
    int buttonHeight = 32;
    int buttonSpacing = 10;

    // Center the buttons
    int totalButtonWidth = buttonWidth * 2 + buttonSpacing;
    int startX = (buttonArea.getWidth() - totalButtonWidth) / 2 + buttonArea.getX();

    mPrevButton.setBounds(startX, buttonArea.getCentreY() - buttonHeight / 2,
                          buttonWidth, buttonHeight);
    mNextButton.setBounds(startX + buttonWidth + buttonSpacing,
                          buttonArea.getCentreY() - buttonHeight / 2,
                          buttonWidth, buttonHeight);

    // Get Started button centered alone
    mGetStartedButton.setBounds(buttonArea.getCentreX() - buttonWidth / 2,
                                buttonArea.getCentreY() - buttonHeight / 2,
                                buttonWidth, buttonHeight);
}

String WelcomeCard::getOverlayTitle() const
{
    return "Welcome";
}

bool WelcomeCard::shouldShowBackButton() const
{
    return false;
}

void WelcomeCard::onOverlayBackButton()
{
    // Not used
}

void WelcomeCard::setParentOverlay(OverlayPanel* parent)
{
    mParentOverlay = parent;
}

void WelcomeCard::themeChanged(ThemeManager::Theme newTheme)
{
    repaint();
}

void WelcomeCard::colorChanged(ThemeManager::ColorRole role, Colour newColor)
{
    repaint();
}

void WelcomeCard::resetToStart()
{
    mCurrentPage = 0;
    updateButtonVisibility();
    repaint();
}

void WelcomeCard::updateButtonVisibility()
{
    bool isFirstPage = (mCurrentPage == 0);
    bool isLastPage = (mCurrentPage == (int)mPages.size() - 1);

    mPrevButton.setVisible(!isFirstPage && !isLastPage);
    mNextButton.setVisible(!isLastPage);
    mGetStartedButton.setVisible(isLastPage);

    // On first page, show only Next
    if (isFirstPage)
    {
        mPrevButton.setVisible(false);
    }
}

void WelcomeCard::nextPage()
{
    if (mCurrentPage < (int)mPages.size() - 1)
    {
        mCurrentPage++;
        updateButtonVisibility();
        repaint();
    }
}

void WelcomeCard::prevPage()
{
    if (mCurrentPage > 0)
    {
        mCurrentPage--;
        updateButtonVisibility();
        repaint();
    }
}

void WelcomeCard::finish()
{
    // Mark as seen and close
    AppValues::getInstance().HAS_SEEN_WELCOME = true;

    if (mParentOverlay != nullptr)
    {
        mParentOverlay->hide();
    }
}
