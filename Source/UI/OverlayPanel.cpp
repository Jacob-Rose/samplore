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
        // Setup title label (always shown, centered)
        mTitleLabel.setText("", dontSendNotification);
        mTitleLabel.setFont(Font(28.0f, Font::bold));
        mTitleLabel.setJustificationType(Justification::centred);
        addAndMakeVisible(mTitleLabel);
        
        // Setup back button (shown based on interface)
        mBackButton.setButtonText("< Back");
        mBackButton.onClick = [this]() {
            if (mOverlayContentInterface)
                mOverlayContentInterface->onOverlayBackButton();
        };
        addChildComponent(mBackButton);
        
        // Setup close button (always shown)
        mCloseButton.setButtonText("X");
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
        
        // Title row with back button (left) and close button (right)
        auto titleRow = contentBounds.removeFromTop(50);
        
        // Title always centered in full width
        mTitleLabel.setBounds(titleRow);
        
        // Back button on left (overlays title when visible)
        if (mBackButton.isVisible())
        {
            mBackButton.setBounds(titleRow.removeFromLeft(100).reduced(0, 5));
        }
        
        // Close button (X) in top right corner (overlays title)
        auto closeSize = 40;
        auto closeButtonBounds = titleRow.withLeft(titleRow.getRight() - closeSize)
                                         .withHeight(closeSize)
                                         .withY(titleRow.getY() + (titleRow.getHeight() - closeSize) / 2);
        mCloseButton.setBounds(closeButtonBounds);
        
        contentBounds.removeFromTop(20); // spacing after title
        
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
        mOverlayContentInterface = nullptr;
        
        if (content != nullptr)
        {
            mViewport.setViewedComponent(content, false);
            
            // Try to cast to interface
            mOverlayContentInterface = dynamic_cast<IOverlayPanelContent*>(content);
            
            if (mOverlayContentInterface)
            {
                // Set parent overlay so content can request refresh
                mOverlayContentInterface->setParentOverlay(this);
                
                // Update chrome from interface
                refreshChrome();
            }
            else
            {
                // No interface - hide title and back button
                mTitleLabel.setText("", dontSendNotification);
                mBackButton.setVisible(false);
            }
            
            // Trigger initial resize to set content width
            resized();
        }
    }
    
    void OverlayPanel::refreshChrome()
    {
        if (mOverlayContentInterface)
        {
            // Update title from interface
            mTitleLabel.setText(mOverlayContentInterface->getOverlayTitle(), dontSendNotification);
            
            // Update back button visibility from interface
            mBackButton.setVisible(mOverlayContentInterface->shouldShowBackButton());
            
            // Re-layout
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
        
        // Update button colors
        auto primaryColor = tm.getColorForRole(ThemeManager::ColorRole::AccentPrimary);
        auto textColor = tm.getColorForRole(ThemeManager::ColorRole::TextPrimary);
        
        mBackButton.setColour(TextButton::buttonColourId, primaryColor);
        mBackButton.setColour(TextButton::textColourOffId, textColor);
        mBackButton.setColour(TextButton::textColourOnId, textColor);
        
        mCloseButton.setColour(TextButton::buttonColourId, primaryColor);
        mCloseButton.setColour(TextButton::textColourOffId, textColor);
        mCloseButton.setColour(TextButton::textColourOnId, textColor);
    }
}
