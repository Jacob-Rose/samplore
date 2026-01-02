#include "TagExplorer.h"
#include "SamplifyProperties.h"
#include "SamplifyLookAndFeel.h"
#include "Sample.h"
#include "SampleLibrary.h"

using namespace samplore;

TagExplorer::TagExplorer()
{
	mNewButtonTag.setButtonText("New Tag");
	mNewButtonTag.onClick = [this] { addNewTag(); };
	addAndMakeVisible(mNewButtonTag);
	addAndMakeVisible(mTagViewport);
	mTagViewport.setViewedComponent(&mTagsContainer, false);
	mTagViewport.setScrollBarsShown(true, false, true, false);

	SamplifyProperties::getInstance()->getSampleLibrary()->addChangeListener(this);
	ThemeManager::getInstance().addListener(this);
}

TagExplorer::~TagExplorer()
{
	ThemeManager::getInstance().removeListener(this);

	if (auto lib = SamplifyProperties::getInstance()->getSampleLibrary())
		lib->removeChangeListener(this);
}

void TagExplorer::resized()
{
	mNewButtonTag.setBounds(0, getHeight() - 30, getWidth(), 30);
	mTagViewport.setBounds(0, 0, getWidth(), getHeight() - 30);

	// Set container width, let content determine height
	int contentWidth = mTagViewport.getWidth() - mTagViewport.getScrollBarThickness();
	mTagsContainer.setSize(contentWidth, mTagsContainer.getHeight());
	mTagsContainer.updateBounds();
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
				mTagsContainer.updateTags();
			}
		}
		mAlertWindow.reset();
	}), true);
}

void TagExplorer::changeListenerCallback(ChangeBroadcaster* source)
{
	mTagsContainer.updateTags();
}

//==============================================================================
// Container implementation
//==============================================================================

void TagExplorer::Container::updateTags()
{
	auto library = SamplifyProperties::getInstance()->getSampleLibrary();
	std::vector<SampleLibrary::Tag> allTags = library->getTags();

	StringArray tagNames;
	for (const auto& tag : allTags)
	{
		tagNames.add(tag.mTitle);
	}

	mTags.setTags(tagNames);
	updateBounds();
}

void TagExplorer::Container::updateBounds()
{
	mTags.setSize(getWidth(), 1000); // Temporary large height for calculation
	int contentHeight = jmax(20, mTags.calculateAllRowsHeight());

	mTags.setBounds(0, 0, getWidth(), contentHeight);
	setSize(getWidth(), contentHeight);
}

//==============================================================================
// ThemeManager::Listener implementation
//==============================================================================

void TagExplorer::themeChanged(ThemeManager::Theme newTheme)
{
	mTagsContainer.repaint();
}

void TagExplorer::colorChanged(ThemeManager::ColorRole role, Colour newColor)
{
	if (role == ThemeManager::ColorRole::TextPrimary || role == ThemeManager::ColorRole::TextSecondary)
	{
		mTagsContainer.repaint();
	}
}
