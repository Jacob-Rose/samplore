/*
  ==============================================================================

    InfoWindow.cpp
    Created: 14 Jun 2020 8:48:43pm
    Author:  jacob

  ==============================================================================
*/

#include "InfoWindow.h"
#include "SamplifyLookAndFeel.h"

using namespace samplore;

InfoWindow::InfoWindow() : DialogWindow("Information", AppValues::getInstance().MAIN_BACKGROUND_COLOR, allButtons, true)
{
    setSize(400, 200);
}

void InfoWindow::paint(Graphics& g)
{
    g.fillAll(getBackgroundColour());
    g.setColour(getBackgroundColour().getPerceivedBrightness() > 0.5 ? Colours::black : Colours::white);
    for (int i = 0; i < attributions.size(); i++)
    {
        g.drawText(attributions[i], Rectangle<float>(0, i * 30, getWidth(), 30), Justification::centred);
    }
}

void InfoWindow::closeButtonPressed()
{
    exitModalState(0);
}
