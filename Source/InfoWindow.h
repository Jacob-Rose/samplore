/*
  ==============================================================================

    InfoWindow.h
    Created: 14 Jun 2020 8:48:43pm
    Author:  jacob

  ==============================================================================
*/

#ifndef INFOWINDOW_H
#define INFOWINDOW_H
#include "JuceHeader.h"
#include "ThemeManager.h"

namespace samplore
{
	class InfoWindow : public DialogWindow, public ThemeManager::Listener
	{
	public:
		StringArray attributions = {
		"Info Icon made by bqlqn from www.flaticon.com",
		"Check Icon made by Pixel Perfect from www.flaticon.com",
		"Minus Icon made by Becris from www.flaticon.com",
		"Cross Icon made by xnimrodx from www.flaticon.com"
		};
		InfoWindow();
		~InfoWindow() override;
		void paint(Graphics& g) override;

		void closeButtonPressed() override;
		
		//==================================================================
		// ThemeManager::Listener interface
		void themeChanged(ThemeManager::Theme newTheme) override;
		void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;
		
	private:

	};
}
#endif
