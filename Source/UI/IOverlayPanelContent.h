/*
  ==============================================================================

    IOverlayPanelContent.h
    Created: 1 Jan 2026
    Author:  OpenCode
    
    Interface for content that can be displayed in an OverlayPanel.
    Content provides title, back button visibility, and can request
    parent overlay to refresh its chrome.

  ==============================================================================
*/

#ifndef IOVERLAYPANELCONTENT_H
#define IOVERLAYPANELCONTENT_H

#include "JuceHeader.h"

namespace samplore
{
    // Forward declaration
    class OverlayPanel;
    
    /// Interface for content components that are displayed in an OverlayPanel
    class IOverlayPanelContent
    {
    public:
        virtual ~IOverlayPanelContent() = default;
        
        /// Returns the title to display in the overlay panel
        virtual String getOverlayTitle() const = 0;
        
        /// Returns true if the back button should be shown
        virtual bool shouldShowBackButton() const = 0;
        
        /// Called when the back button is clicked
        virtual void onOverlayBackButton() = 0;
        
        /// Sets the parent overlay panel (called by OverlayPanel)
        /// Allows content to request chrome refresh when needed
        virtual void setParentOverlay(OverlayPanel* parent) = 0;
    };
}

#endif
