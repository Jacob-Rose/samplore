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
        
        // Register with theme manager
        ThemeManager::getInstance().addListener(this);
        updateColors();
        
        // Set initial content height
        setSize(600, 300);
    }
    
    ImportWizard::~ImportWizard()
    {
        ThemeManager::getInstance().removeListener(this);
    }
    
    void ImportWizard::paint(Graphics& g)
    {
        // Content background is handled by OverlayPanel
    }
    
    void ImportWizard::resized()
    {
        auto bounds = getLocalBounds();
        const int buttonHeight = 60;
        const int buttonSpacing = 20;
        
        // Center the buttons vertically in available space
        auto totalButtonHeight = (buttonHeight * 3) + (buttonSpacing * 2);
        auto buttonStartY = (bounds.getHeight() - totalButtonHeight) / 2;
        bounds.removeFromTop(buttonStartY);
        
        // Add buttons with spacing
        mSpliceImportButton.setBounds(bounds.removeFromTop(buttonHeight));
        bounds.removeFromTop(buttonSpacing);
        
        mGeneralImportButton.setBounds(bounds.removeFromTop(buttonHeight));
        bounds.removeFromTop(buttonSpacing);
        
        mManualImportButton.setBounds(bounds.removeFromTop(buttonHeight));
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
    
    void ImportWizard::updateColors()
    {
        auto& tm = ThemeManager::getInstance();
        
        // Update button colors
        auto primaryColor = tm.getColorForRole(ThemeManager::ColorRole::AccentPrimary);
        auto textColor = tm.getColorForRole(ThemeManager::ColorRole::TextPrimary);
        
        for (auto* button : {&mSpliceImportButton, &mGeneralImportButton, &mManualImportButton})
        {
            button->setColour(TextButton::buttonColourId, primaryColor);
            button->setColour(TextButton::textColourOffId, textColor);
            button->setColour(TextButton::textColourOnId, textColor);
        }
    }
}
