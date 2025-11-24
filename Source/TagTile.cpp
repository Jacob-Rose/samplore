#include "TagTile.h"
#include "SamplifyProperties.h"
#include "SampleTile.h"
#include "SamplifyLookAndFeel.h"
#include "SamplifyMainComponent.h"

using namespace samplify;
TagTile::TagTile(juce::String tag, Font& font)
{
	mTag = tag;
	mFont = &font;
}

TagTile::~TagTile()
{
}

void TagTile::setTag(juce::String tag)
{
	mTag = tag;
	repaint();
}

void TagTile::paint (Graphics& g)
{
	if (mTag != "")
	{
		float cornerSize = 4.0f;
		Colour mainColor = SamplifyProperties::getInstance()->getSampleLibrary()->getTagColor(mTag);
		g.setColour(mainColor);
		g.fillRoundedRectangle(getLocalBounds().toFloat(), cornerSize);   // draw an outline around the component
		g.setColour(mainColor.darker());
		g.drawRoundedRectangle(getLocalBounds().toFloat(), cornerSize, 1.0f);
		float oldFontSize = g.getCurrentFont().getHeight();
		if (mainColor.getPerceivedBrightness() > 0.5f)
		{
			g.setColour(Colours::black);
		}
		else
		{
			g.setColour(Colours::white);
		}
		g.setFont(*mFont);
		g.drawText(mTag, getLocalBounds(), Justification::centred, true);
		g.setFont(oldFontSize);
	}
}

void TagTile::resized()
{
	//repainting is automatic
}


void TagTile::mouseDown(const MouseEvent& e)
{
	
}

void TagTile::mouseUp(const MouseEvent& e)
{
	if (!isDragAndDropActive())
	{
		if (e.mods.isLeftButtonDown())
		{
			//todo set sample container filter
			String text = SamplifyMainComponent::getInstance()->getSampleExplorer().getSearchBar().getText();
			SamplifyMainComponent::getInstance()->getSampleExplorer().getSearchBar().setText("#" + mTag);
		}
		else if (e.mods.isRightButtonDown())
		{
			PopupMenu menu;
			//TagContainer -> sampleTile?
			if (SampleTile* parent = dynamic_cast<SampleTile*>(getParentComponent()->getParentComponent()))
			{
				//if on a sample tile
				menu.addItem(1, "Edit Tag", false, false);
				menu.addItem(2, "Untag", true, false);
				String tagCopy = mTag;
				menu.showMenuAsync(PopupMenu::Options(), [parent, tagCopy](int selection)
				{
					if (selection == 2)
					{
						parent->getSample().removeTag(tagCopy);
						parent->repaint();
					}
				});
			}
			else
			{
				menu.addItem(1, "Edit Tag", false, false);
				menu.addItem(2, "Delete Tag (+ References)", true, false);
				menu.showMenuAsync(PopupMenu::Options(), [](int selection)
				{
					// Currently not implemented
				});
			}

		}
	}
}

void TagTile::mouseDrag(const MouseEvent& e)
{
	startDragging("Tags", this, juce::Image(), true);
}
