/*
  ==============================================================================

    ImportWizard.cpp
    Created: 31 Dec 2025
    Author:  OpenCode

  ==============================================================================
*/

#include "ImportWizard.h"
#include "UI/OverlayPanel.h"

namespace samplore
{
    // Define view titles in a centralized map
    const std::map<ImportWizard::View, String> ImportWizard::sViewTitles = {
        {View::MainMenu, "Import Wizard"},
        {View::SpliceImport, "Splice Import"},
        {View::GeneralImport, "General Import"},
        {View::ManualImport, "Manual Import"}
    };
    ImportWizard::ImportWizard()
        : mGeneralImportView("General Import")
        , mManualImportView("Manual Import")
    {
        // Setup main menu buttons
        mSpliceImportButton.setButtonText("Splice Import");
        mSpliceImportButton.addListener(this);
        addAndMakeVisible(mSpliceImportButton);
        
        mGeneralImportButton.setButtonText("General Import [In Progress]");
        mGeneralImportButton.addListener(this);
        mGeneralImportButton.setEnabled(false);
        addAndMakeVisible(mGeneralImportButton);
        
        mManualImportButton.setButtonText("Manual Import [In Progress]");
        mManualImportButton.addListener(this);
        mManualImportButton.setEnabled(false);
        addAndMakeVisible(mManualImportButton);
        
        // Setup import views (hidden initially)
        addChildComponent(mSpliceImportView);
        addChildComponent(mGeneralImportView);
        addChildComponent(mManualImportView);
        
        // Setup splice import completion callback
        mSpliceImportView.onImportComplete = [this](bool success, int samplesImported) {
            // Auto-return to main menu after import completes
            showMainMenu();
        };
        
        // Register with theme manager
        ThemeManager::getInstance().addListener(this);
        updateColors();
        
        // Set initial content height
        setSize(600, 300);
        showMainMenu();
    }
    
    ImportWizard::~ImportWizard()
    {
        ThemeManager::getInstance().removeListener(this);
    }
    
    void ImportWizard::paint(Graphics& g)
    {
        // Content background is handled by OverlayPanel
        auto& tm = ThemeManager::getInstance();
        g.fillAll(tm.getColorForRole(ThemeManager::ColorRole::Background));
    }
    
    void ImportWizard::resized()
    {
        auto bounds = getLocalBounds();
        
        if (mCurrentView == View::MainMenu)
        {
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
        else
        {
            // Content view fills remaining space
            switch (mCurrentView)
            {
                case View::SpliceImport:
                    mSpliceImportView.setBounds(bounds);
                    break;
                case View::GeneralImport:
                    mGeneralImportView.setBounds(bounds);
                    break;
                case View::ManualImport:
                    mManualImportView.setBounds(bounds);
                    break;
                default:
                    break;
            }
        }
    }
    
    void ImportWizard::buttonClicked(Button* button)
    {
        if (button == &mSpliceImportButton)
        {
            showView(View::SpliceImport);
        }
        else if (button == &mGeneralImportButton)
        {
            showView(View::GeneralImport);
        }
        else if (button == &mManualImportButton)
        {
            showView(View::ManualImport);
        }
    }
    
    void ImportWizard::showMainMenu()
    {
        showView(View::MainMenu);
    }
    
    String ImportWizard::getTitleForView(View view) const
    {
        auto it = sViewTitles.find(view);
        if (it != sViewTitles.end())
            return it->second;
        return "Import Wizard";
    }
    
    void ImportWizard::showView(View view)
    {
        mCurrentView = view;
        
        // Hide all views
        mSpliceImportButton.setVisible(false);
        mGeneralImportButton.setVisible(false);
        mManualImportButton.setVisible(false);
        mSpliceImportView.setVisible(false);
        mGeneralImportView.setVisible(false);
        mManualImportView.setVisible(false);
        
        // Show current view
        if (view == View::MainMenu)
        {
            mSpliceImportButton.setVisible(true);
            mGeneralImportButton.setVisible(true);
            mManualImportButton.setVisible(true);
            setSize(getWidth(), 300);
        }
        else
        {
            switch (view)
            {
                case View::SpliceImport:
                    mSpliceImportView.setVisible(true);
                    mSpliceImportView.show(); // Initialize the view
                    setSize(getWidth(), 550);
                    break;
                case View::GeneralImport:
                    mGeneralImportView.setVisible(true);
                    setSize(getWidth(), 400);
                    break;
                case View::ManualImport:
                    mManualImportView.setVisible(true);
                    setSize(getWidth(), 400);
                    break;
                default:
                    break;
            }
        }
        
        resized();
        repaint();
        
        // Request parent overlay to refresh chrome (title and back button)
        if (mParentOverlay)
            mParentOverlay->refreshChrome();
        
        // Notify parent (OverlayPanel's viewport) to update
        if (auto* parent = getParentComponent())
            parent->resized();
    }
    
    //==================================================================
    // IOverlayPanelContent interface implementation
    String ImportWizard::getOverlayTitle() const
    {
        return getTitleForView(mCurrentView);
    }
    
    bool ImportWizard::shouldShowBackButton() const
    {
        return mCurrentView != View::MainMenu;
    }
    
    void ImportWizard::onOverlayBackButton()
    {
        showMainMenu();
    }
    
    void ImportWizard::setParentOverlay(OverlayPanel* parent)
    {
        mParentOverlay = parent;
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
        auto disabledColor = tm.getColorForRole(ThemeManager::ColorRole::TextSecondary);
        
        for (auto* button : {&mSpliceImportButton, &mGeneralImportButton, &mManualImportButton})
        {
            button->setColour(TextButton::buttonColourId, primaryColor);
            button->setColour(TextButton::textColourOffId, textColor);
            button->setColour(TextButton::textColourOnId, textColor);
        }
        
        // Disabled buttons get dimmed appearance
        mGeneralImportButton.setColour(TextButton::buttonColourId, primaryColor.withAlpha(0.3f));
        mGeneralImportButton.setColour(TextButton::textColourOffId, disabledColor);
        
        mManualImportButton.setColour(TextButton::buttonColourId, primaryColor.withAlpha(0.3f));
        mManualImportButton.setColour(TextButton::textColourOffId, disabledColor);
    }
}
