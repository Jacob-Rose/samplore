/*
  ==============================================================================

    SampleContainer.h
    Created: 31 May 2018 1:20:21pm
    Author:  Jake Rose
	Contains SampleTiles and properly adjust depending on input search terms

  ==============================================================================
*/
#ifndef SAMPLECONTAINER_H
#define SAMPLECONTAINER_H

#include "JuceHeader.h"

#include "Sample.h"
#include "SampleTile.h"

namespace samplore
{
	class SampleContainer : public Component
	{
	public:
		//========================================================
		SampleContainer();
		~SampleContainer();

		void paint(Graphics&) override;
		void resized() override;

		void updateItems();
		void extendItems();
		void clearItems();

		void setSampleItems(Sample::List mSamples);
		//======================================================
		int calculateAllRowsHeight();
		int calculateRowCount();
		int calculateColumnCount();
	private:
		//=============================================================================
		std::vector<SampleTile*> mUsedSampleTiles;
		std::vector<SampleTile*> mUnusedSampleTiles;
		Sample::List mCurrentSamples;
		int mMaxItems = 20;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleContainer)
	};
}
#endif