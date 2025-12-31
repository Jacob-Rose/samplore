/*
  ==============================================================================

    ImportWizard.cpp
    Created: 31 Dec 2025
    Author:  OpenCode

  ==============================================================================
*/

#include "ImportWizard.h"

namespace samplore
{
    ImportWizard::ImportWizard()
    {
        // Setup title label
        mTitleLabel.setText("Import Wizard", dontSendNotification);
        mTitleLabel.setFont(Font(28.0f, Font::bold));
        mTitleLabel.setJustificationType(Justification::centred);
        addAndMakeVisible(mTitleLabel);
        
        // Setup Splice Import button
        mSpliceImportButton.setButtonText("Splice Import");
        mSpliceImportButton.addListener(this);
        addAndMakeVisible(mSpliceImportButton);
        
        // Setup General Import button
        mGeneralImportButton.setButtonText("General Import");
        mGeneralImportButton.addListener(this);
        addAndMakeVisible(mGeneralImportButton);
        
        // Setup Manual Import button
        mManualImportButton.setButtonText("Manual Import");
        mManualImportButton.addListener(this);
        addAndMakeVisible(mManualImportButton);
        
        // Setup close button
        mCloseButton.setButtonText("Close");
        mCloseButton.addListener(this);
        addAndMakeVisible(mCloseButton);
        
        // Register with theme manager
        ThemeManager::getInstance().addListener(this);
        updateColors();
        
        // Start hidden
        setVisible(false);
    }
    
    ImportWizard::~ImportWizard()
    {
        ThemeManager::getInstance().removeListener(this);
    }
    
    void ImportWizard::paint(Graphics& g)
    {
        // Draw semi-transparent dark overlay background
        g.fillAll(Colour(0, 0, 0).withAlpha(0.75f));
        
        // Calculate the content panel bounds (with more padding)
        auto bounds = getLocalBounds();
        auto padding = 80;
        auto panelBounds = bounds.reduced(padding);
        
        // Draw shadow effect for depth
        g.setColour(Colours::black.withAlpha(0.3f));
        g.fillRoundedRectangle(panelBounds.toFloat().translated(0, 4), 12.0f);
        
        // Draw the main panel background (brighter)
        auto bgColor = ThemeManager::getInstance().getColorForRole(ThemeManager::ColorRole::BackgroundSecondary);
        g.setColour(bgColor.brighter(0.1f));
        g.fillRoundedRectangle(panelBounds.toFloat(), 12.0f);
        
        // Draw subtle panel border
        g.setColour(ThemeManager::getInstance().getColorForRole(ThemeManager::ColorRole::Border).brighter(0.2f));
        g.drawRoundedRectangle(panelBounds.toFloat(), 12.0f, 1.0f);
    }
    
    void ImportWizard::resized()
    {
        auto bounds = getLocalBounds();
        auto padding = 80;
        auto panelBounds = bounds.reduced(padding);
        
        auto contentBounds = panelBounds.reduced(30);
        
        // Title at the top
        mTitleLabel.setBounds(contentBounds.removeFromTop(50));
        
        contentBounds.removeFromTop(20); // spacing
        
        // Calculate button dimensions
        auto buttonHeight = 60;
        auto buttonSpacing = 20;
        
        // Center the buttons vertically in remaining space
        auto totalButtonHeight = (buttonHeight * 3) + (buttonSpacing * 2);
        auto buttonStartY = (contentBounds.getHeight() - totalButtonHeight - 60) / 2;
        contentBounds.removeFromTop(buttonStartY);
        
        // Add buttons with spacing
        mSpliceImportButton.setBounds(contentBounds.removeFromTop(buttonHeight));
        contentBounds.removeFromTop(buttonSpacing);
        
        mGeneralImportButton.setBounds(contentBounds.removeFromTop(buttonHeight));
        contentBounds.removeFromTop(buttonSpacing);
        
        mManualImportButton.setBounds(contentBounds.removeFromTop(buttonHeight));
        
        // Close button at the bottom
        auto closeButtonBounds = panelBounds.reduced(20);
        closeButtonBounds.removeFromTop(closeButtonBounds.getHeight() - 40);
        mCloseButton.setBounds(closeButtonBounds);
    }
    
    void ImportWizard::buttonClicked(Button* button)
    {
        if (button == &mSpliceImportButton)
        {
            if (onSpliceImport)
                onSpliceImport();
        }
        else if (button == &mGeneralImportButton)
        {
            if (onGeneralImport)
                onGeneralImport();
        }
        else if (button == &mManualImportButton)
        {
            if (onManualImport)
                onManualImport();
        }
        else if (button == &mCloseButton)
        {
            hide();
            if (onClose)
                onClose();
        }
    }
    
    void ImportWizard::themeChanged(ThemeManager::Theme newTheme)
    {
        updateColors();
        repaint();
    }
    
    void ImportWizard::colorChanged(ThemeManager::ColorRole role, Colour newColor)
    {
        updateColors();
        repaint();
    }
    
    void ImportWizard::show()
    {
        setVisible(true);
        toFront(true);
    }
    
    void ImportWizard::hide()
    {
        setVisible(false);
    }
    
    void ImportWizard::updateColors()
    {
        auto& tm = ThemeManager::getInstance();
        
        // Update title color
        mTitleLabel.setColour(Label::textColourId, tm.getColorForRole(ThemeManager::ColorRole::TextPrimary));
        
        // Update button colors
        auto primaryColor = tm.getColorForRole(ThemeManager::ColorRole::AccentPrimary);
        auto textColor = tm.getColorForRole(ThemeManager::ColorRole::TextPrimary);
        
        for (auto* button : {&mSpliceImportButton, &mGeneralImportButton, &mManualImportButton, &mCloseButton})
        {
            button->setColour(TextButton::buttonColourId, primaryColor);
            button->setColour(TextButton::textColourOffId, textColor);
            button->setColour(TextButton::textColourOnId, textColor);
        }
    }
}
