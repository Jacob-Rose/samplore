/*
  ==============================================================================

    Sample.h
    Created: 31 May 2018 12:46:00pm
    Author:  Jake Rose

  ==============================================================================
*/

#ifndef SAMPLE_H
#define SAMPLE_H

#include "JuceHeader.h"

#include "SampleAudioThumbnail.h"
#include "SortingMethod.h"

namespace samplore
{
	struct FilterQuery; // Forward declaration

	class Sample : public ChangeBroadcaster, public ChangeListener
	{
	public:
		/// <summary>
		/// Clean pointer of Sample for easy passoff
		/// </summary>
		class Reference
		{
		public:
			Reference(std::shared_ptr<Sample> sample);
			Reference(nullptr_t null);
			Reference(const Sample::Reference& ref);
			
			bool isNull() const { return mSample.expired(); }

			std::shared_ptr<SampleAudioThumbnail> getThumbnail() const;

			File getFile() const;

			StringArray getRelativeParentFolders() const;

			//String getRelativePathName() const;
			
			String getInfoText() const;
			void setInfoText(String newText) const;

			void setColor(Colour newColor);

			Colour getColor() const;

			double getLength() const;

			StringArray getTags() const;
			void addTag(juce::String tag);
			void removeTag(juce::String tag);
			
			bool isPropertiesFileValid() const;
			void generateThumbnailAndCache();
			float getValueForSortType(SortingMethod method) const { return mSample.lock()->getValueForSortType(method); }
		
			void addChangeListener(ChangeListener* listener);
			void removeChangeListener(ChangeListener* listener);

			void renameFile(String name);

			friend bool operator==(const Sample::Reference& lhs, const Sample::Reference& rhs);
			friend bool operator!=(const Sample::Reference& lhs, const Sample::Reference& rhs);
		private:
			std::weak_ptr<Sample> mSample; //weak pointer for safety
			JUCE_LEAK_DETECTOR(Sample::Reference)
		};
		class List
		{
		public:
			List(const std::vector<Sample::Reference>& list);
			List();
			
			void addSample(const Sample::Reference sample);
			void addSamples(const Sample::List& list);
			void addSamples(const std::vector<Sample::Reference>& samples);

			void removeSample(Sample::Reference sample);
			void removeSample(int index);
			void removeSamples(std::vector<Sample::Reference> samples);
			void removeSamples(const Sample::List& list);

			void clear();
			int size() const;

			void randomize();
			void sort(SortingMethod method);
			void quickSort(SortingMethod method, int low, int high);
			int partition(SortingMethod method, int low, int high);


			void operator+=(const Sample::List& toAdd);
			Sample::Reference operator[](int index) const;
			friend Sample::List operator+(const Sample::List& lhs, const Sample::List& rhs);
			void operator=(const Sample::List& other);
		protected:
			std::vector<Sample::Reference> mSamples;
			SortingMethod mListSortingMethod = SortingMethod::None;
			JUCE_LEAK_DETECTOR(List)
		};

		Sample(const File&);
		~Sample();

		void changeListenerCallback(ChangeBroadcaster* source);
		ChangeListener* getChangeListener() { return this; }
		void savePropertiesFile();
		void loadPropertiesFile();

		float getValueForSortType(SortingMethod method) const;
		/*Checks if file both exist and has same or older version number*/
		bool isPropertiesFileValid();
		bool isQueryValid(const FilterQuery& query); //used in search
		static PropertiesFile* getPropertiesFile(const File& sampleFile);
	private:
		File mFile;
		std::unique_ptr<PropertiesFile> mPropertiesFile = nullptr;
		StringArray mTags;
		//std::map<juce::String, double> mCuePoints;
		juce::String mInformationDescription;
		double mLength = -1;
		std::shared_ptr<AudioThumbnailCache> mThumbnailCache = nullptr;
		std::shared_ptr<SampleAudioThumbnail> mThumbnail = nullptr;
		juce::Colour mColor; //saved with sample, the sampletile core color
		int mUseCount; //count number of times dragged into the program
		bool mUserHidden; //todo
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Sample)
	};

	inline bool operator==(const Sample::Reference& lhs, const Sample::Reference& rhs)
	{
		if (!lhs.isNull() && !rhs.isNull())
		{
			return lhs.getFile() == rhs.getFile();
		}
		else
		{
			if (lhs.isNull())
			{
				if (rhs.isNull())
				{
					return true;
				}
				return false;
			}
			else
			{
				//then rhs is null
				return true;
			}
		}

	}
	inline bool operator!=(const Sample::Reference& lhs, const Sample::Reference& rhs)
	{
		return !(lhs==rhs);
	}
	
	inline Sample::List operator+(const Sample::List& lhs, const Sample::List& rhs)
	{
		Sample::List combined;
		combined.addSamples(lhs);
		combined.addSamples(rhs);
		return combined;
	}
	
}
#endif