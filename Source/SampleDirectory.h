/*
  ==============================================================================

    SampleDirectory.h
    Created: 13 Sep 2019 5:43:59pm
    Author:  jacob

  ==============================================================================
*/

#ifndef SAMPLEDIRECTORY_H
#define SAMPLEDIRECTORY_H
#include <JuceHeader.h>

#include <vector>

#include "Sample.h"

namespace samplify
{
	enum class CheckStatus
	{
		NotLoaded = -1,
		Enabled,
		Disabled,
		Mixed,
	};
	class SampleDirectory: public ChangeBroadcaster, public ChangeListener
	{
	public:
		SampleDirectory(File file);
		File getFile() const { return mDirectory; }
		Sample::List getChildSamplesRecursive(juce::String query, bool ignoreCheckSystem);
		Sample::List getChildSamples();

		void updateChildrenItems(CheckStatus checkStatus);

		void changeListenerCallback(ChangeBroadcaster* source) override;
		void cycleCurrentCheck();

		void setCheckStatus(CheckStatus newCheckStatus);
		CheckStatus getCheckStatus() { return mCheckStatus; }
		int getChildDirectoryCount() { return mChildDirectories.size(); }

		void recursiveRefresh();
		void rescanFiles();
		std::shared_ptr<SampleDirectory> getChildDirectory(int index);


		friend class SamplifyPlusApplication; //sets the wildcard really fucking early
		friend class DirectoryExplorerTreeViewItem;
	private:

		SampleDirectory(const samplify::SampleDirectory& samplify) {}; //dont call me
		CheckStatus mCheckStatus = CheckStatus::Enabled;
		File mDirectory;
		bool mIncludeChildSamples = true; //if the folder should load its own samples when getsamples is called
		std::vector<std::shared_ptr<Sample>> mChildSamples; //safer
		std::vector<std::shared_ptr<SampleDirectory>> mChildDirectories;


		static String mWildcard; //set by SampleLibrary using the audioplayer to get all supported formats
	};
}
#endif
