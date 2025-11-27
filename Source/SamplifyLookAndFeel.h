/*
  ==============================================================================

    SamplifyColorPallete.h
    Created: 27 Jun 2019 8:23:08pm
    Author:  Jake Rose

  ==============================================================================
*/

#ifndef SAMPLIFYLOOKANDFEEL_H
#define SAMPLIFYLOOKANDFEEL_H

#include "JuceHeader.h"
#include "Fonts.h"

namespace samplore
{
	class AppValues
	{
	public:
		AppValues() {}
		static void initInstance();
		static void cleanupInstance();
		static AppValues& getInstance();
		Colour MAIN_BACKGROUND_COLOR = Colours::white;
		Colour MAIN_FOREGROUND_COLOR = Colours::blueviolet;

		int SAMPLE_TAG_FONT_SIZE = 14;
		float SAMPLE_TAG_TEXT_PADDING = 2.0f;

		float WINDOW_WIDTH = 800;
		float WINDOW_HEIGHT = 600;
		float SAMPLE_TILE_MIN_WIDTH = 150.0f;
		float SAMPLE_TILE_ASPECT_RATIO = 0.666f;
		float SAMPLE_TILE_CONTAINER_ITEM_PADDING = 2.0f;
		float SAMPLE_TILE_CORNER_RADIUS = 8.0f;
		float SAMPLE_TILE_OUTLINE_THICKNESS = 2.0f;

		//Thumbnail Properties
		float AUDIO_THUMBNAIL_LINE_GAP_WIDTH = 0.5f; //ratio based, not pixels
		float AUDIO_THUMBNAIL_LINE_FILL_WIDTH = 2.0f;
		int AUDIO_THUMBNAIL_LINE_COUNT = 60;
	int AUDIO_THUMBNAIL_LINE_COUNT_PLAYER = 120;

		bool FILTERWINDOWOPEN = true;
		bool DIRECTORYWINDOWOPEN = true;
		bool EXTENDEDSAMPLEWINDOWOPEN = true;

		bool RIGHTCLICKPLAYFROMPOINT = true;

		Drawable* getDrawable(String id);
		void loadDrawables();
		void updateDrawablesColors();
		void cleanupDrawables();
	private:
		std::map<String, std::unique_ptr<Drawable>> mDrawables;
		static std::unique_ptr<AppValues> instance;
	};

	const Font SAMPLE_TILE_TITLE_FONT = Font(Typeface::createSystemTypefaceFor(Fonts::AbelRegular_ttf, Fonts::AbelRegular_ttfSize)).withHeight(24.0f);
}
#endif
