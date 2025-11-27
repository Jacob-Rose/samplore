/*
  ==============================================================================

	TagDrawer.h
	Author:  Jake Rose

  ==============================================================================
*/
#ifndef TAG_DRAWER_HEADER
#define TAG_DRAWER_HEADER

#include "JuceHeader.h"

#include <vector>
#include <string>

namespace samplore
{
	class TagDrawer
	{
	public:
		static void DrawTags(Graphics& g, std::vector<std::string> tags, Rectangle<float> bounds,
			float padding = 2.0f,
			float tagSpacerWidth = 3.0f);
		//can draw with any input, but use proper sizing to ensure proper drawing
		static void DrawTagBox(Graphics& g, std::string tag, Rectangle<float> bounds,
			float roundness = 0.5f,
			float thickness = 1.0f);
	};
}
#endif
