/*
  ==============================================================================

    ActiveTagsBar.cpp
    Created: 1 Jan 2026
    Author:  OpenCode

  ==============================================================================
*/

#include "ActiveTagsBar.h"
#include "SamplifyProperties.h"
#include "SampleLibrary.h"

namespace samplore
{
    //==============================================================================
    // ActiveTagPill
    //==============================================================================

    ActiveTagPill::ActiveTagPill(const String& tag)
        : mTag(tag)
    {
    }

    void ActiveTagPill::paint(Graphics& g)
    {
        auto* library = SamplifyProperties::getInstance()->getSampleLibrary().get();

        Colour tagColor = library ? library->getTagColor(mTag) : Colours::grey;

        // Background with hover effect
        float alpha = mHovered ? 1.0f : 0.85f;
        g.setColour(tagColor.withAlpha(alpha));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 12.0f);

        // Border
        g.setColour(tagColor.darker(0.3f).withAlpha(0.7f));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 12.0f, 1.0f);

        // Text color for contrast
        Colour textColor;
        if (tagColor.getPerceivedBrightness() > 0.55f)
            textColor = Colours::black.withAlpha(0.85f);
        else
            textColor = Colours::white.withAlpha(0.95f);

        g.setColour(textColor);
        g.setFont(FontOptions(13.0f));

        // Draw tag text with X button
        auto bounds = getLocalBounds().reduced(8, 2);
        auto xBounds = bounds.removeFromRight(14);

        g.drawText(mTag, bounds, Justification::centredLeft, true);

        // Draw X for removal
        g.setFont(FontOptions(11.0f, Font::bold));
        g.drawText(CharPointer_UTF8("\xc3\x97"), xBounds, Justification::centred);
    }

    void ActiveTagPill::mouseUp(const MouseEvent& e)
    {
        if (e.mods.isLeftButtonDown() && onRemove)
        {
            onRemove(mTag);
        }
    }

    void ActiveTagPill::mouseEnter(const MouseEvent& e)
    {
        mHovered = true;
        repaint();
    }

    void ActiveTagPill::mouseExit(const MouseEvent& e)
    {
        mHovered = false;
        repaint();
    }

    //==============================================================================
    // ActiveTagsBar
    //==============================================================================

    ActiveTagsBar::ActiveTagsBar()
    {
        ThemeManager::getInstance().addListener(this);

        addAndMakeVisible(mViewport);
        mViewport.setViewedComponent(&mPillContainer, false);
        mViewport.setScrollBarsShown(false, true, false, true);
        mViewport.setScrollBarThickness(6);
    }

    ActiveTagsBar::~ActiveTagsBar()
    {
        ThemeManager::getInstance().removeListener(this);
    }

    void ActiveTagsBar::paint(Graphics& g)
    {
        if (mActiveTags.isEmpty())
            return;

        auto& theme = ThemeManager::getInstance();

        // Subtle background
        g.setColour(theme.getColorForRole(ThemeManager::ColorRole::Surface).withAlpha(0.3f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
    }

    void ActiveTagsBar::resized()
    {
        mViewport.setBounds(getLocalBounds());

        int totalWidth = layoutPills();
        mPillContainer.setSize(jmax(totalWidth, getWidth()), getHeight());
    }

    int ActiveTagsBar::layoutPills()
    {
        if (mPills.isEmpty())
            return 0;

        int x = BAR_PADDING;
        int y = (BAR_HEIGHT - PILL_HEIGHT) / 2;
        Font font(FontOptions(13.0f));

        for (auto* pill : mPills)
        {
            int textWidth = font.getStringWidth(pill->getTag());
            int pillWidth = textWidth + PILL_PADDING * 2 + 16;

            pill->setBounds(x, y, pillWidth, PILL_HEIGHT);
            x += pillWidth + PILL_SPACING;
        }

        return x + BAR_PADDING;
    }

    void ActiveTagsBar::addTag(const String& tag)
    {
        if (tag.isEmpty() || mActiveTags.contains(tag))
            return;

        mActiveTags.add(tag);
        rebuildPills();

        if (onTagsChanged)
            onTagsChanged();
    }

    void ActiveTagsBar::removeTag(const String& tag)
    {
        int index = mActiveTags.indexOf(tag);
        if (index < 0)
            return;

        mActiveTags.remove(index);
        rebuildPills();

        if (onTagsChanged)
            onTagsChanged();
    }

    bool ActiveTagsBar::toggleTag(const String& tag)
    {
        if (mActiveTags.contains(tag))
        {
            removeTag(tag);
            return false;
        }
        else
        {
            addTag(tag);
            return true;
        }
    }

    void ActiveTagsBar::clearTags()
    {
        if (mActiveTags.isEmpty())
            return;

        mActiveTags.clear();
        rebuildPills();

        if (onTagsChanged)
            onTagsChanged();
    }

    void ActiveTagsBar::rebuildPills()
    {
        mPills.clear();

        for (const auto& tag : mActiveTags)
        {
            auto* pill = new ActiveTagPill(tag);
            pill->onRemove = [this](const String& t) { removeTag(t); };
            mPillContainer.addAndMakeVisible(pill);
            mPills.add(pill);
        }

        resized();
        repaint();
    }

    void ActiveTagsBar::themeChanged(ThemeManager::Theme)
    {
        repaint();
    }

    void ActiveTagsBar::colorChanged(ThemeManager::ColorRole, Colour)
    {
        repaint();
    }
}
