#include "TagExplorer.h"
#include "SamplifyProperties.h"
#include "SamplifyLookAndFeel.h"
#include "Sample.h"

using namespace samplore;

TagExplorer::TagExplorer()
{
	mNewButtonTag.setButtonText("New Tag");
	mNewButtonTag.onClick = [this] {addNewTag(); };
	addAndMakeVisible(mNewButtonTag);
	addAndMakeVisible(mTagViewport);
	mTagViewport.addAndMakeVisible(mTagsContainer);
	mTagViewport.setViewedComponent(&mTagsContainer);
	//mTagViewport.setViewedComponent()
	mTagViewport.setScrollBarsShown(true, false, true, false);
	SamplifyProperties::getInstance()->getSampleLibrary()->addChangeListener(this);
}

TagExplorer::~TagExplorer()
{
}

void TagExplorer::resized()
{
	mNewButtonTag.setBounds(0,getHeight()-30, getWidth(), 30);
	mTagViewport.setBounds(0, 0, getWidth(), getHeight() - 30);
	mTagsContainer.setBounds(mTagViewport.getBounds());
}

void TagExplorer::paint(Graphics& g)
{

}

void TagExplorer::addNewTag()
{
	mAlertWindow = std::make_unique<AlertWindow>("New Tag Name", "", MessageBoxIconType::NoIcon);
	mAlertWindow->addTextEditor("tagName", "", "Tag Name:");
	mAlertWindow->addButton("OK", 1, KeyPress(KeyPress::returnKey));
	mAlertWindow->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	mAlertWindow->enterModalState(true, ModalCallbackFunction::create([this](int result)
	{
		if (result == 1 && mAlertWindow)
		{
			String tagName = mAlertWindow->getTextEditorContents("tagName");
			if (tagName.isNotEmpty())
			{
				SamplifyProperties::getInstance()->getSampleLibrary()->addTag(tagName);
				mTagsContainer.updateTags(SamplifyProperties::getInstance()->getSampleLibrary()->getCurrentQuery());
			}
		}
		mAlertWindow.reset();
	}), true);
}

void samplore::TagExplorer::changeListenerCallback(ChangeBroadcaster* source)
{
	mTagsContainer.updateTags("");
}

void samplore::TagExplorer::Container::removeNewTag(juce::String tag)
{
	mNewTags.removeTag(tag);
}

void TagExplorer::Container::paint(Graphics& g)
{
	//g.fillAll(Colours::bisque);
	String firstLine = "New Tags:";
	float cHeight = 0.0f;
	g.drawText(firstLine, Rectangle<float>(0.0f, cHeight, getWidth(), infoFont.getHeight()), Justification::centred);
	String secondLine = "Directory Contained:";
	cHeight += mNewTags.getHeight();
	cHeight += infoFont.getHeight();
	g.drawText(secondLine, Rectangle<float>(0.0f, cHeight, getWidth(), infoFont.getHeight()), Justification::centred);
	String thirdLine = "Outside Tags:";
	cHeight += mContainedTags.getHeight();
	cHeight += infoFont.getHeight();
	g.drawText(thirdLine, Rectangle<float>(0.0f, cHeight, getWidth(), infoFont.getHeight()), Justification::centred);
}

void TagExplorer::Container::updateTags(juce::String newSearch)
{
	
	std::vector<SampleLibrary::Tag> allTags = SamplifyProperties::getInstance()->getSampleLibrary()->getTags();
	//remove all new tags that have been used now
	resetTags();
	StringArray passedTags; //in dir
	StringArray failedTags; //not in dir
	//StringArray currentSampleTags = SamplifyProperties::getInstance()->getSampleLibrary()->getCurrentSamples();
	Sample::List currentSamps = SamplifyProperties::getInstance()->getSampleLibrary()->getCurrentSamples();
	
	for (int i = 0; i < allTags.size(); i++)
	{
		if (allTags[i].mTitle.contains(newSearch))
		{
			bool found = false;
			for (int j = 0; j < currentSamps.size(); j++)
			{
				StringArray sampTags = currentSamps[j].getTags();
				if (sampTags.contains(allTags[i].mTitle))
				{
					found = true;
					break;
				}
			}
			if (found)
			{
				passedTags.add(allTags[i].mTitle);
			}
			else
			{
				failedTags.add(allTags[i].mTitle);
			}
		}
	}
	
	mContainedTags.setTags(passedTags);
	mNotContainedTags.setTags(failedTags);
	//updateTagContainerBounds();
	
}

void TagExplorer::Container::updateTagContainerBounds()
{
	float textHeight = infoFont.getHeight();
	float cHeight = 0.0f;
	cHeight += textHeight;
	mNewTags.setBounds(0, cHeight, getWidth(), getHeight());
	mNewTags.setTopLeftPosition(0, cHeight);
	cHeight += mNewTags.getBounds().getHeight();
	cHeight += textHeight;
	mContainedTags.setBounds(0, cHeight, getWidth(), getHeight());
	mContainedTags.setTopLeftPosition(0, cHeight);
	cHeight += mContainedTags.getBounds().getHeight();
	cHeight += textHeight;
	mNotContainedTags.setBounds(0, cHeight, getWidth(), getHeight());
	mNotContainedTags.setTopLeftPosition(0, cHeight);
	cHeight += mNotContainedTags.getBounds().getHeight();
	//setBounds(0, 0, getWidth(),cHeight);
	repaint();
}

void TagExplorer::Container::resetTags()
{
	/*
	StringArray allTags = SamplifyProperties::getInstance()->getSampleLibrary().getAllTags();
	for (int i = 0; i < mNewTags.getTags().size(); i++)
	{
		if (allTags.contains(mNewTags.getTags()[i]))
		{
			mContainedTags.addTag(mNewTags.getTags()[i]);
			mNewTags.removeTag(mNewTags.getTags()[i]);
			i--;
		}
	}
	for (int i = 0; i < mContainedTags.getTags().size(); i++)
	{
		if (!allTags.contains(mContainedTags.getTags()[i]))
		{
			mContainedTags.removeTag(mContainedTags.getTags()[i]);
			i--;
		}
	}
	for (int i = 0; i < mNotContainedTags.getTags().size(); i++)
	{
		if (!allTags.contains(mNotContainedTags.getTags()[i]))
		{
			mNotContainedTags.removeTag(mNotContainedTags.getTags()[i]);
			i--;
		}
	}
	*/
}
