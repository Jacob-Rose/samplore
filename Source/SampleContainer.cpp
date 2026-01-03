#include "SampleContainer.h"
#include "SampleLibrary.h"
#include "SamplifyProperties.h"
#include "SamplifyLookAndFeel.h"
#include "PerformanceProfiler.h"

#include <algorithm>
#include <cmath>

using namespace samplore;

SampleContainer::SampleContainer()
{
	// Register with SampleLibrary for thumbnail ready notifications
	if (auto library = SamplifyProperties::getInstance()->getSampleLibrary())
	{
		library->addRequestProvider(this);
	}
}

SampleContainer::~SampleContainer()
{
	// Unregister from SampleLibrary
	if (auto* props = SamplifyProperties::getInstance())
	{
		if (auto library = props->getSampleLibrary())
		{
			library->removeRequestProvider(this);
		}
	}
	clearItems();
}

void SampleContainer::retryVisibleThumbnails()
{
	// Retry thumbnail generation for visible samples that are still missing thumbnails
	if (mLastViewportTop < 0 || mLastViewportHeight <= 0)
		return;

	int columns = getColumnCount();
	if (columns <= 0)
		return;

	int tileHeight = getTileHeight();
	if (tileHeight <= 0)
		return;

	int firstVisibleRow = jmax(0, (mLastViewportTop / tileHeight) - 1);
	int lastVisibleRow = jmin(getTotalRowCount() - 1,
	                          ((mLastViewportTop + mLastViewportHeight) / tileHeight) + 1);

	int firstVisibleIndex = firstVisibleRow * columns;
	int lastVisibleIndex = jmin((int)mCurrentSamples.size() - 1,
	                            (lastVisibleRow + 1) * columns - 1);

	for (int i = firstVisibleIndex; i <= lastVisibleIndex && i < (int)mCurrentSamples.size(); i++)
	{
		Sample::Reference sample = mCurrentSamples[i];
		if (!sample.isNull())
		{
			auto thumbnail = sample.getThumbnail();
			if (thumbnail == nullptr)
			{
				sample.generateThumbnailAndCache();
				break; // Only start one at a time to respect throttling
			}
		}
	}
}

void SampleContainer::paint (Graphics& g)
{
	
}

void SampleContainer::resized()
{
	// Set total height based on all samples
	int totalHeight = calculateTotalHeight();
	setSize(getWidth(), totalHeight);
	
	// Pre-allocate tile pool when size changes (e.g., window resize)
	preallocateTilePool();
	
	// Update visible items (will be called by viewport)
	// For now, just update with current view (0, 0)
	updateVisibleItems(0, getParentHeight());
}

void SampleContainer::updateVisibleItems(int viewportTop, int viewportHeight)
{
	PROFILE_SCOPE("SampleContainer::updateVisibleItems");

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

	// Note: Tile pool should already be pre-allocated by preallocateTilePool()
	// This is a safety check in case pool wasn't allocated yet
	if ((int)mTilePool.size() < visibleCount)
	{
		DBG("WARNING: Tile pool not pre-allocated, allocating during scroll (this should not happen)");
		preallocateTilePool();
	}

	// Mark all tiles as potentially unused (we'll mark used ones below)
	static std::vector<bool> tileUsedThisFrame;
	tileUsedThisFrame.clear();
	tileUsedThisFrame.resize(mTilePool.size(), false);

	// Update visible tiles using MODULO INDEXING for stable tile reuse
	int boundsUpdates = 0;
	int sampleUpdates = 0;

	for (int i = 0; i < visibleCount && (firstVisibleIndex + i) < (int)mCurrentSamples.size(); i++)
	{
		int sampleIndex = firstVisibleIndex + i;
		int column = sampleIndex % columns;
		int row = sampleIndex / columns;

		// KEY: Use modulo so sample[0]->pool[0], sample[1]->pool[1], sample[128]->pool[0], etc.
		// This way tiles maintain stable sample mappings as we scroll
		int poolIndex = sampleIndex % (int)mTilePool.size();
		SampleTile* tile = mTilePool[poolIndex].get();

		tileUsedThisFrame[poolIndex] = true;

		if (!tile->isVisible())
			tile->setVisible(true);

		// Only update bounds if changed (avoids unnecessary resized() calls)
		Rectangle<int> newBounds((column * tileWidth) + padding,
		                         (row * tileHeight) + padding,
		                         tileWidth - (padding * 2),
		                         tileHeight - (padding * 2));
		if (tile->getBounds() != newBounds)
		{
			tile->setBounds(newBounds);
			boundsUpdates++;
		}

		// Only update sample if changed (avoids redundant thumbnail generation and repaints)
		Sample::Reference newSample = mCurrentSamples[sampleIndex];
		if (tile->getSample() != newSample)
		{
			tile->setSample(newSample);
			sampleUpdates++;
		}
	}

	// Hide tiles that weren't used this frame
	for (size_t i = 0; i < mTilePool.size(); i++)
	{
		if (!tileUsedThisFrame[i] && mTilePool[i]->isVisible())
		{
			mTilePool[i]->setVisible(false);
		}
	}

	// Retry thumbnail generation for visible tiles that are missing thumbnails
	// This handles cases where thumbnails were skipped due to throttling
	for (int i = 0; i < visibleCount && (firstVisibleIndex + i) < (int)mCurrentSamples.size(); i++)
	{
		int sampleIndex = firstVisibleIndex + i;
		Sample::Reference sample = mCurrentSamples[sampleIndex];
		if (!sample.isNull())
		{
			auto thumbnail = sample.getThumbnail();
			if (thumbnail == nullptr)
			{
				// This sample needs a thumbnail - retry generation
				sample.generateThumbnailAndCache();
			}
		}
	}

	// Debug trace for understanding update patterns
	DBG("SampleContainer: " << boundsUpdates << " bounds, "
	    << sampleUpdates << " samples | viewportTop=" << viewportTop
	    << " delta=" << (viewportTop - mLastViewportTop));

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
	
	// Pre-allocate tile pool for new sample list
	preallocateTilePool();
	
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

void SampleContainer::preallocateTilePool()
{
	int columns = getColumnCount();
	if (columns <= 0)
		return;
	
	int tileHeight = getTileHeight();
	if (tileHeight <= 0)
		return;
	
	// Get viewport height
	int viewportHeight = getParentHeight();
	if (viewportHeight <= 0)
		viewportHeight = 800;  // Default fallback
	
	// Calculate visible rows in viewport
	int visibleRows = (viewportHeight / tileHeight) + 1;  // +1 for partial row
	
	// Pre-allocate 4x the visible tiles for smooth scrolling
	// Tiles are REUSED and repositioned as user scrolls (virtual scrolling)
	// We just need enough in the pool to cover scroll buffer without mid-scroll allocation
	int tilesNeeded = visibleRows * columns * 4;
	
	// Cap at total samples if library is small
	if (mCurrentSamples.size() > 0)
	{
		tilesNeeded = jmin(tilesNeeded, (int)mCurrentSamples.size());
	}
	
	// Pre-allocate tiles if pool is smaller than needed
	int tilesAdded = 0;
	while ((int)mTilePool.size() < tilesNeeded)
	{
		auto tile = std::make_unique<SampleTile>(nullptr);
		addAndMakeVisible(tile.get());
		mTilePool.push_back(std::move(tile));
		tilesAdded++;
	}
	
	if (tilesAdded > 0)
	{
		DBG("Tile pool pre-allocated: added " << tilesAdded << " tiles, "
		    << "total: " << mTilePool.size() << " tiles "
		    << "(4x visible rows for tile reuse during scroll)");
	}
}
