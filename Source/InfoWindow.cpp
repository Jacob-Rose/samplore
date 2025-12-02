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

InfoWindow::InfoWindow() : DialogWindow("Information", ThemeManager::getInstance().getColorForRole(ThemeManager::ColorRole::Background), allButtons, true)
{
    setSize(400, 200);
    
    // Register with ThemeManager
    ThemeManager::getInstance().addListener(this);
}

InfoWindow::~InfoWindow()
{
    ThemeManager::getInstance().removeListener(this);
}

void InfoWindow::paint(Graphics& g)
{
    auto& theme = ThemeManager::getInstance();
    g.fillAll(theme.getColorForRole(ThemeManager::ColorRole::Background));
    g.setColour(theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    for (int i = 0; i < attributions.size(); i++)
    {
        g.drawText(attributions[i], Rectangle<float>(0, i * 30, getWidth(), 30), Justification::centred);
    }
}

void InfoWindow::closeButtonPressed()
{
    exitModalState(0);
}

//==============================================================================
// ThemeManager::Listener implementation
void InfoWindow::themeChanged(ThemeManager::Theme newTheme)
{
    auto& theme = ThemeManager::getInstance();
    setBackgroundColour(theme.getColorForRole(ThemeManager::ColorRole::Background));
    repaint();
}

void InfoWindow::colorChanged(ThemeManager::ColorRole role, Colour newColor)
{
    if (role == ThemeManager::ColorRole::Background || role == ThemeManager::ColorRole::TextPrimary)
    {
        repaint();
    }
}
