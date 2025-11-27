/*
  ==============================================================================

    IconLibrary.cpp
    Created: 2025
    Author:  Samplore Team

  ==============================================================================
*/

#include "IconLibrary.h"

using namespace samplore;

std::unique_ptr<IconLibrary> IconLibrary::instance = nullptr;

IconLibrary::IconLibrary()
{
    loadAllIcons();
}

IconLibrary::~IconLibrary()
{
    cleanupIcons();
}

void IconLibrary::initInstance()
{
    instance = std::make_unique<IconLibrary>();
}

void IconLibrary::cleanupInstance()
{
    instance.reset();
}

IconLibrary& IconLibrary::getInstance()
{
    return *instance;
}

const char* IconLibrary::getSvgForIcon(Icon icon) const
{
    switch (icon)
    {
        // Existing icons
        case Icon::Close:         return Icons::close_svg;
        case Icon::Info:          return Icons::info_svg;
        case Icon::Check:         return Icons::correct_svg;
        case Icon::Minus:         return Icons::minus_svg;

        // Playback
        case Icon::Play:          return Icons::play_svg;
        case Icon::Pause:         return Icons::pause_svg;
        case Icon::Stop:          return Icons::stop_svg;
        case Icon::Loop:          return Icons::loop_svg;
        case Icon::Shuffle:       return Icons::shuffle_svg;

        // File operations
        case Icon::Folder:        return Icons::folder_svg;
        case Icon::FolderOpen:    return Icons::folder_open_svg;
        case Icon::File:          return Icons::file_svg;
        case Icon::Search:        return Icons::search_svg;
        case Icon::Filter:        return Icons::filter_svg;

        // Editing
        case Icon::Tag:           return Icons::tag_svg;
        case Icon::Edit:          return Icons::edit_svg;
        case Icon::Delete:        return Icons::delete_svg;
        case Icon::ColorPicker:   return Icons::color_picker_svg;
        case Icon::Settings:      return Icons::settings_svg;

        // Navigation
        case Icon::ChevronLeft:   return Icons::chevron_left_svg;
        case Icon::ChevronRight:  return Icons::chevron_right_svg;
        case Icon::ChevronUp:     return Icons::chevron_up_svg;
        case Icon::ChevronDown:   return Icons::chevron_down_svg;

        // Status
        case Icon::Loading:       return Icons::loading_svg;

        default:                  return Icons::info_svg;
    }
}

void IconLibrary::loadAllIcons()
{
    // Icons are loaded on-demand, so just clear the cache
    iconCache.clear();
}

void IconLibrary::cleanupIcons()
{
    iconCache.clear();
}

Drawable* IconLibrary::getIcon(Icon icon, Colour color)
{
    // Check if icon is in cache
    auto it = iconCache.find(icon);
    if (it != iconCache.end())
    {
        // Update color and return cached drawable
        it->second->replaceColour(Colours::white, color);
        it->second->replaceColour(Colours::black, color);
        return it->second.get();
    }

    // Load icon from SVG
    const char* svgData = getSvgForIcon(icon);
    if (svgData == nullptr)
        return nullptr;

    std::unique_ptr<XmlElement> svgXml = XmlDocument::parse(svgData);
    if (svgXml == nullptr)
        return nullptr;

    auto drawable = Drawable::createFromSVG(*svgXml);
    if (drawable == nullptr)
        return nullptr;

    // Replace colors
    drawable->replaceColour(Colours::white, color);
    drawable->replaceColour(Colours::black, color);

    // Cache and return
    Drawable* result = drawable.get();
    iconCache[icon] = std::move(drawable);
    return result;
}

void IconLibrary::drawIcon(Graphics& g, Icon icon, Rectangle<float> bounds, Colour color)
{
    Drawable* drawable = getIcon(icon, color);
    if (drawable != nullptr)
    {
        drawable->drawWithin(g, bounds, RectanglePlacement::centred, 1.0f);
    }
}

void IconLibrary::updateIconColors(Colour newColor)
{
    for (auto& pair : iconCache)
    {
        pair.second->replaceColour(Colours::white, newColor);
        pair.second->replaceColour(Colours::black, newColor);
    }
}
