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
        Colour getColorForRole(ColorRole role) const;
        void setCustomColor(ColorRole role, Colour color);
        void resetToDefaultColors();

        // Persistence
        void savePreferences();
        void loadPreferences();

        // Backward compatibility with AppValues
        Colour getBackgroundColor() const { return getColorForRole(ColorRole::Background); }
        Colour getForegroundColor() const { return getColorForRole(ColorRole::AccentPrimary); }

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
