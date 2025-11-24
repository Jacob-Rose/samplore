/*
  ==============================================================================

	SampleTile.h
	Created: 27 June 2019
	Author:  Jake Rose

  ==============================================================================
*/
#ifndef TAGEXPLORER_H
#define TAGEXPLORER_H

#include "JuceHeader.h"

#include "TagContainer.h"

namespace samplify
{
	class TagExplorer : public Component, public ChangeListener
	{
	public:
		TagExplorer();
		~TagExplorer();
		class Container : public Component
		{
		public:
			Container() : Component(), mNewTags(true), mContainedTags(true), mNotContainedTags(true)
			{
				addAndMakeVisible(mNewTags);
				addAndMakeVisible(mContainedTags);
				addAndMakeVisible(mNotContainedTags);
			}
			void paint(Graphics& g) override;
			//this references samplifyproperties
			void addNewTag(juce::String tag)
			{
				mNewTags.addTag(tag);
				updateTagContainerBounds();
			}
			void updateTags(juce::String newSearch);
			void updateTagContainerBounds();
			void resetTags();
			void removeNewTag(juce::String tag);

			void resized() override
			{
				updateTagContainerBounds();
			}
		private:
			const Font infoFont = Font(16.0f);
			TagContainer mNewTags;
			TagContainer mContainedTags; //all tags for current files in directory with text from search bar contained in it
			TagContainer mNotContainedTags; //all tags for lower half, only ones with the text in search bar contained in it
		};
		Container& getTagContainer() {
			return mTagsContainer;
		}
		void resized() override;
		void paint(Graphics&) override;
		void addNewTag();
		Container& getContainer() { return mTagsContainer; }

		void changeListenerCallback(ChangeBroadcaster* source) override;
	private:

		TextButton mNewButtonTag;
		Viewport mTagViewport;
		Container mTagsContainer;
		std::unique_ptr<AlertWindow> mAlertWindow;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TagExplorer)
	};


}
#endif