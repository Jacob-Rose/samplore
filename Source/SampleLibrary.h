/*
  ==============================================================================

    SampleLibrary.h
    Created: 31 May 2018 12:45:49pm
    Author:  Jake Rose
	Summary: This is the core system Samplify uses to load samples, and basically offers an interface for SamplifyProperties to access. 
	This way, this all is independent from SamplifyProperties and the actual execution
  ==============================================================================
*/
#ifndef SAMPLELIBRARY_H
#define SAMPLELIBRARY_H

#include "JuceHeader.h"

#include "SampleDirectory.h"

#include <vector>
#include <future>
#include <algorithm>

namespace samplore
{
	/// Interface for components that can provide visible samples for thumbnail retry
	class ISampleRequestProvider
	{
	public:
		virtual ~ISampleRequestProvider() = default;

		/// Called when a thumbnail finishes loading - providers should retry visible samples
		virtual void retryVisibleThumbnails() = 0;
	};

	/// Filter criteria for sample searches
	struct FilterQuery
	{
		String searchText;      // Matches against filename/path
		StringArray tags;       // All tags must be present on sample

		bool isEmpty() const { return searchText.isEmpty() && tags.isEmpty(); }
		void clear() { searchText.clear(); tags.clear(); }
	};

	class SampleLibrary : public ChangeBroadcaster, public ChangeListener, public Timer
	{
	public:

	struct Tag
	{
		// Use function-local static to avoid static destruction order issues
		static const Tag& getEmptyTag()
		{
			static const Tag emptyTag(juce::String(), 0.83f); // Magenta hue
			return emptyTag;
		}

		/// Tag stores just a hue (0.0-1.0) for theme-consistent colors
		Tag(juce::String title, float hue) : mTitle(title), mHue(hue) {}

		juce::String mTitle;
		float mHue; // 0.0-1.0, used with fixed saturation/brightness
	};

		SampleLibrary();
		~SampleLibrary();

		void refreshCurrentSamples() { updateCurrentSamples(mCurrentQuery); }
		void updateCurrentSamples(const FilterQuery& query);

		void sortSamples(SortingMethod method);

		Sample::List getCurrentSamples();
		const FilterQuery& getCurrentQuery() { return mCurrentQuery; }

		StringArray getUsedTags(); //get tags that are currently connected to one or more samples

		void timerCallback() override;

		///Tag Library Merger - They are dependent on each other for results and modifications
		void addTag(String tag, float hue);
		void addTag(String tag);
		//void renameTag(juce::String currentTagName, juce::String desiredName);
		void deleteTag(String tag);
		int getTagCount() { return mTags.size(); }
		Colour getTagColor(String tag);
		float getTagHue(String tag);
		std::vector<Tag> getTags() { return mTags; }
		StringArray getTagsStringArray();

		void setTagHue(juce::String tag, float hue);
		SampleLibrary::Tag getTag(juce::String tag);

		/// Converts a hue to a display color with theme-appropriate saturation/brightness
		static Colour hueToColor(float hue);


		///Directory Manager Merger - Reduce dependencies, less pointers, easier saving
		void addDirectory(const File& dir);
		const std::vector<std::shared_ptr<SampleDirectory>>& getDirectories() const { return mDirectories; }
		void removeDirectory(const File& dir);
		void refreshDirectories();
		int getDirectoryCount() { return mDirectories.size(); }

		File getRelativeDirectoryForFile(const File& sampleFile) const;

		void changeListenerCallback(ChangeBroadcaster* source) override;

		bool isAsyncValid() { return mUpdateSampleFuture.valid(); }

		//Get Samples
		Sample::List getAllSamplesInDirectories(const FilterQuery& query, bool ignoreCheckSystem);
		std::future<Sample::List> getAllSamplesInDirectories_Async(const FilterQuery& query = {}, bool ignoreCheckSystem = false);

		/// Find a sample by its file path (for cue binding restoration)
		Sample::Reference findSampleByFile(const juce::File& file);

		/// Preload all sample files and extract their tags asynchronously
		void launchPreloadAllTags();
		bool isPreloadingTags() const { return mPreloadingTags; }

		//======================================================================
		// Sample request providers (for thumbnail retry)
		void addRequestProvider(ISampleRequestProvider* provider);
		void removeRequestProvider(ISampleRequestProvider* provider);

		/// Called when a thumbnail finishes loading - notifies all providers to retry
		void notifyThumbnailReady();

	private:
		void preloadTags_Worker();
		
		std::future<Sample::List> mUpdateSampleFuture;
		std::future<void> mPreloadTagsFuture;
		bool mUpdatingSamples = false;
		bool mCancelUpdating = false;
		bool mPreloadingTags = false;
		Sample::List mCurrentSamples;
		FilterQuery mCurrentQuery;

		std::vector<Tag> mTags;
		//pointer necessary to keep the check system
		std::vector<std::shared_ptr<SampleDirectory>> mDirectories = std::vector<std::shared_ptr<SampleDirectory>>();

		/// Registered providers for thumbnail retry notifications
		std::vector<ISampleRequestProvider*> mRequestProviders;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleLibrary)
	};
}
#endif