/*
  ==============================================================================

    OverlayPanel.h
    Created: 1 Jan 2026
    Author:  OpenCode

  ==============================================================================
*/

#ifndef OVERLAYPANEL_H
#define OVERLAYPANEL_H
#include "JuceHeader.h"
#include "../ThemeManager.h"
#include "IOverlayPanelContent.h"

namespace samplore
{
    /// Reusable overlay panel with semi-transparent background, title, and centered content
    /// Queries content via IOverlayPanelContent interface for title and back button state
    class OverlayPanel : public Component, public ThemeManager::Listener
    {
    public:
        OverlayPanel();
        ~OverlayPanel() override;
        
        void paint(Graphics& g) override;
        void resized() override;
        
        //==================================================================
        // ThemeManager::Listener interface
        void themeChanged(ThemeManager::Theme newTheme) override;
        void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;
        
        /// Shows the overlay panel
        void show();
        
        /// Hides the overlay panel
        void hide();
        
        /// Sets the content component that will be shown in the scrollable viewport
        void setContentComponent(Component* content, bool deleteOnDestroy = true);
        
        /// Refreshes the title and back button from content interface
        void refreshChrome();
        
        /// Returns the viewport for direct access if needed
        Viewport& getViewport() { return mViewport; }
        
        /// Callback when close is requested
        std::function<void()> onClose;
        
    private:
        Label mTitleLabel;
        TextButton mBackButton;
        TextButton mCloseButton;
        Viewport mViewport;
        Component::SafePointer<Component> mContentComponent;
        IOverlayPanelContent* mOverlayContentInterface = nullptr;
        bool mDeleteContentOnDestroy;
        
        void updateColors();
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OverlayPanel)
    };
}
#endif
