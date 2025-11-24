/*
  ==============================================================================

    TagExplorer_V2.h
    Created: 30 May 2020 6:33:22pm
    Author:  jacob

  ==============================================================================
*/

#ifndef TAGEXPLORERV2
#define TAGEXPLORERV2

#include "JuceHeader.h"
#include "TagContainer.h"

namespace samplify
{
	class TagExplorer_V2 : public Component, public ChangeListener
	{
	public:
		TagExplorer_V2();
		~TagExplorer_V2();

		void resized() override;
		void paint(Graphics&) override;
		void addNewTag();
		void updateTags(juce::String query);

		void changeListenerCallback(ChangeBroadcaster* source) override;
	private:

		TextButton mNewButtonTag;
		Viewport mTagViewport;
		TagContainer mTagsContainer;
		std::unique_ptr<AlertWindow> mAlertWindow;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TagExplorer_V2)
	};
}
#endif