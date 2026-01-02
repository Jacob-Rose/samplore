#include "DirectoryExplorerTreeViewItem.h"
#include "SamplifyMainComponent.h"
#include "SamplifyProperties.h"
#include "SamplifyLookAndFeel.h"
#include "ThemeManager.h"


using namespace samplore;

DirectoryExplorerTreeViewItem::DirectoryExplorerTreeViewItem(std::shared_ptr<SampleDirectory> dir)
{
	mSampleDirectory = dir;
	mSampleDirectory->addChangeListener(this);
	mShouldUseFile = true;
	
	// Register with ThemeManager
	ThemeManager::getInstance().addListener(this);
}

DirectoryExplorerTreeViewItem::DirectoryExplorerTreeViewItem(String string)
{ 
	mText = string;
	mShouldUseFile = false;
	
	// Register with ThemeManager
	ThemeManager::getInstance().addListener(this);
}

DirectoryExplorerTreeViewItem::~DirectoryExplorerTreeViewItem()
{
	// Remove ourselves as listener from SampleDirectory before destruction
	if (mSampleDirectory)
		mSampleDirectory->removeChangeListener(this);
	
	ThemeManager::getInstance().removeListener(this);
}

bool DirectoryExplorerTreeViewItem::mightContainSubItems()
{
	if (mShouldUseFile)
	{
		return mSampleDirectory->getFile().containsSubDirectories();
	}
	else
	{
		return true;
	}
}

bool DirectoryExplorerTreeViewItem::isInterestedInFileDrag(const StringArray& files)
{
	bool allDirs = true;
	for (int i = 0; i < files.size(); i++)
	{
		allDirs = File(files[i]).isDirectory() && allDirs;
	}
	return allDirs;
}

void DirectoryExplorerTreeViewItem::filesDropped(const StringArray& files, int x, int y)
{
	//todo handle adding directories
}

void samplore::DirectoryExplorerTreeViewItem::changeListenerCallback(ChangeBroadcaster* source)
{
	repaintItem();
}

String DirectoryExplorerTreeViewItem::getName()
{
	if (mShouldUseFile)
	{
		return mSampleDirectory->getFile().getFileName();
	}
	else
	{
		return mText;
	}
}

void DirectoryExplorerTreeViewItem::paintItem(Graphics & g, int width, int height)
{
	//check if not added yet, dont draw
	if (getOwnerView() != nullptr)
	{
		Colour itemBackgroundColor;
		Colour textColor;
		if (isSelected())
		{
			itemBackgroundColor = getOwnerView()->getLookAndFeel().findColour(selectedBackgroundId);
		}
		else
		{
			itemBackgroundColor = getOwnerView()->getLookAndFeel().findColour(defaultBackgroundId);
		}

		//draw highlight
		g.setColour(itemBackgroundColor);
		g.fillRoundedRectangle(0, 0, width, height, 4.0f);
		g.setColour(itemBackgroundColor.darker(0.2f));
		g.drawRoundedRectangle(0, 0, width, height, 4.0f, 1.0f);

		// Use theme text color instead of calculating from background
		textColor = ThemeManager::getInstance().getColorForRole(ThemeManager::ColorRole::TextPrimary);

		if (mShouldUseFile)
		{
			Colour checkboxBackgroundColor;
			switch (mSampleDirectory->getCheckStatus())
			{
			case CheckStatus::NotLoaded:
				checkboxBackgroundColor = getOwnerView()->getLookAndFeel().findColour(checkboxNotLoadedBackgroundId);
				break;
			case CheckStatus::Mixed:
				checkboxBackgroundColor = getOwnerView()->getLookAndFeel().findColour(checkboxMixedBackgroundId);
				break;
			case CheckStatus::Enabled:
				checkboxBackgroundColor = getOwnerView()->getLookAndFeel().findColour(checkboxActiveBackgroundId);
				break;
			case CheckStatus::Disabled:
				checkboxBackgroundColor = getOwnerView()->getLookAndFeel().findColour(checkboxDisabledBackgroundId);
				break;
			}

			g.setFont(12);
			float padding = 2.0f;
			Rectangle<float> checkBoxRect = Rectangle<float>(padding, padding, height - (padding * 2), height - (padding * 2));
			float checkBoxCornerWidth = 4.0f;

			g.setColour(checkboxBackgroundColor);
			g.fillRoundedRectangle(checkBoxRect, checkBoxCornerWidth);


			if (mSampleDirectory->getCheckStatus() == CheckStatus::NotLoaded)
			{
				AppValues::getInstance().getDrawable("minus")->drawWithin(g, checkBoxRect.reduced(1.0f), RectanglePlacement::centred, 1.0f);
			}
			else if (mSampleDirectory->getCheckStatus() == CheckStatus::Enabled)
			{
				AppValues::getInstance().getDrawable("correct")->drawWithin(g, checkBoxRect.reduced(1.0f), RectanglePlacement::centred, 1.0f);
			}
			else if (mSampleDirectory->getCheckStatus() == CheckStatus::Mixed)
			{
				AppValues::getInstance().getDrawable("minus")->drawWithin(g, checkBoxRect.reduced(1.0f), RectanglePlacement::centred, 1.0f);
			}
			else //disabled
			{

			}

			g.setColour(Colours::black);
			g.drawRoundedRectangle(checkBoxRect, checkBoxCornerWidth, 1.0f);
		}


		//draw text no matter what
		g.setColour(textColor);
		juce::String text = getName();
		g.drawText(text, height, 0, width, height, Justification::centredLeft, true);
	}
}

void DirectoryExplorerTreeViewItem::paintOpenCloseButton(Graphics& g, const Rectangle<float>& area, Colour backgroundColour, bool isMouseOver)
{
	//ignore background colour and isMouseOver
	getOwnerView()->getLookAndFeel().drawTreeviewPlusMinusBox(g, area, getOwnerView()->getLookAndFeel().findColour(checkboxActiveBackgroundId), isOpen(), true);
}

void DirectoryExplorerTreeViewItem::itemOpennessChanged(bool isNowOpen)
{
	if (isNowOpen && mShouldUseFile && mSampleDirectory != nullptr)
	{
		if (getNumSubItems() == 0)
		{
			Array<File> files;
			int childDirCount = mSampleDirectory->getChildDirectoryCount();
			for (int i = 0; i < childDirCount; i++)
			{
				DirectoryExplorerTreeViewItem* item = new DirectoryExplorerTreeViewItem(mSampleDirectory->getChildDirectory(i));
				addSubItem(item);
			}
		}
	}
}

void DirectoryExplorerTreeViewItem::itemClicked(const MouseEvent& e)
{
	if (getParentItem() != nullptr)
	{
		if (e.mods.isLeftButtonDown())
		{
			int itemHeight = getItemHeight();
			int itemXPos = getItemPosition(false).getX();
			int xPos = e.getMouseDownPosition().getX() - itemXPos;
			if (xPos < itemHeight)
			{
				itemCheckCycled();
			}
		}
		else if (e.mods.isRightButtonDown())
		{
			PopupMenu dirOptions;
			dirOptions.addItem(1, "Select Exclusively");
			auto* parentItem = getParentItem();
			auto* sampleDir = mSampleDirectory.get();
			dirOptions.showMenuAsync(PopupMenu::Options(), [parentItem, sampleDir](int selection)
			{
				if (selection == 1)
				{
					if (DirectoryExplorerTreeViewItem* dirTVI = dynamic_cast<DirectoryExplorerTreeViewItem*>(parentItem))
					{
						dirTVI->mSampleDirectory->setCheckStatus(CheckStatus::Disabled);
					}
					sampleDir->setCheckStatus(CheckStatus::Enabled);
				}
			});
		}
	}
}

void samplore::DirectoryExplorerTreeViewItem::refreshChildrenPaint()
{
	repaintItem();
	for (int i = 0; i < getNumSubItems(); i++)
	{
		((DirectoryExplorerTreeViewItem*)getSubItem(i))->refreshChildrenPaint();
	}
}

void DirectoryExplorerTreeViewItem::itemCheckCycled()
{
	mSampleDirectory->cycleCurrentCheck();
}

//==============================================================================
// ThemeManager::Listener implementation
void DirectoryExplorerTreeViewItem::themeChanged(ThemeManager::Theme newTheme)
{
	repaintItem();
}

void DirectoryExplorerTreeViewItem::colorChanged(ThemeManager::ColorRole role, Colour newColor)
{
	repaintItem();
}
