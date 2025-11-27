/*
  ==============================================================================

    ThemeManager.h
    Created: 2025
    Author:  Samplore Team

  ==============================================================================
*/

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "JuceHeader.h"
#include <map>

namespace samplore
{
    class ThemeManager
    {
    public:
        enum class Theme { Dark, Light };

        enum class ColorRole {
            // Surface colors (backgrounds)
            Background,
            BackgroundSecondary,
            BackgroundTertiary,
            Surface,
            SurfaceHover,
            SurfaceActive,

            // Text colors
            TextPrimary,
            TextSecondary,
            TextDisabled,

            // Accent colors
            AccentPrimary,
            AccentSecondary,

            // Semantic colors
            Success,
            Warning,
            Error,
            Info,

            // Specialized colors
            WaveformPrimary,
            WaveformSecondary,

            // Border colors
            Border,
            BorderFocus
        };

        enum class Spacing {
            Hairline,   // 1px
            XXS,        // 2px
            XS,         // 4px
            SM,         // 8px (base unit)
            MD,         // 16px
            LG,         // 24px
            XL,         // 32px
            XXL         // 48px
        };

        enum class Elevation {
            Level0,  // none
            Level1,  // tiles at rest
            Level2,  // tiles on hover
            Level3,  // dialogs
            Level4   // modals
        };

        struct ThemePalette {
            std::map<ColorRole, Colour> colors;
        };

        // Singleton pattern
        ThemeManager();
        static void initInstance();
        static void cleanupInstance();
        static ThemeManager& getInstance();

        // Theme management
        Theme getCurrentTheme() const { return currentTheme; }
        void setTheme(Theme theme);

        // Color access
        Colour get(ColorRole role) const;
        void setCustomColor(ColorRole role, Colour color);
        void resetToDefaultColors();

        // Spacing and elevation
        float getSpacing(Spacing spacing) const;
        juce::DropShadow getShadow(Elevation elevation) const;

        // Persistence
        void savePreferences();
        void loadPreferences();

        // Backward compatibility with AppValues
        Colour getBackgroundColor() const { return get(ColorRole::Background); }
        Colour getForegroundColor() const { return get(ColorRole::AccentPrimary); }

    private:
        void initializeDefaultPalettes();
        const ThemePalette& getCurrentPalette() const;

        ThemePalette darkTheme;
        ThemePalette lightTheme;
        ThemePalette customColors;  // User overrides
        Theme currentTheme;
        bool useCustomColors;

        static std::unique_ptr<ThemeManager> instance;
    };
}

#endif // THEMEMANAGER_H
