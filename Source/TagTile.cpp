#include "TagTile.h"
#include "SamplifyProperties.h"
#include "SampleTile.h"
#include "SamplifyLookAndFeel.h"
#include "SamplifyMainComponent.h"
#include "ThemeManager.h"
#include "PerformanceProfiler.h"

using namespace samplore;
TagTile::TagTile(juce::String tag, Font& font)
{
	mTag = tag;
	mFont = &font;
	
	// Register with ThemeManager
	ThemeManager::getInstance().addListener(this);
	
	// CRITICAL OPTIMIZATION: Cache tag tiles as images
	// Tags are static content, perfect for buffering
	// Reduces 42k+ paint calls to only when mouse enter/exit or content changes
	setBufferedToImage(true);
}

TagTile::~TagTile()
{
	ThemeManager::getInstance().removeListener(this);
}

void TagTile::setTag(juce::String tag)
{
	mTag = tag;
	repaint();
}

void TagTile::paint (Graphics& g)
{
	PROFILE_PAINT("TagTile::paint");
	
	if (mTag != "")
	{
		auto& theme = ThemeManager::getInstance();
		const float cornerSize = 6.0f;
		// Match padding from TagContainer calculation (SAMPLE_TAG_TEXT_PADDING)
		const int padding = static_cast<int>(AppValues::getInstance().SAMPLE_TAG_TEXT_PADDING);
		bool isHovered = isMouseOver(true);

		Colour mainColor = SamplifyProperties::getInstance()->getSampleLibrary()->getTagColor(mTag);

		// Use semi-transparent background with higher opacity on hover
		float alpha = isHovered ? 0.95f : 0.85f;
		g.setColour(mainColor.withAlpha(alpha));
		g.fillRoundedRectangle(getLocalBounds().toFloat(), cornerSize);

		// Subtle border using darker shade of the tag color
		g.setColour(mainColor.darker(0.3f).withAlpha(0.7f));
		g.drawRoundedRectangle(getLocalBounds().toFloat(), cornerSize, 1.0f);

		// Text color: use contrasting color for readability
		// With our fixed saturation (0.65) and brightness (0.85), most colors are fairly bright
		// Use dark text for light backgrounds, white for dark backgrounds
		Colour textColor;
		float luminance = mainColor.getPerceivedBrightness();
		if (luminance > 0.55f)
		{
			// Light background: use dark text with slight transparency for softer look
			textColor = Colours::black.withAlpha(0.85f);
		}
		else
		{
			// Dark background: use white text
			textColor = Colours::white.withAlpha(0.95f);
		}

		g.setColour(textColor);
		g.setFont(*mFont);
		auto textBounds = getLocalBounds().reduced(padding, 2);
		// Draw full text without truncation (last param = false)
		g.drawText(mTag, textBounds, Justification::centred, false);
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
			// Toggle this tag in the active filter
			SamplifyMainComponent::getInstance()->getSampleExplorer().toggleActiveTag(mTag);
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
	startDragging("Tags", this, juce::ScaledImage(juce::Image()), true, nullptr, nullptr);
}

void TagTile::mouseEnter(const MouseEvent& e)
{
	repaint();
}

void TagTile::mouseExit(const MouseEvent& e)
{
	repaint();
}

//==============================================================================
// ThemeManager::Listener implementation
void TagTile::themeChanged(ThemeManager::Theme newTheme)
{
	repaint();
}

void TagTile::colorChanged(ThemeManager::ColorRole role, Colour newColor)
{
	repaint();
}
