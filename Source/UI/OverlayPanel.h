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

namespace samplore
{
    /// Reusable overlay panel with semi-transparent background, centered content area,
    /// scrollable viewport, title label, and close button
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
        
        /// Sets the title text
        void setTitle(const String& title);
        
        /// Sets the content component that will be shown in the scrollable viewport
        /// The panel takes ownership of the component
        void setContentComponent(Component* content, bool deleteOnDestroy = true);
        
        /// Returns the viewport for direct access if needed
        Viewport& getViewport() { return mViewport; }
        
        /// Returns the close button for custom configuration if needed
        TextButton& getCloseButton() { return mCloseButton; }
        
        /// Callback when close is requested
        std::function<void()> onClose;
        
    private:
        Label mTitleLabel;
        TextButton mCloseButton;
        Viewport mViewport;
        Component::SafePointer<Component> mContentComponent;
        bool mDeleteContentOnDestroy;
        
        void updateColors();
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OverlayPanel)
    };
}
#endif
