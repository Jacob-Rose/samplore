/*
  ==============================================================================

    SampleExplorer.h
    Created: 31 May 2018 12:43:08pm
    Author:  Jake Rose

  ==============================================================================
*/

#ifndef SAMPLEEXPLORER_H
#define SAMPLEEXPLORER_H

#include "JuceHeader.h"

#include "SampleContainer.h"
#include "ActiveTagsBar.h"

#include "SamplifyProperties.h"
#include "ThemeManager.h"

namespace samplore
{
	class SampleExplorer : public Component, 
		public TextEditor::Listener, 
		public ComboBox::Listener,
		public ChangeListener,
		public ThemeManager::Listener
	{
	public:
		enum ColourIds
		{
			loadingWheelColorId
		};
		///Custom Viewport to add support for the visible area changed method,
		///needed to check if scrolled to bottom 
		class SampleViewport : public Viewport
		{
		public:
			SampleViewport(SampleContainer* container);
			void visibleAreaChanged(const Rectangle<int>& newVisibleArea) override;
		private:
			SampleContainer* mSampleContainer = nullptr;
		};

		class SampleSearchbar : public TextEditor, public Button::Listener
		{
		public:
			SampleSearchbar();
			void resized() override;

			void buttonClicked(Button* button) { setText(""); }
		private:
			TextButton mEraseSearchButton;
		};

		//============================================================
		SampleExplorer();
		~SampleExplorer();

		void paint(Graphics&) override;
		void resized() override;

		void textEditorTextChanged(TextEditor&) override;
		void changeListenerCallback(ChangeBroadcaster* source) override;

		void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

		TextEditor& getSearchBar() { return mSearchBar; }
		SampleContainer& getSampleContainer() { return mSampleContainer; }
		ActiveTagsBar& getActiveTagsBar() { return mActiveTagsBar; }

		/// Toggles a tag in the active filter
		void toggleActiveTag(const String& tag);

		//==================================================================
		// ThemeManager::Listener interface
		void themeChanged(ThemeManager::Theme newTheme) override;
		void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;

	private:
		/// Triggers sample update with current search + active tags
		void updateFilter();
		//============================================================
		bool mIsUpdating = false;
		ComboBox mFilter;
		SampleViewport mViewport;
		SampleSearchbar mSearchBar;
		ActiveTagsBar mActiveTagsBar;
		SampleContainer mSampleContainer;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleExplorer)
	};
}

#endif
