/*
  ==============================================================================

    AppProperties.h
    Created: 31 May 2018 3:16:14pm
    Author:  Jake Rose

	extends JUCE::ApplicationProperties and makes it a singleton with added features for my needs
  ==============================================================================
*/

#ifndef SAMPLIFYPROPERTIES_H
#define SAMPLIFYPROPERTIES_H

#include "JuceHeader.h"

#include "AudioPlayer.h"
#include "SampleLibrary.h"

namespace samplore
{
	class SamplifyProperties : public ApplicationProperties, public ChangeListener
	{
	public:
		//=Instance Handling========================================
		static void initInstance();
		static void cleanupInstance();
		static SamplifyProperties* getInstance();
		//=Init/Cleanup============================================
		void init();
		void cleanup();
		//=Saving=================================================
		void loadPropertiesFile();
		void savePropertiesFile();

		void browseForDirectory(std::function<void(const File&)> callback);
		void changeListenerCallback(ChangeBroadcaster* source);
		//=======================================================
		void setAudioPlayer(std::shared_ptr<AudioPlayer> ap) { mAudioPlayer = ap; }

		std::shared_ptr<AudioPlayer> getAudioPlayer() { return mAudioPlayer; }
		std::shared_ptr<SampleLibrary> getSampleLibrary() { return mSampleLibrary; }
	private:
		//========================================================
		SamplifyProperties();
		~SamplifyProperties();

		std::shared_ptr<SampleLibrary> mSampleLibrary = nullptr;

		std::shared_ptr<AudioPlayer> mAudioPlayer = nullptr;
		std::unique_ptr<FileChooser> mFileChooser;
		//========================================================
		static SamplifyProperties* smAppProperties;
		bool mIsInit = false;

		JUCE_LEAK_DETECTOR(SamplifyProperties)
	};
}

#endif