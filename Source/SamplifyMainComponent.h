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
#include "UI/LeftPanelTabs.h"
#include "FilterExplorer.h"
#include "SamplePlayerComponent.h"
#include "SampleExplorer.h"
#include "ServerAuthUnlockComponent.h"
#include "KeyBindingManager.h"
#include "InputContext.h"
#include "PreferenceWindow.h"
#include "ThemeManager.h"
#include "ImportWizard.h"
#include "PerformanceProfiler.h"
#include "UI/OverlayPanel.h"
#include "UI/CueBindingsWindow.h"
#include "UI/KeyCaptureOverlay.h"

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


		DirectoryExplorer& getDirectoryExplorer() { return mLeftPanel.getDirectoryExplorer(); }
		LeftPanelTabs& getLeftPanel() { return mLeftPanel; }
		SampleExplorer& getSampleExplorer() { return mSampleExplorer; }
		FilterExplorer& getFilterExplorer() { return mFilterExplorer; }
		SamplePlayerComponent& getSamplePlayerComponent() { return mSamplePlayerComponent; }
		std::shared_ptr<AudioPlayer> getAudioPlayer() { return mAudioPlayer; }
	
		/// Shows the import wizard overlay
		void showImportWizard();

		/// Shows the preferences overlay
		void showPreferences();

		/// Shows/toggles the cue bindings window
		void showCueBindingsWindow();

		/// Shows the key capture overlay for binding a key to current sample
		void showKeyCaptureOverlay();

		//==================================================================
		// ThemeManager::Listener interface
		void themeChanged(ThemeManager::Theme newTheme) override;
		void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;
		
	private:
		LeftPanelTabs mLeftPanel;
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
		
		// Central overlay panel for modal views
		OverlayPanel mOverlayPanel;
		
		// Content views for overlay
		ImportWizard mImportWizard;
		PreferencePanel mPreferencePanel;
		KeyCaptureOverlay mKeyCaptureOverlay;

		// Floating cue bindings window
		std::unique_ptr<CueBindingsWindow> mCueBindingsWindow;

		/// Register callbacks for global key bindings with KeyBindingManager
		void registerKeyBindingCallbacks();

		static SamplifyMainComponent* mInstance;

			//ServerAuthStatus authorizationStatus;
			//ServerAuthUnlockComponent unlockForm;
			//bool isUnlocked = false;

			JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplifyMainComponent)
		};
}
#endif
