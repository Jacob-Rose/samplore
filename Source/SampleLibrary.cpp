#include "SampleLibrary.h"
#include "SamplifyMainComponent.h"

using namespace samplore;

SampleLibrary::SampleLibrary()
{
	
}

SampleLibrary::~SampleLibrary()
{
	// Remove ourselves as a listener from all directories before destruction
	for (auto& dir : mDirectories)
	{
		if (dir)
		{
			dir->removeChangeListener(this);
		}
	}
	
	// Explicitly clear all directories to release Sample objects
	mDirectories.clear();
}

void SampleLibrary::updateCurrentSamples(const FilterQuery& query)
{
	mCurrentQuery = query;

	if (mUpdatingSamples)
	{
		mCancelUpdating = true;
	}
	mUpdateSampleFuture = std::future<Sample::List>(getAllSamplesInDirectories_Async(query));
	mUpdatingSamples = true;
	sendChangeMessage();
}

void SampleLibrary::sortSamples(SortingMethod method)
{
	mCurrentSamples.sort(method);
	sendChangeMessage();
}

void SampleLibrary::addDirectory(const File& dir)
{
	// Check if directory already exists
	for (const auto& existingDir : mDirectories)
	{
		if (existingDir->getFile() == dir)
		{
			// Directory already added, skip
			return;
		}
	}
	
	std::shared_ptr<SampleDirectory> sampDir = std::make_shared<SampleDirectory>(dir);
	sampDir->addChangeListener(this);
	mDirectories.push_back(sampDir);
	
	// Rescan all samples to include new directory
	refreshCurrentSamples();
	
	// Preload all tags from all samples asynchronously
	launchPreloadAllTags();
	
	sendChangeMessage();
}

void SampleLibrary::removeDirectory(const File& dir)
{
	for (auto it = mDirectories.begin(); it != mDirectories.end(); ++it)
	{
		if ((*it)->getFile() == dir)
		{
			(*it)->removeChangeListener(this);
			mDirectories.erase(it);
			
			// Rescan all samples to remove samples from deleted directory
			refreshCurrentSamples();
			sendChangeMessage();
			return;
		}
	}
}


void SampleLibrary::refreshDirectories()
{
	for (auto& dir : mDirectories)
	{
		dir->rescanFiles();
	}
	refreshCurrentSamples();
}
File SampleLibrary::getRelativeDirectoryForFile(const File& sampleFile) const
{
	for (int i = 0; i < mDirectories.size(); i++)
	{
		if (sampleFile.isAChildOf(mDirectories[i]->getFile()))
		{
			return sampleFile.getRelativePathFrom(mDirectories[i]->getFile());
		}
	}
	return File();
}

void SampleLibrary::changeListenerCallback(ChangeBroadcaster* source)
{
	refreshCurrentSamples();
}

Sample::List SampleLibrary::getCurrentSamples()
{
	return mCurrentSamples;
}

StringArray samplore::SampleLibrary::getUsedTags()
{
	StringArray tags;
	Sample::List allSamps = getAllSamplesInDirectories({}, true);
	for (int i = 0; i < allSamps.size(); i++)
	{
		StringArray sampleTags = allSamps[i].getTags();
		for (int j = 0; j < sampleTags.size(); j++)
		{
			tags.addIfNotAlreadyThere(sampleTags[j]);
		}
	}
	return tags;
}

void SampleLibrary::timerCallback()
{
	if (mUpdateSampleFuture.valid() && mUpdatingSamples && !mCancelUpdating)
	{
		mCurrentSamples = mUpdateSampleFuture.get();
		stopTimer();
		mUpdatingSamples = false;
		sendChangeMessage();
	}
	else
	{
		stopTimer();
	}
	
}

Colour SampleLibrary::hueToColor(float hue)
{
	// Use fixed saturation and brightness for consistent, vibrant tag colors
	const float saturation = 0.45f;
	const float brightness = 0.75f;
	return Colour::fromHSV(hue, saturation, brightness, 1.0f);
}

void SampleLibrary::addTag(juce::String text, float hue)
{
	mTags.push_back(Tag(text, hue));
	sendChangeMessage();
}

void SampleLibrary::addTag(juce::String text)
{
	Random& r = Random::getSystemRandom();
	// Generate random hue (0.0-1.0)
	float hue = r.nextFloat();
	mTags.push_back(Tag(text, hue));
	sendChangeMessage();
}

void SampleLibrary::deleteTag(juce::String tag)
{
	Sample::List allSamps = getAllSamplesInDirectories({}, true);
	for (int i = 0; i < allSamps.size(); i++)
	{
		allSamps[i].removeTag(tag); //remove if exist
		break;
	}
	for (int i = 0; i < mTags.size(); i++)
	{
		if (mTags[i].mTitle == tag)
		{
			mTags.erase(mTags.begin() + i);
			break;
		}
		//todo
		//SamplifyMainComponent::getInstance()->getFilterExplorer().getTagExplorer().resized();
	}
}

Colour SampleLibrary::getTagColor(String tag)
{
	for (int i = 0; i < mTags.size(); i++)
	{
		if (mTags[i].mTitle == tag)
		{
			return hueToColor(mTags[i].mHue);
		}
	}
	//if not in list, make and then return again
	addTag(tag);
	return getTagColor(tag);
}

float SampleLibrary::getTagHue(String tag)
{
	for (int i = 0; i < mTags.size(); i++)
	{
		if (mTags[i].mTitle == tag)
		{
			return mTags[i].mHue;
		}
	}
	//if not in list, make and then return again
	addTag(tag);
	return getTagHue(tag);
}

StringArray SampleLibrary::getTagsStringArray()
{
	StringArray tags = StringArray();
	for (int i = 0; i < mTags.size(); i++)
	{
		tags.add(mTags[i].mTitle);
	}
	return tags;
}

void SampleLibrary::setTagHue(juce::String tag, float hue)
{
	for (int i = 0; i < mTags.size(); i++)
	{
		if (mTags[i].mTitle == tag)
		{
			mTags[i].mHue = hue;
			sendChangeMessage();
			return;
		}
	}
}

SampleLibrary::Tag SampleLibrary::getTag(juce::String tag)
{
	for (int i = 0; i < mTags.size(); i++)
	{
		if (mTags[i].mTitle == tag)
		{
			return mTags[i];
		}
	}
	return SampleLibrary::Tag::getEmptyTag();
}

Sample::List SampleLibrary::getAllSamplesInDirectories(const FilterQuery& query, bool ignoreCheckSystem)
{
	Sample::List list;
	for (int i = 0; i < mDirectories.size(); i++)
	{
		list += mDirectories[i]->getChildSamplesRecursive(query, ignoreCheckSystem);
		if (mCancelUpdating)
		{
			mCancelUpdating = false;
			return list;
		}
	}
	return list;
}


std::future<Sample::List> SampleLibrary::getAllSamplesInDirectories_Async(const FilterQuery& query, bool ignoreCheckSystem)
{
	std::future<Sample::List> asfunc = std::async(std::launch::async, &SampleLibrary::getAllSamplesInDirectories, this, query, ignoreCheckSystem);
	startTimer(300);
	return asfunc;
}

void SampleLibrary::launchPreloadAllTags()
{
	if (!mPreloadingTags)
	{
		mPreloadingTags = true;
		mPreloadTagsFuture = std::async(std::launch::async, &SampleLibrary::preloadTags_Worker, this);
	}
}

void SampleLibrary::preloadTags_Worker()
{
	DBG("Starting tag preload from all sample files...");

	// Get all samples without filtering (empty query, ignore check system)
	Sample::List allSamples = getAllSamplesInDirectories({}, true);
	
	// Iterate through all samples and load their properties/tags
	int sampleCount = allSamples.size();
	int processedCount = 0;
	
	for (int i = 0; i < sampleCount; i++)
	{
		Sample::Reference sampleRef = allSamples[i];
		if (!sampleRef.isNull())
		{
			// Access tags which will trigger loading from properties file
			StringArray tags = sampleRef.getTags();
			
			// Add any new tags to the library with auto-generated colors
			for (const auto& tag : tags)
			{
				// Check if tag already exists in mTags
				bool tagExists = false;
				for (const auto& existingTag : mTags)
				{
					if (existingTag.mTitle == tag)
					{
						tagExists = true;
						break;
					}
				}
				
				// If tag doesn't exist, add it (this will auto-generate a color)
				if (!tagExists && tag.isNotEmpty())
				{
					addTag(tag);
				}
			}
			
			processedCount++;
			
			// Log progress every 100 samples
			if (processedCount % 100 == 0)
			{
				DBG("Preloaded tags from " + String(processedCount) + "/" + String(sampleCount) + " samples");
			}
		}
	}
	
	DBG("Tag preload complete! Processed " + String(processedCount) + " samples");
	mPreloadingTags = false;
	
	// Notify listeners that tags have been updated
	sendChangeMessage();
}