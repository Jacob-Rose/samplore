/*
  ==============================================================================

    SamplifyMenuBar.h
    Created: 6 Jul 2019 11:36:41am
    Author:  Jake Rose

	No idea how menus worked, source for help
	https://github.com/vinniefalco/SimpleDJ/blob/master/Extern/JUCE/extras/JuceDemo/Source/MainDemoWindow.cpp

  ==============================================================================
*/

#ifndef SAMPLIFYMENUBAR_H
#define SAMPLIFYMENUBAR_H

#include "JuceHeader.h"

namespace samplify
{
	/*
	class DeleteSamplifyFilesThread : public ThreadWithProgressWindow
	{
	public:
		DeleteSamplifyFilesThread(File dir) : ThreadWithProgressWindow("deleting .samplify", true, false),
			mDirectory(dir) {}
		void run() override;
	private:
		File mDirectory;
	};
	*/

	class SamplifyMainMenu : public Component, public MenuBarModel
	{
	public:
		enum CommandIDs
		{
			noCommand = 0,
			addDirectory = 1,
			refreshDirectories,
			removeDirectory = 1024, //outlier number required for removeDirectory sub-menu
			removeSampFiles = 2,
			setPreferences,
			setVolume,
			exitApplication,
			togglePlayerWindow,
			toggleFilterWindow,
			toggleDirectoryWindow,
			viewInformation,
			visitWebsite
		};

		SamplifyMainMenu();
		//Menu Bar Model Functions
		StringArray getMenuBarNames() override;
		PopupMenu getMenuForIndex(int topLevelMenuIndex, const String& menuName) override;
		void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

	private:
		std::unique_ptr<AlertWindow> mVolumeWindow;
	};
}
#endif
