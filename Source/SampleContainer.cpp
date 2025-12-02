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
	// Set total height based on all samples
	int totalHeight = calculateTotalHeight();
	setSize(getWidth(), totalHeight);
	
	// Update visible items (will be called by viewport)
	// For now, just update with current view (0, 0)
	updateVisibleItems(0, getParentHeight());
}

void SampleContainer::updateVisibleItems(int viewportTop, int viewportHeight)
{
	if (mCurrentSamples.size() == 0)
	{
		// Hide all tiles
		for (auto& tile : mTilePool)
		{
			tile->setVisible(false);
		}
		return;
	}
	
	int columns = getColumnCount();
	if (columns <= 0)
		return;
	
	int tileWidth = getTileWidth();
	int tileHeight = getTileHeight();
	int padding = AppValues::getInstance().SAMPLE_TILE_CONTAINER_ITEM_PADDING;
	
	// Calculate which rows are visible (with buffer for smooth scrolling)
	int firstVisibleRow = jmax(0, (viewportTop / tileHeight) - 1);
	int lastVisibleRow = jmin(getTotalRowCount() - 1, 
	                          ((viewportTop + viewportHeight) / tileHeight) + 1);
	
	// Calculate range of sample indices that are visible
	int firstVisibleIndex = firstVisibleRow * columns;
	int lastVisibleIndex = jmin((int)mCurrentSamples.size() - 1, 
	                            (lastVisibleRow + 1) * columns - 1);
	
	int visibleCount = lastVisibleIndex - firstVisibleIndex + 1;
	
	// Ensure we have enough tiles in the pool
	while ((int)mTilePool.size() < visibleCount)
	{
		auto tile = std::make_unique<SampleTile>(nullptr);
		addAndMakeVisible(tile.get());
		mTilePool.push_back(std::move(tile));
	}
	
	// Update visible tiles
	for (int i = 0; i < visibleCount && (firstVisibleIndex + i) < (int)mCurrentSamples.size(); i++)
	{
		int sampleIndex = firstVisibleIndex + i;
		int column = sampleIndex % columns;
		int row = sampleIndex / columns;
		
		SampleTile* tile = mTilePool[i].get();
		tile->setVisible(true);
		tile->setBounds((column * tileWidth) + padding,
		                (row * tileHeight) + padding,
		                tileWidth - (padding * 2),
		                tileHeight - (padding * 2));
		tile->setSample(mCurrentSamples[sampleIndex]);
	}
	
	// Hide unused tiles
	for (int i = visibleCount; i < (int)mTilePool.size(); i++)
	{
		mTilePool[i]->setVisible(false);
	}
	
	mLastViewportTop = viewportTop;
	mLastViewportHeight = viewportHeight;
}

void SampleContainer::clearItems()
{
	mTilePool.clear();
}

void SampleContainer::setSampleItems(Sample::List currentSamples)
{
	mCurrentSamples = currentSamples;
	
	// Recalculate total height based on all samples
	int totalHeight = calculateTotalHeight();
	setSize(getWidth(), totalHeight);
	
	// Update visible items
	updateVisibleItems(mLastViewportTop >= 0 ? mLastViewportTop : 0, 
	                   mLastViewportHeight >= 0 ? mLastViewportHeight : getParentHeight());
}

int SampleContainer::calculateTotalHeight() const
{
	int tileHeight = getTileHeight();
	int totalRows = getTotalRowCount();
	return tileHeight * totalRows;
}

int SampleContainer::getTotalRowCount() const
{
	int columns = getColumnCount();
	if (columns <= 0)
		return 0;
	
	// Calculate total rows needed for ALL samples
	return (mCurrentSamples.size() + columns - 1) / columns;  // Ceiling division
}

int SampleContainer::getColumnCount() const
{
	int minWidth = AppValues::getInstance().SAMPLE_TILE_MIN_WIDTH;
	if (minWidth <= 0)
		return 1;
	
	return jmax(1, getWidth() / minWidth);
}

int SampleContainer::getTileHeight() const
{
	int tileWidth = getTileWidth();
	return AppValues::getInstance().SAMPLE_TILE_ASPECT_RATIO * tileWidth;
}

int SampleContainer::getTileWidth() const
{
	int columns = getColumnCount();
	if (columns <= 0)
		return AppValues::getInstance().SAMPLE_TILE_MIN_WIDTH;
	
	return getWidth() / columns;
}
