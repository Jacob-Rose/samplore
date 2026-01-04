/*
  ==============================================================================

    TagContainer.h
    Created: 26 Jun 2019 6:31:38pm
    Author:  Jake Rose

  ==============================================================================
*/

#ifndef TAGCONTAINER_H
#define TAGCONTAINER_H

#include "JuceHeader.h"

#include "TagTile.h"

namespace samplore
{
	class TagContainer : public Component
	{
	public:
		//========================================================
		TagContainer(bool updateHeight);
		~TagContainer();

		void paint(Graphics&) override;
		void resized() override;

		StringArray getTags();
		void addTag(juce::String tag);
		void setTags(StringArray newTags);
		void removeTag(juce::String tag);
		void clearTags();


		//======================================================
		int calculateAllRowsHeight() const;
		int getRowCount() const;
	private:

		void updateItemBounds();
		//=============================================================================
		std::vector<TagTile*> mUsedSampleTags;
		StringArray mCurrentTags; //tags to store in the container
		int mLineCount = 0;
		Font tagFont;
		bool mIsFull = false;
		bool mUpdateHeight = false;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TagContainer)
	};
}
#endif