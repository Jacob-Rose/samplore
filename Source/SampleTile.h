/*
  ==============================================================================

    SampleTile.h
    Created: 31 May 2018 1:20:01pm
    Author:  Jake Rose

  ==============================================================================
*/

#ifndef SAMPLETILE_H
#define SAMPLETILE_H

#include "JuceHeader.h"

#include "Sample.h"
#include "TagContainer.h"
#include "SamplifyProperties.h"
#include "Animation/AnimationManager.h"
#include "ThemeManager.h"

namespace samplore
{

	class SampleTile : public Component,
		public DragAndDropTarget,
		public ChangeListener,
		private AnimatedComponent,
		public ThemeManager::Listener
	{
	public:

		enum ColourIds
		{
			foregroundHoverColorID = 720,
		    foregroundDefaultColorID,
		    backgroundDefaultColorID,
			backgroundHoverColorID
		};

		enum class RightClickOptions
		{
			openExplorer = 1,
			renameSample,
			deleteSample,
			addTriggerKeyAtStart,
			addTriggerKeyAtCue
		};

		//===========================================================================
		SampleTile(Sample::Reference);
		~SampleTile();

		void paint(Graphics&) override;
		void resized() override;
		//===========================================================================
		void mouseUp(const MouseEvent& e) override;
		void mouseDrag(const MouseEvent& e) override;
		void mouseExit(const MouseEvent& e) override;

		bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
		void itemDropped(const SourceDetails& dragSourceDetails) override;
		void itemDragEnter(const SourceDetails& dragSourceDetails) override;
		void itemDragExit(const SourceDetails& dragSourceDetails) override;
		void changeListenerCallback(ChangeBroadcaster* source) override;

		void setSample(Sample::Reference);
		Sample::Reference getSample();
		void refreshTags();

		//===========================================================================
		// ThemeManager::Listener interface
		void themeChanged(ThemeManager::Theme newTheme) override;
		void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;

		class InfoIcon : public Component, public TooltipClient
		{
		public:
			InfoIcon();
			String getTooltip() override;
			void setTooltip(String newTooltip);

			void paint(Graphics& g) override;
		private:
			String mTooltip;
		};

	protected:
		// AnimatedComponent interface
		void onAnimationUpdate() override { repaint(); }

	private:
		Sample::Reference mSample = nullptr;
		TagContainer mTagContainer;

		Rectangle<int> m_TitleRect;
		Rectangle<int> m_TypeRect;
		Rectangle<int> m_TimeRect;
		Rectangle<int> m_ThumbnailRect;
		Rectangle<int> m_TagRect;
		InfoIcon m_InfoIcon;

		const int INFO_ICON_PADDING = 4;

		std::unique_ptr<FileChooser> mFileChooser;
		
		// Cached fonts for performance
		static Font getTitleFont();
		static Font getTimeFont();
		
		// Track if this tile is currently playing (for playback overlay and dynamic buffering)
		bool mIsPlaying = false;

		// Track if this tile is the active sample (for rainbow cue animation)
		bool mIsActiveSample = false;

		// Track if a tag is being dragged over this tile
		bool mDragHighlight = false;

		// Timer for rainbow cue animation at 30hz
		class RainbowAnimationTimer : public juce::Timer
		{
		public:
			RainbowAnimationTimer(SampleTile& owner) : mOwner(owner) {}
			void timerCallback() override { mOwner.repaint(); }
		private:
			SampleTile& mOwner;
		};
		RainbowAnimationTimer mRainbowTimer{*this};

		//Rectangle<float> m_FavoriteButtonRect;
		//Rectangle<float> m_SaveForLaterRect;


		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleTile)
	};
}

#endif
