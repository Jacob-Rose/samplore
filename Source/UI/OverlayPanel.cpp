/*
  ==============================================================================

    OverlayPanel.cpp
    Created: 1 Jan 2026
    Author:  OpenCode

  ==============================================================================
*/

#include "OverlayPanel.h"

namespace samplore
{
    OverlayPanel::OverlayPanel()
        : mDeleteContentOnDestroy(true)
    {
        // Setup title label
        mTitleLabel.setText("", dontSendNotification);
        mTitleLabel.setFont(Font(28.0f, Font::bold));
        mTitleLabel.setJustificationType(Justification::centred);
        addAndMakeVisible(mTitleLabel);
        
        // Setup close button
        mCloseButton.setButtonText("Close");
        mCloseButton.onClick = [this]() {
            hide();
            if (onClose)
                onClose();
        };
        addAndMakeVisible(mCloseButton);
        
        // Setup viewport
        mViewport.setScrollBarsShown(true, false, true, false);
        addAndMakeVisible(mViewport);
        
        // Register with theme manager
        ThemeManager::getInstance().addListener(this);
        updateColors();
        
        // Start hidden
        setVisible(false);
    }
    
    OverlayPanel::~OverlayPanel()
    {
        ThemeManager::getInstance().removeListener(this);
        
        // Clean up content component if we own it
        if (mDeleteContentOnDestroy && mContentComponent != nullptr)
        {
            mViewport.setViewedComponent(nullptr, false);
            delete mContentComponent.getComponent();
        }
    }
    
    void OverlayPanel::paint(Graphics& g)
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
    
    void OverlayPanel::resized()
    {
        auto bounds = getLocalBounds();
        auto padding = 80;
        auto panelBounds = bounds.reduced(padding);
        
        auto contentBounds = panelBounds.reduced(30);
        
        // Title at the top
        mTitleLabel.setBounds(contentBounds.removeFromTop(50));
        
        contentBounds.removeFromTop(20); // spacing
        
        // Close button at the bottom
        auto closeButtonBounds = contentBounds.removeFromBottom(40);
        contentBounds.removeFromBottom(10); // spacing
        mCloseButton.setBounds(closeButtonBounds);
        
        // Viewport fills remaining space
        mViewport.setBounds(contentBounds);
        
        // Update content component width to match viewport
        if (mContentComponent != nullptr)
        {
            mContentComponent->setSize(contentBounds.getWidth(), mContentComponent->getHeight());
        }
    }
    
    void OverlayPanel::show()
    {
        setVisible(true);
        toFront(true);
    }
    
    void OverlayPanel::hide()
    {
        setVisible(false);
    }
    
    void OverlayPanel::setTitle(const String& title)
    {
        mTitleLabel.setText(title, dontSendNotification);
    }
    
    void OverlayPanel::setContentComponent(Component* content, bool deleteOnDestroy)
    {
        // Clean up old content if we own it
        if (mDeleteContentOnDestroy && mContentComponent != nullptr)
        {
            mViewport.setViewedComponent(nullptr, false);
            delete mContentComponent.getComponent();
        }
        
        mContentComponent = content;
        mDeleteContentOnDestroy = deleteOnDestroy;
        
        if (content != nullptr)
        {
            mViewport.setViewedComponent(content, false);
            
            // Trigger initial resize to set content width
            resized();
        }
    }
    
    void OverlayPanel::themeChanged(ThemeManager::Theme newTheme)
    {
        updateColors();
        repaint();
    }
    
    void OverlayPanel::colorChanged(ThemeManager::ColorRole role, Colour newColor)
    {
        updateColors();
        repaint();
    }
    
    void OverlayPanel::updateColors()
    {
        auto& tm = ThemeManager::getInstance();
        
        // Update title color
        mTitleLabel.setColour(Label::textColourId, tm.getColorForRole(ThemeManager::ColorRole::TextPrimary));
        
        // Update close button colors
        auto primaryColor = tm.getColorForRole(ThemeManager::ColorRole::AccentPrimary);
        auto textColor = tm.getColorForRole(ThemeManager::ColorRole::TextPrimary);
        
        mCloseButton.setColour(TextButton::buttonColourId, primaryColor);
        mCloseButton.setColour(TextButton::textColourOffId, textColor);
        mCloseButton.setColour(TextButton::textColourOnId, textColor);
    }
}
