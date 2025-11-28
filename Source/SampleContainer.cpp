#include "SampleContainer.h"
#include "SampleLibrary.h"
#include "SamplifyProperties.h"
#include "SamplifyLookAndFeel.h"

#include <algorithm>
#include <cmath>

using namespace samplore;

SampleContainer::SampleContainer()
{

}

SampleContainer::~SampleContainer()
{
	clearItems();
}

void SampleContainer::paint (Graphics& g)
{
	
}

void SampleContainer::resized()
{
	updateItems();
}

void SampleContainer::updateItems()
{
	int columns = calculateColumnCount();
	//first, if there are too many then move them
	if (mUsedSampleTiles.size() > mCurrentSamples.size())
	{
		for (int i = mCurrentSamples.size(); i < mUsedSampleTiles.size(); i++)
		{
			mUsedSampleTiles[i]->setSample(nullptr);
			mUnusedSampleTiles.push_back(mUsedSampleTiles[i]);
			mUsedSampleTiles.erase(mUsedSampleTiles.begin() + i);
			i--;
		}
	}
	if (columns > 0)
	{
		int width = getWidth() / columns;
		int height = AppValues::getInstance().SAMPLE_TILE_ASPECT_RATIO * width;
		for (unsigned int i = 0; i < mCurrentSamples.size() && i < mMaxItems; i++)
		{
			int column = i % columns;
			int row = i / columns; //will cut off, not round (i feel like a real coder)

			SampleTile* tile;
			if (i < mUsedSampleTiles.size())
			{
				tile = mUsedSampleTiles[i];
			}
			else
			{
				tile = new SampleTile(mCurrentSamples[i]);
				mUsedSampleTiles.push_back(tile);
				addAndMakeVisible(tile);
			}
			tile->setBounds((column * width) + AppValues::getInstance().SAMPLE_TILE_CONTAINER_ITEM_PADDING,
								(row * height) + AppValues::getInstance().SAMPLE_TILE_CONTAINER_ITEM_PADDING,
								width - (AppValues::getInstance().SAMPLE_TILE_CONTAINER_ITEM_PADDING * 2),
								height - (AppValues::getInstance().SAMPLE_TILE_CONTAINER_ITEM_PADDING * 2));
			tile->setSample(mCurrentSamples[i]);
		}

		setBounds(Rectangle<int>(0, 0, calculateColumnCount() * width, calculateRowCount() * height));
	}
}

void SampleContainer::clearItems()
{
	for (unsigned int i = 0; i < mUsedSampleTiles.size(); i++)
	{
		delete mUsedSampleTiles[i];
		mUsedSampleTiles[i] = nullptr;
	}

	for (unsigned int i = 0; i < mUnusedSampleTiles.size(); i++)
	{
		delete mUnusedSampleTiles[i];
		mUnusedSampleTiles[i] = nullptr;
	}
	mUsedSampleTiles.clear();
}

void SampleContainer::setSampleItems(Sample::List currentSamples)
{
	Sample::List oldSamples = mCurrentSamples;
	mCurrentSamples = currentSamples;
	/*
	bool same = true;
	for (int i = 0; i < oldSamples.getCount() && i < mCurrentSamples.getCount(); i++)
	{
		same = same && mCurrentSamples.getSamples()[i] == oldSamples.getSamples()[i];
	}
	
	if (!same)
	{*/
		//updateItems();
	//}
	//todo causing threading issue? causes error in xmemory0
	updateItems();
}

int SampleContainer::calculateAllRowsHeight()
{
	int columns = calculateColumnCount();
	float height;
	if (mUsedSampleTiles.size() > 0)
	{
		height = mUsedSampleTiles[0]->getHeight();
	}
	else
	{
		height = 0;
	}
	return height * calculateRowCount();
}

int SampleContainer::calculateRowCount()
{
	int columns = calculateColumnCount();
	if (columns > 0)
	{
		return mUsedSampleTiles.size() / columns;
	}
	return 0;
}

int SampleContainer::calculateColumnCount()
{
	return getWidth() / AppValues::getInstance().SAMPLE_TILE_MIN_WIDTH;
}

void SampleContainer::extendItems()
{
	if (mCurrentSamples.size() > mMaxItems)
	{
		mMaxItems += calculateColumnCount() * 3;
		updateItems();
	}
}
