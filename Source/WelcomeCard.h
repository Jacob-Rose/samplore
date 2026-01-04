/*
  ==============================================================================

    WelcomeCard.h
    Author:  Jake Rose

    A welcome/intro card that provides a light tutorial for new users.
    Shows on first startup and can be accessed via Info -> Intro Card menu.

  ==============================================================================
*/

#ifndef WELCOMECARD_H
#define WELCOMECARD_H

#include "JuceHeader.h"
#include "UI/IOverlayPanelContent.h"
#include "ThemeManager.h"

namespace samplore
{
    class WelcomeCard : public Component,
                        public IOverlayPanelContent,
                        public ThemeManager::Listener
    {
    public:
        WelcomeCard();
        ~WelcomeCard() override;

        // Component overrides
        void paint(Graphics& g) override;
        void resized() override;

        // IOverlayPanelContent interface
        String getOverlayTitle() const override;
        bool shouldShowBackButton() const override;
        void onOverlayBackButton() override;
        void setParentOverlay(OverlayPanel* parent) override;

        // ThemeManager::Listener interface
        void themeChanged(ThemeManager::Theme newTheme) override;
        void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;

        /// Resets the card to the first page
        void resetToStart();

    private:
        /// Holds content for a single tutorial page
        struct TutorialPage
        {
            String title;
            String content;
        };

        std::vector<TutorialPage> mPages;
        int mCurrentPage = 0;

        TextButton mNextButton;
        TextButton mPrevButton;
        TextButton mGetStartedButton;

        OverlayPanel* mParentOverlay = nullptr;

        void setupPages();
        void updateButtonVisibility();
        void nextPage();
        void prevPage();
        void finish();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WelcomeCard)
    };
}

#endif
