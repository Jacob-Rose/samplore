/*
  ==============================================================================

    DirectoryExplorerTreeViewItem.h
    Created: 31 May 2018 1:22:28pm
    Author:  Jake Rose

  ==============================================================================
*/

#ifndef DIRECTORYEXPLORERTREEVIEWITEM_H
#define DIRECTORYEXPLORERTREEVIEWITEM_H

#include "JuceHeader.h"

#include "SampleDirectory.h"

namespace samplore
{
	class DirectoryExplorerTreeViewItem : public TreeViewItem, public FileDragAndDropTarget, public ChangeListener
	{
	public:
		enum ColourIds
		{
			defaultBackgroundId = 0x22123,
			selectedBackgroundId,
			checkboxActiveBackgroundId,
			checkboxMixedBackgroundId,
			checkboxDisabledBackgroundId,
			checkboxNotLoadedBackgroundId
		};


		const juce::String containedSamplesTitle = "Contained Samples";

		DirectoryExplorerTreeViewItem(std::shared_ptr<SampleDirectory> dir);
		DirectoryExplorerTreeViewItem(String string); //this is only used for the root item
		~DirectoryExplorerTreeViewItem();

		bool mightContainSubItems() override;
		bool isInterestedInFileDrag(const StringArray& files) override;
		void filesDropped(const StringArray& files, int x, int y) override;
		void changeListenerCallback(ChangeBroadcaster* source) override;

		void paintItem(Graphics& g, int width, int height) override;
		void paintOpenCloseButton(Graphics&, const Rectangle<float>& area, Colour backgroundColour, bool isMouseOver) override;
		void itemOpennessChanged(bool isNowOpen) override;
		void itemClicked(const MouseEvent& e) override;

		void refreshChildrenPaint();
		//todo allow external drag drop of files/folders into the directory
		//if the samples are in the root directory, just move them
		//else then add samples to sample library

		String getName();
		
		void itemCheckCycled();

	private:
		std::shared_ptr<SampleDirectory> mSampleDirectory;
		String mText;
		bool mShouldUseFile = true;

		
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectoryExplorerTreeViewItem)
	};
}
#endif