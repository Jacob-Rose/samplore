/*
  ==============================================================================

	SamplifyMainComponent.h
	Author:  Jake Rose

	The main component of the application
  ==============================================================================
*/

#ifndef SAMPLIFYMAINCOMPONENT_H
#define SAMPLIFYMAINCOMPONENT_H

#include "JuceHeader.h"

#include "AudioPlayer.h"
#include "DirectoryExplorer.h"
#include "FilterExplorer.h"
#include "SamplePlayerComponent.h"
#include "SampleExplorer.h"
#include "ServerAuthUnlockComponent.h"
#include "KeyBindingManager.h"
#include "PreferenceWindow.h"
#include "ThemeManager.h"
#include "ImportWizard.h"
#include "SpliceImporter.h"
#include "SpliceImportDialog.h"
#include "PerformanceProfiler.h"

namespace samplore
{
	class SamplifyMainComponent : public AudioAppComponent, public KeyListener, public ChangeListener, private Timer, public ThemeManager::Listener
	{
	public:
		static SamplifyMainComponent* getInstance();
		//=====================================================
		SamplifyMainComponent();
		~SamplifyMainComponent();

		bool keyPressed(const KeyPress& key, Component* originatingComponent);
		void changeListenerCallback(ChangeBroadcaster* source);
		void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
		void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;
		void releaseResources() override;

		static void setupLookAndFeel(LookAndFeel& laf);

		//==============================================================================
		void paint(Graphics&) override;
		void resized() override;

		void timerCallback() override;

		void mouseDrag(const MouseEvent& e) override;


		DirectoryExplorer& getDirectoryExplorer() { return mDirectoryExplorer; }
		SampleExplorer& getSampleExplorer() { return mSampleExplorer; }
		FilterExplorer& getFilterExplorer() { return mFilterExplorer; }
		SamplePlayerComponent& getSamplePlayerComponent() { return mSamplePlayerComponent; }
	std::shared_ptr<AudioPlayer> getAudioPlayer() { return mAudioPlayer; }
	
	/// Shows the import wizard overlay
	void showImportWizard();
	
	/// Shows the preferences overlay
	void showPreferences();

	//==================================================================
	// ThemeManager::Listener interface
	void themeChanged(ThemeManager::Theme newTheme) override;
	void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;
		
	private:
		DirectoryExplorer mDirectoryExplorer;
		SampleExplorer mSampleExplorer;
		FilterExplorer mFilterExplorer;
		SamplePlayerComponent mSamplePlayerComponent;
		ResizableEdgeComponent mResizableEdgeDirectoryExplorer;
		ResizableEdgeComponent mResizableEdgeFilterExplorer;
		ResizableEdgeComponent mResizableEdgeAudioPlayer;
		ComponentBoundsConstrainer mResizableEdgeDirectoryExplorerBounds;
		ComponentBoundsConstrainer mResizableEdgeFilterExplorerBounds;
		ComponentBoundsConstrainer mResizableEdgeAudioPlayerBounds;

	std::shared_ptr<AudioPlayer> mAudioPlayer;
	juce::SharedResourcePointer<TooltipWindow> mTooltip;
	
	ImportWizard mImportWizard;
	PreferenceWindow mPreferenceWindow;
	SpliceImporter mSpliceImporter;
	SpliceImportDialog mSpliceImportDialog;

	static SamplifyMainComponent* mInstance;

		//ServerAuthStatus authorizationStatus;
		//ServerAuthUnlockComponent unlockForm;
		//bool isUnlocked = false;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplifyMainComponent)
	};
}
#endif
