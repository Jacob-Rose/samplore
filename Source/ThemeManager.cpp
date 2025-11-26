/*
  ==============================================================================

    ThemeManager.cpp
    Created: 2025
    Author:  SamplifyPlus Team

  ==============================================================================
*/

#include "ThemeManager.h"
#include "SamplifyProperties.h"

using namespace samplify;

std::unique_ptr<ThemeManager> ThemeManager::instance = nullptr;

ThemeManager::ThemeManager()
    : currentTheme(Theme::Dark)
    , useCustomColors(false)
{
    initializeDefaultPalettes();
}

void ThemeManager::initInstance()
{
    instance = std::make_unique<ThemeManager>();
    instance->loadPreferences();
}

void ThemeManager::cleanupInstance()
{
    instance.reset();
}

ThemeManager& ThemeManager::getInstance()
{
    return *instance;
}

void ThemeManager::initializeDefaultPalettes()
{
    // ====== DARK THEME - "Studio Pro Dark" ======
    darkTheme.colors[ColorRole::Background]          = Colour(0xFF0A0A0B);  // #0A0A0B
    darkTheme.colors[ColorRole::BackgroundSecondary] = Colour(0xFF151518);  // #151518
    darkTheme.colors[ColorRole::BackgroundTertiary]  = Colour(0xFF1E1E22);  // #1E1E22
    darkTheme.colors[ColorRole::Surface]             = Colour(0xFF252529);  // #252529
    darkTheme.colors[ColorRole::SurfaceHover]        = Colour(0xFF2D2D32);  // #2D2D32
    darkTheme.colors[ColorRole::SurfaceActive]       = Colour(0xFF35353B);  // #35353B

    darkTheme.colors[ColorRole::TextPrimary]         = Colour(0xFFE8E8EA);  // #E8E8EA
    darkTheme.colors[ColorRole::TextSecondary]       = Colour(0xFFA0A0A5);  // #A0A0A5
    darkTheme.colors[ColorRole::TextDisabled]        = Colour(0xFF5A5A5F);  // #5A5A5F

    darkTheme.colors[ColorRole::AccentPrimary]       = Colour(0xFF4A9EFF);  // #4A9EFF (blue)
    darkTheme.colors[ColorRole::AccentSecondary]     = Colour(0xFF7B61FF);  // #7B61FF (purple)

    darkTheme.colors[ColorRole::Success]             = Colour(0xFF34C759);  // #34C759 (green)
    darkTheme.colors[ColorRole::Warning]             = Colour(0xFFFF9500);  // #FF9500 (orange)
    darkTheme.colors[ColorRole::Error]               = Colour(0xFFFF3B30);  // #FF3B30 (red)
    darkTheme.colors[ColorRole::Info]                = Colour(0xFF4A9EFF);  // #4A9EFF (blue)

    darkTheme.colors[ColorRole::WaveformPrimary]     = Colour(0xFF4A9EFF);  // #4A9EFF
    darkTheme.colors[ColorRole::WaveformSecondary]   = Colour(0xFF7B61FF);  // #7B61FF

    darkTheme.colors[ColorRole::Border]              = Colour(0xFF2D2D32);  // #2D2D32
    darkTheme.colors[ColorRole::BorderFocus]         = Colour(0xFF4A9EFF);  // #4A9EFF

    // ====== LIGHT THEME - "Studio Pro Light" ======
    lightTheme.colors[ColorRole::Background]          = Colour(0xFFFAFAFA);  // #FAFAFA
    lightTheme.colors[ColorRole::BackgroundSecondary] = Colour(0xFFF0F0F2);  // #F0F0F2
    lightTheme.colors[ColorRole::BackgroundTertiary]  = Colour(0xFFE5E5E8);  // #E5E5E8
    lightTheme.colors[ColorRole::Surface]             = Colour(0xFFFFFFFF);  // #FFFFFF
    lightTheme.colors[ColorRole::SurfaceHover]        = Colour(0xFFF5F5F7);  // #F5F5F7
    lightTheme.colors[ColorRole::SurfaceActive]       = Colour(0xFFECECEF);  // #ECECEF

    lightTheme.colors[ColorRole::TextPrimary]         = Colour(0xFF1C1C1E);  // #1C1C1E
    lightTheme.colors[ColorRole::TextSecondary]       = Colour(0xFF636366);  // #636366
    lightTheme.colors[ColorRole::TextDisabled]        = Colour(0xFFAEAEB2);  // #AEAEB2

    lightTheme.colors[ColorRole::AccentPrimary]       = Colour(0xFF007AFF);  // #007AFF (blue)
    lightTheme.colors[ColorRole::AccentSecondary]     = Colour(0xFF5E5CE6);  // #5E5CE6 (purple)

    lightTheme.colors[ColorRole::Success]             = Colour(0xFF28A745);  // #28A745 (green)
    lightTheme.colors[ColorRole::Warning]             = Colour(0xFFFF9500);  // #FF9500 (orange)
    lightTheme.colors[ColorRole::Error]               = Colour(0xFFDC3545);  // #DC3545 (red)
    lightTheme.colors[ColorRole::Info]                = Colour(0xFF007AFF);  // #007AFF (blue)

    lightTheme.colors[ColorRole::WaveformPrimary]     = Colour(0xFF007AFF);  // #007AFF
    lightTheme.colors[ColorRole::WaveformSecondary]   = Colour(0xFF5E5CE6);  // #5E5CE6

    lightTheme.colors[ColorRole::Border]              = Colour(0xFFD1D1D6);  // #D1D1D6
    lightTheme.colors[ColorRole::BorderFocus]         = Colour(0xFF007AFF);  // #007AFF
}

void ThemeManager::setTheme(Theme theme)
{
    if (currentTheme != theme)
    {
        currentTheme = theme;
        savePreferences();
    }
}

Colour ThemeManager::get(ColorRole role) const
{
    // Check for user custom colors first
    if (useCustomColors)
    {
        auto it = customColors.colors.find(role);
        if (it != customColors.colors.end())
        {
            return it->second;
        }
    }

    // Fall back to theme palette
    const ThemePalette& palette = getCurrentPalette();
    auto it = palette.colors.find(role);
    if (it != palette.colors.end())
    {
        return it->second;
    }

    // Fallback to magenta for missing colors (debug)
    return Colours::magenta;
}

void ThemeManager::setCustomColor(ColorRole role, Colour color)
{
    customColors.colors[role] = color;
    useCustomColors = true;
    savePreferences();
}

void ThemeManager::resetToDefaultColors()
{
    customColors.colors.clear();
    useCustomColors = false;
    savePreferences();
}

const ThemeManager::ThemePalette& ThemeManager::getCurrentPalette() const
{
    return (currentTheme == Theme::Dark) ? darkTheme : lightTheme;
}

float ThemeManager::getSpacing(Spacing spacing) const
{
    switch (spacing)
    {
        case Spacing::Hairline:  return 1.0f;
        case Spacing::XXS:       return 2.0f;
        case Spacing::XS:        return 4.0f;
        case Spacing::SM:        return 8.0f;   // Base unit
        case Spacing::MD:        return 16.0f;
        case Spacing::LG:        return 24.0f;
        case Spacing::XL:        return 32.0f;
        case Spacing::XXL:       return 48.0f;
        default:                 return 8.0f;
    }
}

juce::DropShadow ThemeManager::getShadow(Elevation elevation) const
{
    switch (elevation)
    {
        case Elevation::Level0:
            return juce::DropShadow(Colours::transparentBlack, 0, juce::Point<int>(0, 0));

        case Elevation::Level1:  // Tiles at rest
            return juce::DropShadow(Colours::black.withAlpha(0.2f), 3, juce::Point<int>(0, 1));

        case Elevation::Level2:  // Tiles on hover
            return juce::DropShadow(Colours::black.withAlpha(0.3f), 8, juce::Point<int>(0, 4));

        case Elevation::Level3:  // Dialogs
            return juce::DropShadow(Colours::black.withAlpha(0.4f), 16, juce::Point<int>(0, 8));

        case Elevation::Level4:  // Modals
            return juce::DropShadow(Colours::black.withAlpha(0.5f), 32, juce::Point<int>(0, 16));

        default:
            return juce::DropShadow(Colours::transparentBlack, 0, juce::Point<int>(0, 0));
    }
}

void ThemeManager::savePreferences()
{
    auto* props = SamplifyProperties::getInstance();
    if (props == nullptr)
        return;

    auto* settings = props->getUserSettings();
    if (settings == nullptr)
        return;

    // Save current theme
    settings->setValue("theme", currentTheme == Theme::Dark ? "dark" : "light");

    // Save custom colors if any
    if (useCustomColors)
    {
        for (const auto& pair : customColors.colors)
        {
            String key = "customColor_" + String((int)pair.first);
            settings->setValue(key, pair.second.toString());
        }
    }

    props->savePropertiesFile();
}

void ThemeManager::loadPreferences()
{
    auto* props = SamplifyProperties::getInstance();
    if (props == nullptr)
        return;

    auto* settings = props->getUserSettings();
    if (settings == nullptr)
        return;

    // Load theme
    String themeStr = settings->getValue("theme", "dark");
    currentTheme = (themeStr == "dark") ? Theme::Dark : Theme::Light;

    // Load custom colors
    customColors.colors.clear();
    useCustomColors = false;

    for (int i = 0; i < (int)ColorRole::BorderFocus + 1; i++)
    {
        String key = "customColor_" + String(i);
        String colorStr = settings->getValue(key, "");
        if (colorStr.isNotEmpty())
        {
            customColors.colors[(ColorRole)i] = Colour::fromString(colorStr);
            useCustomColors = true;
        }
    }

    // Migrate old AppValues colors if present and no custom colors set
    if (!useCustomColors)
    {
        String oldBg = settings->getValue("MAIN_BACKGROUND_COLOR", "");
        String oldFg = settings->getValue("MAIN_FOREGROUND_COLOR", "");

        if (oldBg.isNotEmpty() || oldFg.isNotEmpty())
        {
            // Attempt to determine which theme was closer to old settings
            Colour oldBgColor = oldBg.isNotEmpty() ?
                Colour::fromString(oldBg) : Colours::white;

            if (oldBgColor.getPerceivedBrightness() > 0.5f)
            {
                currentTheme = Theme::Light;
            }
            else
            {
                currentTheme = Theme::Dark;
            }
        }
    }
}
