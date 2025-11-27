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

namespace samplore
{
	class InfoWindow : public DialogWindow
	{
	public:
		StringArray attributions = {
		"Info Icon made by bqlqn from www.flaticon.com",
		"Check Icon made by Pixel Perfect from www.flaticon.com",
		"Minus Icon made by Becris from www.flaticon.com",
		"Cross Icon made by xnimrodx from www.flaticon.com"
		};
		InfoWindow();
		void paint(Graphics& g) override;

		void closeButtonPressed() override;
	private:

	};
}
#endif
