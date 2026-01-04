/*
  ==============================================================================

    TagExplorer.h
    Created: 30 May 2020 6:33:22pm
    Author:  jacob

  ==============================================================================
*/

#ifndef TAGEXPLORER_H
#define TAGEXPLORER_H

#include "JuceHeader.h"
#include "TagCollectionSection.h"

namespace samplore
{
	class TagExplorer : public Component, public ChangeListener
	{
	public:
		TagExplorer();
		~TagExplorer();

		void resized() override;
		void paint(Graphics&) override;
		void addNewTag();
		void updateTags(juce::String query);

		void changeListenerCallback(ChangeBroadcaster* source) override;

	private:
		void rebuildSections();

		TextButton mNewButtonTag;
		Viewport mTagViewport;
		Component mScrollContent;
		std::vector<std::unique_ptr<TagCollectionSection>> mSections;
		std::unique_ptr<AlertWindow> mAlertWindow;
		juce::String mCurrentQuery;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TagExplorer)
	};
}
#endif
