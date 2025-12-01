#include "TagDrawer.h"
#include "SamplifyProperties.h"

using namespace samplore;

void TagDrawer::DrawTags(Graphics& g, std::vector<std::string> tags, Rectangle<float> bounds, float padding, float tagSpacerWidth)
{
	float fontHeight = g.getCurrentFont().getHeight();
	float boxHeight = fontHeight + padding;
	float currentWidth = 0.0f;
	int line = 0;
	for (int i = 0; i < tags.size(); i++)
	{
		float fontWidth = g.getCurrentFont().getStringWidthFloat(tags[i]);
		float boxWidth = fontWidth + (padding * 2);
		if (currentWidth + boxWidth < bounds.getWidth())
		{
			//draw the box bih
			DrawTagBox(g, tags[i], 
				Rectangle<float>(currentWidth, line * boxHeight, boxWidth, boxHeight));

			currentWidth += fontWidth + tagSpacerWidth;
		}
		else
		{
			line++;
			currentWidth = 0.0f;
			i--; //redo this next line
			if ((fontHeight + tagSpacerWidth) * line > bounds.getHeight())
			{
				DrawTagBox(g,"...", 
					Rectangle<float>(currentWidth, line * boxHeight, boxWidth, boxHeight));
				line--; 
				break; //end the loop. saying the list is incomplete in a visually pleasing way
			}
		}
	}
}

void TagDrawer::DrawTagBox(Graphics& g,
	std::string tag, Rectangle<float> bounds,
	float roundness,
	float thickness)
{
	Colour color = SamplifyProperties::getInstance()->getSampleLibrary()->getTagColor(tag);
	g.setColour(color);
	//g.fillRoundedRectangle();
	g.fillRoundedRectangle(bounds, roundness);
	//g.drawRoundedRectangle();
	g.drawRoundedRectangle(bounds, roundness, thickness);
	//g.drawText();
	if (color.getBrightness() > 90)
	{
		g.setColour(Colours::black);
	}
	else
	{
		g.setColour(Colours::white);
	}
	g.drawText(tag, bounds, Justification::centred, true);
}


