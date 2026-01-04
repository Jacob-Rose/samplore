#include "SamplifyProperties.h"
#include "SamplifyMainComponent.h"
#include "SamplifyLookAndFeel.h"

using namespace samplore;

SamplifyProperties* SamplifyProperties::smAppProperties = nullptr;

SamplifyProperties::SamplifyProperties()
{
	PropertiesFile::Options propFileOptions = PropertiesFile::Options();
	propFileOptions.applicationName = "Samplore";
	propFileOptions.commonToAllUsers = false;
	propFileOptions.filenameSuffix = ".settings";
    propFileOptions.osxLibrarySubFolder = "Application Support/Samplore";
	propFileOptions.ignoreCaseOfKeyNames = true;
	propFileOptions.storageFormat = PropertiesFile::StorageFormat::storeAsXML;
	setStorageParameters(propFileOptions);
}

SamplifyProperties::~SamplifyProperties()
{
	cleanup();
}

void SamplifyProperties::browseForDirectory(std::function<void(const File&)> callback)
{
	mFileChooser = std::make_unique<FileChooser>("Select Music Directory", File::getSpecialLocation(File::userHomeDirectory));
	mFileChooser->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories,
		[callback](const FileChooser& fc)
		{
			auto result = fc.getResult();
			if (callback)
				callback(result);
		});
}

void SamplifyProperties::cleanupInstance()
{
	if (smAppProperties != nullptr)
	{
		delete smAppProperties;
		smAppProperties = nullptr;
	}
}

void SamplifyProperties::initInstance()
{
	if (smAppProperties == nullptr)
	{
		smAppProperties = new SamplifyProperties();
		smAppProperties->init();
	}
}

SamplifyProperties* SamplifyProperties::getInstance()
{
	return smAppProperties;
}

void SamplifyProperties::init()
{
	mSampleLibrary = std::make_shared<SampleLibrary>();
	//mTagLibrary = std::make_shared<TagLibrary>();
	loadPropertiesFile();
	mIsInit = true;
}

void SamplifyProperties::cleanup()
{
	if (mIsInit)
	{
		mAudioPlayer->stop();
		savePropertiesFile();
		closeFiles();
		
		// Explicitly clear sample library to release all Sample objects
		if (mSampleLibrary)
		{
			mSampleLibrary.reset();
		}
		
		mIsInit = false;
	}
}

void SamplifyProperties::loadPropertiesFile()
{
	PropertiesFile* propFile = getUserSettings();
	if (propFile->isValidFile())
	{
		//load dirs
		int dirCount = propFile->getIntValue("directory count");
		if (dirCount == 0)
		{
			browseForDirectory([this](const File& dir)
			{
				if (dir.exists())
				{
					mSampleLibrary->addDirectory(dir);
					// Preload tags after adding the first directory
					mSampleLibrary->launchPreloadAllTags();
				}
			});
		}
		else
		{
			for (int i = 0; i < dirCount; i++)
			{
				mSampleLibrary->addDirectory(File(propFile->getValue("directory " + String(i))));
			}
			// After loading all directories, preload tags from all samples
			mSampleLibrary->launchPreloadAllTags();
		}
		
		//load tags (stored as hue values and collection)
		int tagCount = propFile->getIntValue("tag count");
		for (int i = 0; i < tagCount; i++)
		{
			String tag = propFile->getValue("tag " + String(i));
			jassert(tag != "");
			float hue = propFile->getValue("tag " + tag).getFloatValue();
			String collection = propFile->getValue("tag " + tag + "_collection", "");
			mSampleLibrary->addTag(tag, hue, collection);
		}

		//load tag collections order
		int collectionCount = propFile->getIntValue("collection count");
		for (int i = 0; i < collectionCount; i++)
		{
			String collection = propFile->getValue("collection " + String(i));
			mSampleLibrary->addCollection(collection);
		}

		//HERE IS WHERE DEFAULT VALUES FOR LOOK AND FEEL ARE SET
		//load window settings
		AppValues::getInstance().MAIN_BACKGROUND_COLOR = Colour::fromString(propFile->getValue("MAIN_BACKGROUND_COLOR", Colours::white.toString()));
		AppValues::getInstance().MAIN_FOREGROUND_COLOR = Colour::fromString(propFile->getValue("MAIN_FOREGROUND_COLOR", Colours::blueviolet.toString()));
		AppValues::getInstance().WINDOW_WIDTH = propFile->getIntValue("START_WIDTH", 1280);
		AppValues::getInstance().WINDOW_HEIGHT = propFile->getIntValue("START_HEIGHT", 900);
		AppValues::getInstance().SAMPLE_TILE_ASPECT_RATIO = (float)propFile->getDoubleValue("SAMPLE_TILE_ASPECT_RATIO", 0.666);
		AppValues::getInstance().SAMPLE_TILE_MIN_WIDTH = (float)propFile->getDoubleValue("SAMPLE_TILE_MIN_WIDTH", 120);
		AppValues::getInstance().AUDIO_THUMBNAIL_LINE_COUNT = (float)propFile->getDoubleValue("AUDIO_THUMBNAIL_LINE_COUNT", 50);
		AppValues::getInstance().AUDIO_THUMBNAIL_LINE_COUNT_PLAYER = (float)propFile->getDoubleValue("AUDIO_THUMBNAIL_LINE_COUNT_PLAYER", 120);
		AppValues::getInstance().updateDrawablesColors();
	}
	else
	{
		//run initial setup here
		browseForDirectory([this](const File& dir)
		{
			if (dir.exists())
			{
				mSampleLibrary->addDirectory(dir);
				// Preload tags after adding the first directory
				mSampleLibrary->launchPreloadAllTags();
			}
		});
	}
}


void SamplifyProperties::savePropertiesFile()
{
	PropertiesFile* propFile = getUserSettings();
	if (propFile->isValidFile())
	{
		propFile->clear();
		//Save Dirs
		const auto& dirs = mSampleLibrary->getDirectories();
		propFile->setValue("directory count", (int)dirs.size());
		for (int i = 0; i < dirs.size(); i++)
		{
			propFile->setValue("directory " + String(i), dirs[i]->getFile().getFullPathName());
		}

		//save tags (stored as hue values and collection)
		std::vector<SampleLibrary::Tag> allTags = mSampleLibrary->getTags();
		for(int i = 0; i < allTags.size(); i++)
		{
			propFile->setValue("tag " + String(i), allTags[i].mTitle);
			propFile->setValue("tag " + allTags[i].mTitle, String(allTags[i].mHue));
			propFile->setValue("tag " + allTags[i].mTitle + "_collection", allTags[i].mCollection);
		}
		propFile->setValue("tag count", (int)allTags.size());

		//save tag collections order
		StringArray collections = mSampleLibrary->getCollections();
		propFile->setValue("collection count", (int)collections.size());
		for (int i = 0; i < collections.size(); i++)
		{
			propFile->setValue("collection " + String(i), collections[i]);
		}
		propFile->saveIfNeeded();

		//save look and feel
		propFile->setValue("MAIN_BACKGROUND_COLOR", AppValues::getInstance().MAIN_BACKGROUND_COLOR.toString());
		propFile->setValue("MAIN_FOREGROUND_COLOR", AppValues::getInstance().MAIN_FOREGROUND_COLOR.toString());
		propFile->setValue("START_WIDTH", AppValues::getInstance().WINDOW_WIDTH);
		propFile->setValue("START_HEIGHT", AppValues::getInstance().WINDOW_HEIGHT);
		propFile->setValue("SAMPLE_TILE_ASPECT_RATIO", AppValues::getInstance().SAMPLE_TILE_ASPECT_RATIO);
		propFile->setValue("SAMPLE_TILE_MIN_WIDTH", AppValues::getInstance().SAMPLE_TILE_MIN_WIDTH);
		propFile->setValue("AUDIO_THUMBNAIL_LINE_COUNT_PLAYER", AppValues::getInstance().AUDIO_THUMBNAIL_LINE_COUNT_PLAYER);
		propFile->setValue("AUDIO_THUMBNAIL_LINE_COUNT", AppValues::getInstance().AUDIO_THUMBNAIL_LINE_COUNT);
	}
	else
	{
		throw "Properties File is not valid file";
	}
}

void SamplifyProperties::changeListenerCallback(ChangeBroadcaster* source)
{
	savePropertiesFile();
}

