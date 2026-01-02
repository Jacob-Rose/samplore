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
#include "ThemeManager.h"

namespace samplore
{
	class TagExplorer 
		: public Component
		, public ChangeListener
		, public ThemeManager::Listener
	{
	public:
		TagExplorer();
		~TagExplorer();
		class Container : public Component
		{
		public:
			Container() : Component(), mTags(true)
			{
				addAndMakeVisible(mTags);
			}

			void updateTags();
			void updateBounds();

			void resized() override
			{
				// Parent handles bounds update
			}
		private:
			TagContainer mTags;
		};
		Container& getTagContainer() { return mTagsContainer; }

		void resized() override;
		void paint(Graphics&) override;
		void addNewTag();

		void changeListenerCallback(ChangeBroadcaster* source) override;
		
		//==================================================================
		// ThemeManager::Listener interface
		void themeChanged(ThemeManager::Theme newTheme) override;
		void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;
		
	private:

		TextButton mNewButtonTag;
		Viewport mTagViewport;
		Container mTagsContainer;
		std::unique_ptr<AlertWindow> mAlertWindow;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TagExplorer)
	};


}
#endif