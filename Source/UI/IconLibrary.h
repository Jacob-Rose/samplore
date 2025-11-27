/*
  ==============================================================================

    IconLibrary.h
    Created: 2025
    Author:  Samplore Team

  ==============================================================================
*/

#ifndef ICONLIBRARY_H
#define ICONLIBRARY_H

#include "JuceHeader.h"
#include "../Icons.h"
#include <map>

namespace samplore
{
    class IconLibrary
    {
    public:
        enum class Icon {
            // Existing icons
            Close,
            Info,
            Check,
            Minus,

            // Playback
            Play,
            Pause,
            Stop,
            Loop,
            Shuffle,

            // File operations
            Folder,
            FolderOpen,
            File,
            Search,
            Filter,

            // Editing
            Tag,
            Edit,
            Delete,
            ColorPicker,
            Settings,

            // Navigation
            ChevronLeft,
            ChevronRight,
            ChevronUp,
            ChevronDown,

            // Status
            Loading
        };

        IconLibrary();
        ~IconLibrary();

        static void initInstance();
        static void cleanupInstance();
        static IconLibrary& getInstance();

        // Get a drawable icon
        Drawable* getIcon(Icon icon, Colour color = Colours::white);

        // Draw an icon directly
        void drawIcon(Graphics& g, Icon icon, Rectangle<float> bounds, Colour color = Colours::white);

        // Reload icons with new color
        void updateIconColors(Colour newColor);

    private:
        void loadAllIcons();
        void cleanupIcons();
        const char* getSvgForIcon(Icon icon) const;

        std::map<Icon, std::unique_ptr<Drawable>> iconCache;
        static std::unique_ptr<IconLibrary> instance;
    };
}

#endif // ICONLIBRARY_H
