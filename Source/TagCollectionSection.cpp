/*
  ==============================================================================

    TagCollectionSection.cpp
    Created: 2025
    Author:  Samplore Team

  ==============================================================================
*/

#include "TagCollectionSection.h"
#include "SamplifyProperties.h"
#include "TagTile.h"

using namespace samplore;

TagCollectionSection::TagCollectionSection(juce::String collectionName, bool isDefault)
	: mCollectionName(collectionName),
	  mIsDefault(isDefault),
	  mTagContainer(true)
{
	// Collapse button (chevron)
	mCollapseButton.setButtonText("v");
	mCollapseButton.onClick = [this] { toggleCollapsed(); };
	mCollapseButton.setColour(TextButton::buttonColourId, Colours::transparentBlack);
	mCollapseButton.setColour(TextButton::textColourOffId,
		ThemeManager::getInstance().getColorForRole(ThemeManager::ColorRole::TextPrimary));
	addAndMakeVisible(mCollapseButton);

	// Title label
	String displayName = mIsDefault ? "Default" : mCollectionName;
	mTitleLabel.setText(displayName, dontSendNotification);
	mTitleLabel.setFont(FontOptions(14.0f, Font::bold));
	mTitleLabel.setColour(Label::textColourId,
		ThemeManager::getInstance().getColorForRole(ThemeManager::ColorRole::TextPrimary));
	addAndMakeVisible(mTitleLabel);

	// Move down button (only for non-default collections)
	if (!mIsDefault)
	{
		mMoveDownButton.setButtonText("v");
		mMoveDownButton.onClick = [this] {
			if (onMoveDown)
				onMoveDown(mCollectionName);
		};
		mMoveDownButton.setColour(TextButton::buttonColourId, Colours::transparentBlack);
		mMoveDownButton.setColour(TextButton::textColourOffId,
			ThemeManager::getInstance().getColorForRole(ThemeManager::ColorRole::TextSecondary));
		addAndMakeVisible(mMoveDownButton);
	}

	addAndMakeVisible(mTagContainer);

	ThemeManager::getInstance().addListener(this);
}

TagCollectionSection::~TagCollectionSection()
{
	ThemeManager::getInstance().removeListener(this);
}

void TagCollectionSection::paint(Graphics& g)
{
	auto& theme = ThemeManager::getInstance();

	// Header background
	auto headerBounds = getLocalBounds().removeFromTop(HEADER_HEIGHT);
	g.setColour(theme.getColorForRole(ThemeManager::ColorRole::BackgroundSecondary));
	g.fillRect(headerBounds);

	// Content background
	if (!mCollapsed)
	{
		auto contentBounds = getLocalBounds().withTrimmedTop(HEADER_HEIGHT);
		g.setColour(theme.getColorForRole(ThemeManager::ColorRole::Background));
		g.fillRect(contentBounds);
	}

	// Drag highlight border
	if (mDragHighlight)
	{
		g.setColour(theme.getColorForRole(ThemeManager::ColorRole::AccentPrimary));
		g.drawRect(getLocalBounds(), 2);
	}

	// Bottom border
	g.setColour(theme.getColorForRole(ThemeManager::ColorRole::Border));
	g.drawHorizontalLine(getHeight() - 1, 0.0f, (float)getWidth());
}

void TagCollectionSection::resized()
{
	auto bounds = getLocalBounds();
	auto headerBounds = bounds.removeFromTop(HEADER_HEIGHT);

	// Collapse button on the left
	mCollapseButton.setBounds(headerBounds.removeFromLeft(HEADER_HEIGHT));

	// Move down button on the right (if not default)
	if (!mIsDefault)
	{
		mMoveDownButton.setBounds(headerBounds.removeFromRight(HEADER_HEIGHT));
	}

	// Title takes remaining header space
	mTitleLabel.setBounds(headerBounds);

	// Tag container takes content area
	if (!mCollapsed)
	{
		mTagContainer.setBounds(bounds.reduced(4, 2));
		mTagContainer.setVisible(true);
	}
	else
	{
		mTagContainer.setVisible(false);
	}
}

void TagCollectionSection::setCollapsed(bool collapsed)
{
	if (mCollapsed != collapsed)
	{
		mCollapsed = collapsed;
		mCollapseButton.setButtonText(mCollapsed ? ">" : "v");
		resized();

		// Notify parent to recalculate layout
		if (onLayoutChanged)
			onLayoutChanged();
	}
}

int TagCollectionSection::calculateHeight() const
{
	if (mCollapsed)
		return HEADER_HEIGHT;

	int contentHeight = mTagContainer.calculateAllRowsHeight();
	if (contentHeight == 0)
		contentHeight = 24; // Minimum height for empty section

	return HEADER_HEIGHT + contentHeight + 8; // 8 for padding
}

void TagCollectionSection::setTags(StringArray tags)
{
	mTagContainer.setTags(tags);
}

bool TagCollectionSection::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
	return dragSourceDetails.description == "Tags";
}

void TagCollectionSection::itemDropped(const SourceDetails& dragSourceDetails)
{
	if (TagTile* tagTile = dynamic_cast<TagTile*>(dragSourceDetails.sourceComponent.get()))
	{
		String tagName = tagTile->getTag();
		SamplifyProperties::getInstance()->getSampleLibrary()
			->setTagCollection(tagName, mIsDefault ? "" : mCollectionName);
	}
	mDragHighlight = false;
	repaint();
}

void TagCollectionSection::itemDragEnter(const SourceDetails&)
{
	mDragHighlight = true;
	repaint();
}

void TagCollectionSection::itemDragExit(const SourceDetails&)
{
	mDragHighlight = false;
	repaint();
}

void TagCollectionSection::themeChanged(ThemeManager::Theme)
{
	auto& theme = ThemeManager::getInstance();

	mCollapseButton.setColour(TextButton::textColourOffId,
		theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
	mTitleLabel.setColour(Label::textColourId,
		theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));

	if (!mIsDefault)
	{
		mMoveDownButton.setColour(TextButton::textColourOffId,
			theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
	}

	repaint();
}
