/*
  ==============================================================================

    ActiveTagsBar.h
    Created: 1 Jan 2026
    Author:  OpenCode

    Summary: Small bar showing currently active/selected tags for filtering

  ==============================================================================
*/

#ifndef ACTIVETAGSBAR_H
#define ACTIVETAGSBAR_H

#include "JuceHeader.h"
#include "ThemeManager.h"

namespace samplore
{
    /// Small tag pill for the active tags bar
    class ActiveTagPill : public Component
    {
    public:
        ActiveTagPill(const String& tag);

        void paint(Graphics& g) override;
        void mouseUp(const MouseEvent& e) override;
        void mouseEnter(const MouseEvent& e) override;
        void mouseExit(const MouseEvent& e) override;

        const String& getTag() const { return mTag; }

        /// Called when user clicks to remove this tag
        std::function<void(const String&)> onRemove;

    private:
        String mTag;
        bool mHovered = false;
    };

    /// Bar displaying active filter tags with horizontal scrolling
    class ActiveTagsBar : public Component, public ThemeManager::Listener
    {
    public:
        ActiveTagsBar();
        ~ActiveTagsBar() override;

        void paint(Graphics& g) override;
        void resized() override;

        // ThemeManager::Listener
        void themeChanged(ThemeManager::Theme newTheme) override;
        void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;

        /// Adds a tag to the active filter
        void addTag(const String& tag);

        /// Removes a tag from the active filter
        void removeTag(const String& tag);

        /// Toggles a tag (adds if not present, removes if present)
        /// Returns true if tag is now active, false if removed
        bool toggleTag(const String& tag);

        /// Clears all active tags
        void clearTags();

        /// Gets the list of active tags
        const StringArray& getActiveTags() const { return mActiveTags; }

        /// Returns true if any tags are active
        bool hasActiveTags() const { return !mActiveTags.isEmpty(); }

        /// Callback when active tags change
        std::function<void()> onTagsChanged;

        /// Fixed height for the bar
        static constexpr int BAR_HEIGHT = 36;

    private:
        /// Content component that holds all pills (can be wider than viewport)
        class PillContainer : public Component
        {
        public:
            void resized() override {}
        };

        void rebuildPills();
        int layoutPills(); // Returns total width needed

        StringArray mActiveTags;
        OwnedArray<ActiveTagPill> mPills;
        PillContainer mPillContainer;
        Viewport mViewport;

        static constexpr int PILL_HEIGHT = 24;
        static constexpr int PILL_SPACING = 6;
        static constexpr int PILL_PADDING = 10;
        static constexpr int BAR_PADDING = 6;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ActiveTagsBar)
    };
}

#endif
