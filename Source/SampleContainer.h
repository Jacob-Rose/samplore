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
#include "SampleLibrary.h"

namespace samplore
{
	// TODO rename SampleTileScrollView
	class SampleContainer : public Component, public ISampleRequestProvider
	{
	public:
		//========================================================
		SampleContainer();
		~SampleContainer();

		void paint(Graphics&) override;
		void resized() override;

		void updateVisibleItems(int viewportTop, int viewportHeight);
		void clearItems();

		void setSampleItems(Sample::List mSamples);

		//======================================================================
		// ISampleRequestProvider interface
		void retryVisibleThumbnails() override;

		//======================================================
		int calculateTotalHeight() const;
		int getTotalRowCount() const;
		int getColumnCount() const;
		int getTileHeight() const;
		int getTileWidth() const;
	private:
		/// Pre-allocate tile pool to avoid mid-scroll allocations
		void preallocateTilePool();
		//=============================================================================
		/// Pool of reusable SampleTile objects
		std::vector<std::unique_ptr<SampleTile>> mTilePool;
		/// All samples (full list)
		Sample::List mCurrentSamples;
		/// Current viewport position for optimization
		int mLastViewportTop = -1;
		int mLastViewportHeight = -1;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleContainer)
	};
}
#endif