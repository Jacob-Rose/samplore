/*
  ==============================================================================

    ThemeManager_Example.cpp
    Example of using ThemeManager::Listener
    
    This file demonstrates how to use the specialized ThemeManager listener.
    You can delete this file - it's just for reference.

  ==============================================================================
*/

#include "JuceHeader.h"
#include "ThemeManager.h"

namespace samplore
{
    //==========================================================================
    /// Example component that responds to theme changes
    class ThemedComponent : public juce::Component,
                           public ThemeManager::Listener
    {
    public:
        ThemedComponent()
        {
            // Register as a listener
            ThemeManager::getInstance().addListener(this);
            
            // Set initial colors
            updateColors();
        }
        
        ~ThemedComponent() override
        {
            // Always remove listener in destructor!
            ThemeManager::getInstance().removeListener(this);
        }
        
        //======================================================================
        // ThemeManager::Listener implementation
        
        void themeChanged(ThemeManager::Theme newTheme) override
        {
            // React to theme change (Dark/Light)
            DBG("Theme changed to: " + 
                String(newTheme == ThemeManager::Theme::Dark ? "Dark" : "Light"));
            
            updateColors();
            repaint();
        }
        
        void colorChanged(ThemeManager::ColorRole role, Colour newColor) override
        {
            // React to specific color changes
            if (role == ThemeManager::ColorRole::AccentPrimary)
            {
                DBG("Accent color changed");
                updateColors();
                repaint();
            }
        }
        
        void themeReset() override
        {
            // React to theme reset
            DBG("Theme reset to defaults");
            updateColors();
            repaint();
        }
        
        //======================================================================
        void paint(juce::Graphics& g) override
        {
            auto& theme = ThemeManager::getInstance();
            
            // Use theme colors
            g.fillAll(theme.get(ThemeManager::ColorRole::Background));
            
            g.setColour(theme.get(ThemeManager::ColorRole::AccentPrimary));
            g.fillRoundedRectangle(getLocalBounds().reduced(20).toFloat(), 10.0f);
            
            g.setColour(theme.get(ThemeManager::ColorRole::TextPrimary));
            g.drawText("Themed Component", getLocalBounds(), 
                      juce::Justification::centred);
        }
        
    private:
        void updateColors()
        {
            // Cache frequently-used colors if needed
            auto& theme = ThemeManager::getInstance();
            mBackgroundColor = theme.get(ThemeManager::ColorRole::Background);
            mAccentColor = theme.get(ThemeManager::ColorRole::AccentPrimary);
        }
        
        Colour mBackgroundColor;
        Colour mAccentColor;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThemedComponent)
    };
    
    //==========================================================================
    /// Example: Only care about theme changes, not individual colors
    class SimpleThemedComponent : public juce::Component,
                                 public ThemeManager::Listener
    {
    public:
        SimpleThemedComponent()
        {
            ThemeManager::getInstance().addListener(this);
        }
        
        ~SimpleThemedComponent() override
        {
            ThemeManager::getInstance().removeListener(this);
        }
        
        // Only implement the callback you care about!
        void themeChanged(ThemeManager::Theme newTheme) override
        {
            repaint();
        }
        
        // No need to implement colorChanged() or themeReset() 
        // if you don't care about them - they have default empty implementations
        
        void paint(juce::Graphics& g) override
        {
            auto& theme = ThemeManager::getInstance();
            g.fillAll(theme.get(ThemeManager::ColorRole::Background));
        }
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleThemedComponent)
    };
}
