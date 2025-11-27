#include "SampleLibrary.h"
#include "SamplifyMainComponent.h"

using namespace samplify;

SampleLibrary::SampleLibrary()
{
	
}

SampleLibrary::~SampleLibrary()
{
}

void SampleLibrary::updateCurrentSamples(String query)
{
	mCurrentQuery = query;

	if (mUpdatingSamples)
	{
		mCancelUpdating = true;
	}
	mUpdateSampleFuture = std::future<Sample::List>(getAllSamplesInDirectories_Async(query));
	mUpdatingSamples = true;
	//mCurrentSamples = getAllSamplesInDirectories(query, false);
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
			sendChangeMessage();
			return;
		}
	}
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

StringArray samplify::SampleLibrary::getUsedTags()
{
	StringArray tags;
	Sample::List allSamps = getAllSamplesInDirectories("", true);
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

void SampleLibrary::addTag(juce::String text, Colour color)
{
	mTags.push_back(Tag(text, color));
	sendChangeMessage();
}

void SampleLibrary::addTag(juce::String text)
{
	Random& r = Random::getSystemRandom();
	addTag(text, Colour(juce::uint8(r.nextInt(Range(0, 256))),
		juce::uint8(r.nextInt(Range(0, 256))),
		juce::uint8(r.nextInt(Range(0, 256)))));
}

void SampleLibrary::deleteTag(juce::String tag)
{
	Sample::List allSamps = getAllSamplesInDirectories("", true);
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
			return mTags[i].mColor;
		}
	}
	//if not in list, make and then return again
	addTag(tag);
	return getTagColor(tag);
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

void SampleLibrary::setTagColor(juce::String tag, juce::Colour newColor)
{
	for (int i = 0; i < mTags.size(); i++)
	{
		if (mTags[i].mTitle == tag)
		{
			mTags[i].mColor = newColor;
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
}

Sample::List SampleLibrary::getAllSamplesInDirectories(juce::String query, bool ignoreCheckSystem)
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


std::future<Sample::List> SampleLibrary::getAllSamplesInDirectories_Async(juce::String query, bool ignoreCheckSystem)
{
	std::future<Sample::List> asfunc = std::async(std::launch::async, &SampleLibrary::getAllSamplesInDirectories, this, query, ignoreCheckSystem);
	startTimer(300);
	return asfunc;
}