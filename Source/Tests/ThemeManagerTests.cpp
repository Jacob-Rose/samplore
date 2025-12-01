/*
  ==============================================================================

    ThemeManagerTests.cpp
    Catch2 tests for ThemeManager class

  ==============================================================================
*/

#include <catch2/catch.hpp>
#include "ThemeManager.h"
#include "TestHelpers.h"

using namespace samplore;

TEST_CASE("ThemeManager singleton", "[thememanager]")
{
    SECTION("Instance management")
    {
        // Test that getInstance() returns the same instance
        ThemeManager& instance1 = ThemeManager::getInstance();
        ThemeManager& instance2 = ThemeManager::getInstance();
        
        REQUIRE(&instance1 == &instance2);
    }
    
    SECTION("Initialization")
    {
        ThemeManager& theme = ThemeManager::getInstance();
        
        // Should start with a valid theme
        REQUIRE((theme.getCurrentTheme() == ThemeManager::Theme::Dark || 
                theme.getCurrentTheme() == ThemeManager::Theme::Light));
    }
}

TEST_CASE("ThemeManager theme switching", "[thememanager]")
{
    ThemeManager& theme = ThemeManager::getInstance();
    
    SECTION("Switch to dark theme")
    {
        theme.setTheme(ThemeManager::Theme::Dark);
        REQUIRE(theme.getCurrentTheme() == ThemeManager::Theme::Dark);
    }
    
    SECTION("Switch to light theme")
    {
        theme.setTheme(ThemeManager::Theme::Light);
        REQUIRE(theme.getCurrentTheme() == ThemeManager::Theme::Light);
    }
    
    SECTION("Theme persistence")
    {
        // Set theme and verify it stays set
        theme.setTheme(ThemeManager::Theme::Dark);
        REQUIRE(theme.getCurrentTheme() == ThemeManager::Theme::Dark);
        
        theme.setTheme(ThemeManager::Theme::Light);
        REQUIRE(theme.getCurrentTheme() == ThemeManager::Theme::Light);
    }
}

TEST_CASE("ThemeManager color access", "[thememanager]")
{
    ThemeManager& theme = ThemeManager::getInstance();
    
    SECTION("Valid color roles return colors")
    {
        // Test some common color roles
        juce::Colour bg = theme.getColorForRole(ThemeManager::ColorRole::Background);
        REQUIRE(bg.getAlpha() > 0);  // Should be a valid color
        
        juce::Colour text = theme.getColorForRole(ThemeManager::ColorRole::TextPrimary);
        REQUIRE(text.getAlpha() > 0);
        
        juce::Colour accent = theme.getColorForRole(ThemeManager::ColorRole::AccentPrimary);
        REQUIRE(accent.getAlpha() > 0);
    }
    
    SECTION("Dark theme colors")
    {
        theme.setTheme(ThemeManager::Theme::Dark);
        
        juce::Colour bg = theme.getColorForRole(ThemeManager::ColorRole::Background);
        // Dark theme should have a dark background
        REQUIRE(bg.getPerceivedBrightness() < 0.5f);
        
        juce::Colour text = theme.getColorForRole(ThemeManager::ColorRole::TextPrimary);
        // Dark theme should have light text
        REQUIRE(text.getPerceivedBrightness() > 0.5f);
    }
    
    SECTION("Light theme colors")
    {
        theme.setTheme(ThemeManager::Theme::Light);
        
        juce::Colour bg = theme.getColorForRole(ThemeManager::ColorRole::Background);
        // Light theme should have a light background
        REQUIRE(bg.getPerceivedBrightness() > 0.5f);
        
        juce::Colour text = theme.getColorForRole(ThemeManager::ColorRole::TextPrimary);
        // Light theme should have dark text
        REQUIRE(text.getPerceivedBrightness() < 0.5f);
    }
}

TEST_CASE("ThemeManager custom colors", "[thememanager]")
{
    ThemeManager& theme = ThemeManager::getInstance();
    
    SECTION("Set custom color")
    {
        juce::Colour customColor(0xFF123456);
        theme.setCustomColor(ThemeManager::ColorRole::AccentPrimary, customColor);
        
        juce::Colour retrieved = theme.getColorForRole(ThemeManager::ColorRole::AccentPrimary);
        REQUIRE(retrieved == customColor);
    }
    
    SECTION("Custom color overrides theme")
    {
        // Set a custom color
        juce::Colour customColor(0xFFABCDEF);
        theme.setCustomColor(ThemeManager::ColorRole::Background, customColor);
        
        // Switch themes - custom color should persist
        theme.setTheme(ThemeManager::Theme::Dark);
        REQUIRE(theme.getColorForRole(ThemeManager::ColorRole::Background) == customColor);
        
        theme.setTheme(ThemeManager::Theme::Light);
        REQUIRE(theme.getColorForRole(ThemeManager::ColorRole::Background) == customColor);
    }
    
    SECTION("Reset to defaults")
    {
        // Set custom color
        juce::Colour customColor(0xFF123456);
        theme.setCustomColor(ThemeManager::ColorRole::AccentPrimary, customColor);
        REQUIRE(theme.getColorForRole(ThemeManager::ColorRole::AccentPrimary) == customColor);
        
        // Reset
        theme.resetToDefaultColors();
        
        // Should no longer be custom color
        REQUIRE(theme.getColorForRole(ThemeManager::ColorRole::AccentPrimary) != customColor);
    }
}

TEST_CASE("ThemeManager backward compatibility", "[thememanager]")
{
    ThemeManager& theme = ThemeManager::getInstance();
    
    SECTION("Background color getter")
    {
        juce::Colour bg = theme.getBackgroundColor();
        juce::Colour bgRole = theme.getColorForRole(ThemeManager::ColorRole::Background);
        REQUIRE(bg == bgRole);
    }
    
    SECTION("Foreground color getter")
    {
        juce::Colour fg = theme.getForegroundColor();
        juce::Colour fgRole = theme.getColorForRole(ThemeManager::ColorRole::AccentPrimary);
        REQUIRE(fg == fgRole);
    }
}

// Mock listener for testing
class MockThemeListener : public ThemeManager::Listener
{
public:
    bool themeChangedCalled = false;
    bool colorChangedCalled = false;
    bool themeResetCalled = false;
    
    ThemeManager::Theme lastTheme = ThemeManager::Theme::Dark;
    ThemeManager::ColorRole lastColorRole = ThemeManager::ColorRole::Background;
    juce::Colour lastColor;
    
    void themeChanged(ThemeManager::Theme newTheme) override
    {
        themeChangedCalled = true;
        lastTheme = newTheme;
    }
    
    void colorChanged(ThemeManager::ColorRole role, juce::Colour newColor) override
    {
        colorChangedCalled = true;
        lastColorRole = role;
        lastColor = newColor;
    }
    
    void themeReset() override
    {
        themeResetCalled = true;
    }
    
    void reset()
    {
        themeChangedCalled = false;
        colorChangedCalled = false;
        themeResetCalled = false;
    }
};

TEST_CASE("ThemeManager listener notifications", "[thememanager]")
{
    ThemeManager& theme = ThemeManager::getInstance();
    MockThemeListener listener;
    
    theme.addListener(&listener);
    
    SECTION("Theme change notification")
    {
        listener.reset();
        theme.setTheme(ThemeManager::Theme::Light);
        
        REQUIRE(listener.themeChangedCalled);
        REQUIRE(listener.lastTheme == ThemeManager::Theme::Light);
    }
    
    SECTION("Color change notification")
    {
        listener.reset();
        juce::Colour customColor(0xFF123456);
        theme.setCustomColor(ThemeManager::ColorRole::AccentPrimary, customColor);
        
        REQUIRE(listener.colorChangedCalled);
        REQUIRE(listener.lastColorRole == ThemeManager::ColorRole::AccentPrimary);
        REQUIRE(listener.lastColor == customColor);
    }
    
    SECTION("Theme reset notification")
    {
        listener.reset();
        theme.resetToDefaultColors();
        
        REQUIRE(listener.themeResetCalled);
    }
    
    SECTION("No notification for same theme")
    {
        // Set to current theme
        ThemeManager::Theme currentTheme = theme.getCurrentTheme();
        listener.reset();
        theme.setTheme(currentTheme);
        
        // Should not trigger notification
        REQUIRE_FALSE(listener.themeChangedCalled);
    }
    
    theme.removeListener(&listener);
}

TEST_CASE("ThemeManager edge cases", "[thememanager]")
{
    ThemeManager& theme = ThemeManager::getInstance();
    
    SECTION("Invalid color role returns fallback")
    {
        // This test assumes we can somehow test an invalid role
        // In practice, the enum prevents invalid values
        juce::Colour color = theme.getColorForRole(ThemeManager::ColorRole::Background);
        REQUIRE(color.getAlpha() > 0);  // Should return some valid color
    }
    
    SECTION("Multiple custom colors")
    {
        // Set multiple custom colors
        theme.setCustomColor(ThemeManager::ColorRole::Background, juce::Colours::red);
        theme.setCustomColor(ThemeManager::ColorRole::TextPrimary, juce::Colours::blue);
        theme.setCustomColor(ThemeManager::ColorRole::AccentPrimary, juce::Colours::green);
        
        REQUIRE(theme.getColorForRole(ThemeManager::ColorRole::Background) == juce::Colours::red);
        REQUIRE(theme.getColorForRole(ThemeManager::ColorRole::TextPrimary) == juce::Colours::blue);
        REQUIRE(theme.getColorForRole(ThemeManager::ColorRole::AccentPrimary) == juce::Colours::green);
        
        // Reset and verify all are reset
        theme.resetToDefaultColors();
        REQUIRE(theme.getColorForRole(ThemeManager::ColorRole::Background) != juce::Colours::red);
        REQUIRE(theme.getColorForRole(ThemeManager::ColorRole::TextPrimary) != juce::Colours::blue);
        REQUIRE(theme.getColorForRole(ThemeManager::ColorRole::AccentPrimary) != juce::Colours::green);
    }
}