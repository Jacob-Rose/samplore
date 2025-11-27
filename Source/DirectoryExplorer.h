/*
  ==============================================================================

    DirectoryExplorer.h
    Created: 31 May 2018 1:22:06pm
    Author: Jake Rose

  ==============================================================================
*/

#ifndef DIRECTORYEXPLORER_H
#define DIRECTORYEXPLORER_H

#include "JuceHeader.h"

#include "DirectoryExplorerTreeViewItem.h"

namespace samplore
{
	class DirectoryExplorer : public Viewport, public ChangeListener
	{
	public:
		//============================================================
		DirectoryExplorer();
		~DirectoryExplorer();

		void paint(Graphics&) override;
		void resized() override;
		void refresh();
		void changeListenerCallback(ChangeBroadcaster* source);

	private:
		//============================================================
		
		TreeView mDirectoryTree;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectoryExplorer)
	};
}
#endif