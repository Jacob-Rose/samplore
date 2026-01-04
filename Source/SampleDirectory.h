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

namespace samplore
{
	struct FilterQuery; // Forward declaration

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
		~SampleDirectory();
		File getFile() const { return mDirectory; }
		Sample::List getChildSamplesRecursive(const FilterQuery& query, bool ignoreCheckSystem);
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


	friend class SamploreApplication; //sets the wildcard really early
	friend class DirectoryExplorerTreeViewItem;
private:

	SampleDirectory(const samplore::SampleDirectory& samplify) {}; //dont call me
	CheckStatus mCheckStatus = CheckStatus::Enabled;
	File mDirectory;
	bool mIncludeChildSamples = true; //if the folder should load its own samples when getsamples is called
	std::vector<std::shared_ptr<Sample>> mChildSamples; //safer
	std::vector<std::shared_ptr<SampleDirectory>> mChildDirectories;

	// Use function to avoid static destruction order issues
	static String& getWildcard()
	{
		static String wildcard;
		return wildcard;
	}
	};
}
#endif
