#include "SamplifyMenuBar.h"
#include "SamplifyProperties.h"
#include "PreferenceWindow.h"
#include "InfoWindow.h"
#include "SamplifyMainComponent.h"

using namespace samplore;

SamplifyMainMenu::SamplifyMainMenu()
{
	ThemeManager::getInstance().addListener(this);
}

SamplifyMainMenu::~SamplifyMainMenu()
{
	ThemeManager::getInstance().removeListener(this);
}

StringArray SamplifyMainMenu::getMenuBarNames()
{
	StringArray names = { "File", "View", "Info" };
	return names;
}

void SamplifyMainMenu::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
	if (menuItemID == refreshDirectories)
	{
		SamplifyProperties::getInstance()->getSampleLibrary()->refreshDirectories();
	}
	else if (menuItemID == openImportWizard)
	{
		if (auto* mainComponent = SamplifyMainComponent::getInstance())
		{
			mainComponent->showImportWizard();
		}
	}
	else if (menuItemID == setVolume)
	{
		mVolumeWindow = std::make_unique<AlertWindow>("Set Gain", "", MessageBoxIconType::NoIcon);
		mVolumeWindow->addCustomComponent(new Slider());
		if (auto* slider = dynamic_cast<Slider*>(mVolumeWindow->getCustomComponent(0)))
		{
			slider->setRange(0, 2);
			slider->setSize(200, 40);
		}
		mVolumeWindow->addButton("OK", 1);
		mVolumeWindow->enterModalState(true, ModalCallbackFunction::create([this](int result)
		{
			if (result == 1 && mVolumeWindow)
			{
				if (auto* slider = dynamic_cast<Slider*>(mVolumeWindow->getCustomComponent(0)))
				{
					SamplifyProperties::getInstance()->getAudioPlayer()->setVolumeMultiply(slider->getValue());
				}
			}
			mVolumeWindow.reset();
		}), true);
	}
	else if (menuItemID == setPreferences)
	{
		if (auto* mainComponent = SamplifyMainComponent::getInstance())
		{
			mainComponent->showPreferences();
		}
	}
	else if (menuItemID == exitApplication)
	{
		JUCEApplication::getInstance()->systemRequestedQuit(); //close app
	}

	else if (menuItemID == viewInformation)
	{
		auto* window = new InfoWindow();
		window->enterModalState(true, ModalCallbackFunction::create([window](int)
		{
			delete window;
		}), true);
	}
	else if (menuItemID == visitWebsite)
	{
		URL("www.samplify.app").launchInDefaultBrowser();
	}
	else if (menuItemID == openCueBindings)
	{
		if (auto* mainComponent = SamplifyMainComponent::getInstance())
		{
			mainComponent->showCueBindingsWindow();
		}
	}

}

PopupMenu SamplifyMainMenu::getMenuForIndex(int menuIndex, const String& menuName)
{
	PopupMenu menu;
	if (menuIndex == 0) //File
	{
		menu.addItem(refreshDirectories, "Refresh Directories", true, false);
		menu.addItem(openImportWizard, "Import Wizard", true, false);
		menu.addSeparator();
		menu.addItem(setPreferences, "Preferences", true, false);
		menu.addItem(exitApplication, "Exit Application", true, false);
	}
	else if (menuIndex == 1) //View
	{
		menu.addItem(openCueBindings, "Cue Bindings (Ctrl+K)", true, false);
		//todo check if enabled or disabled
		//menu.addItem(togglePlayerWindow, "Toggle Player Window (TBD)");
		//menu.addItem(toggleFilterWindow, "Toggle Filter Window (TBD)");
		//menu.addItem(toggleDirectoryWindow, "Toggle Directory Window (TBD)");
	}
	else if (menuIndex == 2) //Info
	{
		menu.addItem(viewInformation, "View Information", true, false);
		menu.addItem(visitWebsite, "Visit Website", true, false);
	}
	//menu.addSeparator();
	return menu;
}

//==============================================================================
// ThemeManager::Listener implementation
void SamplifyMainMenu::themeChanged(ThemeManager::Theme newTheme)
{
	// The MainWindow will update the LookAndFeel
	// We just need to notify JUCE that menu needs redrawing
	menuItemsChanged();
}

void SamplifyMainMenu::colorChanged(ThemeManager::ColorRole role, Colour newColor)
{
	// The MainWindow will update the LookAndFeel
	// We just need to notify JUCE that menu needs redrawing
	menuItemsChanged();
}

/*
void SamplifyMainMenu::DeleteSamplifyFilesThread::run()
{
	DirectoryIterator iterator(mDirectory, true, "*.samplify");
	while (iterator.next())
	{
		iterator.getFile().deleteFile();
	}
}
*/