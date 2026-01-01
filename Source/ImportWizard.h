/*
  ==============================================================================

    ImportWizard.h
    Created: 31 Dec 2025
    Author:  OpenCode

  ==============================================================================
*/

#ifndef IMPORTWIZARD_H
#define IMPORTWIZARD_H
#include "JuceHeader.h"
#include "ThemeManager.h"
#include "SpliceImportDialog.h"

namespace samplore
{
    /// Content view for importing samples with multiple import methods
    /// Manages different import mode views similar to how OverlayPanel works
    class ImportWizard : public Component, public Button::Listener, public ThemeManager::Listener
    {
    public:
        ImportWizard();
        ~ImportWizard() override;
        
        void paint(Graphics& g) override;
        void resized() override;
        void buttonClicked(Button* button) override;
        
        void themeChanged(ThemeManager::Theme newTheme) override;
        void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;
        
        /// Resets to the main menu view
        void showMainMenu();
        
    private:
        enum class View
        {
            MainMenu,
            SpliceImport,
            GeneralImport,
            ManualImport
        };
        
        void showView(View view);
        void updateColors();
        
        View mCurrentView = View::MainMenu;
        
        // Main menu components
        TextButton mSpliceImportButton;
        TextButton mGeneralImportButton;
        TextButton mManualImportButton;
        
        // Back button (shown in sub-views)
        TextButton mBackButton;
        
        // Import mode views
        SpliceImportDialog mSpliceImportView;
        
        // Placeholder view class for future import modes
        class PlaceholderView : public Component
        {
        public:
            PlaceholderView(const String& title) : mTitle(title) {}
            void paint(Graphics& g) override
            {
                auto& tm = ThemeManager::getInstance();
                g.fillAll(tm.getColorForRole(ThemeManager::ColorRole::Background));
                g.setColour(tm.getColorForRole(ThemeManager::ColorRole::TextSecondary));
                g.setFont(FontOptions(16.0f));
                g.drawText(mTitle + "\n(Coming Soon)", getLocalBounds(), Justification::centred);
            }
        private:
            String mTitle;
        };
        
        PlaceholderView mGeneralImportView;
        PlaceholderView mManualImportView;
    };
}
#endif
