/*
  ==============================================================================

    TagExplorer.cpp
    Created: 30 May 2020 6:33:22pm
    Author:  jacob

  ==============================================================================
*/

#include "TagExplorer.h"
#include "SamplifyLookAndFeel.h"
#include "SamplifyProperties.h"

using namespace samplore;

TagExplorer::TagExplorer()
{
	mNewButtonTag.setButtonText("New Tag");
	mNewButtonTag.onClick = [this] { addNewTag(); };
	addAndMakeVisible(mNewButtonTag);
	addAndMakeVisible(mTagViewport);
	mTagViewport.setViewedComponent(&mScrollContent, false);
	mTagViewport.setScrollBarsShown(true, false, true, false);
	SamplifyProperties::getInstance()->getSampleLibrary()->addChangeListener(this);

	rebuildSections();
}

TagExplorer::~TagExplorer()
{
	// Remove from SampleLibrary to prevent dangling pointer
	if (auto lib = SamplifyProperties::getInstance()->getSampleLibrary())
		lib->removeChangeListener(this);
}

void TagExplorer::resized()
{
	mNewButtonTag.setBounds(0, getHeight() - 30, getWidth(), 30);
	mTagViewport.setBounds(0, 0, getWidth(), getHeight() - 30);

	// Calculate content width (minus scrollbar)
	int contentWidth = mTagViewport.getWidth() - mTagViewport.getScrollBarThickness();

	// Stack sections vertically
	int yPos = 0;
	for (auto& section : mSections)
	{
		section->setSize(contentWidth, section->calculateHeight());
		section->setTopLeftPosition(0, yPos);
		yPos += section->getHeight();
	}

	// Set scroll content size
	mScrollContent.setSize(contentWidth, yPos);
}

void TagExplorer::paint(Graphics&)
{
}

void TagExplorer::rebuildSections()
{
	// Clear existing sections
	mSections.clear();
	mScrollContent.removeAllChildren();

	auto library = SamplifyProperties::getInstance()->getSampleLibrary();

	// Get ordered collections
	StringArray collections = library->getCollections();

	// Create a section for each user collection
	for (const auto& collectionName : collections)
	{
		auto section = std::make_unique<TagCollectionSection>(collectionName, false);

		// Wire up move down callback
		section->onMoveDown = [this](juce::String name) {
			SamplifyProperties::getInstance()->getSampleLibrary()->moveCollectionDown(name);
		};

		// Wire up layout changed callback for collapse/expand
		section->onLayoutChanged = [this]() {
			resized();
		};

		mScrollContent.addAndMakeVisible(section.get());
		mSections.push_back(std::move(section));
	}

	// Create the default section (always at bottom)
	auto defaultSection = std::make_unique<TagCollectionSection>("", true);
	defaultSection->onLayoutChanged = [this]() {
		resized();
	};
	mScrollContent.addAndMakeVisible(defaultSection.get());
	mSections.push_back(std::move(defaultSection));

	// Update tags in all sections
	updateTags(mCurrentQuery);

	// Trigger layout
	resized();
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
			}
		}
		mAlertWindow.reset();
	}), true);
}

void TagExplorer::updateTags(juce::String query)
{
	mCurrentQuery = query;

	auto library = SamplifyProperties::getInstance()->getSampleLibrary();
	StringArray collections = library->getCollections();

	// Update each section with its tags
	for (auto& section : mSections)
	{
		String collectionName = section->getCollectionName();
		std::vector<SampleLibrary::Tag> tagsInCollection = library->getTagsInCollection(collectionName);

		// Filter by query and extract tag names
		StringArray filteredTags;
		for (const auto& tag : tagsInCollection)
		{
			if (query.isEmpty() || tag.mTitle.containsIgnoreCase(query))
			{
				filteredTags.add(tag.mTitle);
			}
		}

		section->setTags(filteredTags);
	}

	// Recalculate layout
	resized();
}

void TagExplorer::changeListenerCallback(ChangeBroadcaster* source)
{
	rebuildSections();
}
