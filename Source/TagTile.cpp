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
						parent->refreshTags();
					}
				});
			}
			else
			{
				menu.addItem(1, "Edit Tag", false, false);
				menu.addItem(2, "Delete Tag (+ References)", true, false);

				// Build "Move to Collection" submenu
				PopupMenu collectionMenu;
				auto library = SamplifyProperties::getInstance()->getSampleLibrary();
				StringArray collections = library->getCollections();

				// Add existing collections
				for (int i = 0; i < collections.size(); i++)
				{
					collectionMenu.addItem(100 + i, collections[i]);
				}

				// Add "Default" option
				collectionMenu.addItem(99, "Default");

				// Separator and "New Collection..." option
				collectionMenu.addSeparator();
				collectionMenu.addItem(98, "New Collection...");

				menu.addSubMenu("Move to Collection", collectionMenu);

				String tagCopy = mTag;
				menu.showMenuAsync(PopupMenu::Options(), [tagCopy, collections](int selection)
				{
					auto lib = SamplifyProperties::getInstance()->getSampleLibrary();

					if (selection == 2)
					{
						// Delete tag
						lib->deleteTag(tagCopy);
					}
					else if (selection == 99)
					{
						// Move to Default collection
						lib->setTagCollection(tagCopy, "");
					}
					else if (selection == 98)
					{
						// Create new collection and move tag to it
						// Use shared_ptr to manage AlertWindow lifetime - don't let JUCE delete it (last param = false)
						auto alertWindow = std::make_shared<AlertWindow>("New Collection", "", MessageBoxIconType::NoIcon);
						alertWindow->addTextEditor("collectionName", "", "Collection Name:");
						alertWindow->addButton("OK", 1, KeyPress(KeyPress::returnKey));
						alertWindow->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

						alertWindow->enterModalState(true, ModalCallbackFunction::create([alertWindow, tagCopy](int result)
						{
							if (result == 1)
							{
								String collectionName = alertWindow->getTextEditorContents("collectionName");
								if (collectionName.isNotEmpty())
								{
									auto lib = SamplifyProperties::getInstance()->getSampleLibrary();
									lib->addCollection(collectionName);
									lib->setTagCollection(tagCopy, collectionName);
								}
							}
							// shared_ptr will clean up AlertWindow when lambda is destroyed
						}), false);  // false = don't delete component, shared_ptr handles it
					}
					else if (selection >= 100 && selection < 100 + collections.size())
					{
						// Move to selected collection
						String collectionName = collections[selection - 100];
						lib->setTagCollection(tagCopy, collectionName);
					}
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
