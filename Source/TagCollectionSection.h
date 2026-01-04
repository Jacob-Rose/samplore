/*
  ==============================================================================

    TagCollectionSection.h
    Created: 2025
    Author:  Samplore Team

  ==============================================================================
*/

#ifndef TAGCOLLECTIONSECTION_H
#define TAGCOLLECTIONSECTION_H

#include "JuceHeader.h"
#include "TagContainer.h"
#include "ThemeManager.h"

namespace samplore
{
	class TagCollectionSection : public Component,
	                              public DragAndDropTarget,
	                              public ThemeManager::Listener
	{
	public:
		TagCollectionSection(juce::String collectionName, bool isDefault = false);
		~TagCollectionSection();

		void paint(Graphics&) override;
		void resized() override;

		/// Collapse/expand control
		void setCollapsed(bool collapsed);
		bool isCollapsed() const { return mCollapsed; }
		void toggleCollapsed() { setCollapsed(!mCollapsed); }

		/// Calculate required height for this section
		int calculateHeight() const;

		/// Update displayed tags
		void setTags(StringArray tags);
		juce::String getCollectionName() const { return mCollectionName; }

		/// DragAndDropTarget interface
		bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
		void itemDropped(const SourceDetails& dragSourceDetails) override;
		void itemDragEnter(const SourceDetails& dragSourceDetails) override;
		void itemDragExit(const SourceDetails& dragSourceDetails) override;

		/// ThemeManager::Listener
		void themeChanged(ThemeManager::Theme newTheme) override;

		/// Callback when down-arrow clicked (to reorder collection)
		std::function<void(juce::String)> onMoveDown;

		/// Callback when collapsed state changes (for parent to relayout)
		std::function<void()> onLayoutChanged;

		static constexpr int HEADER_HEIGHT = 28;

	private:
		juce::String mCollectionName;
		bool mCollapsed = false;
		bool mIsDefault = false;
		bool mDragHighlight = false;

		// Header components
		TextButton mCollapseButton;
		Label mTitleLabel;
		TextButton mMoveDownButton;

		// Content
		TagContainer mTagContainer;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TagCollectionSection)
	};
}

#endif
