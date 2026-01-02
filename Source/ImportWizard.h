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
#include "UI/IOverlayPanelContent.h"

namespace samplore
{
    /// Content view for importing samples with multiple import methods
    /// Implements IOverlayPanelContent to provide title and back button state
    class ImportWizard : public Component, 
                         public Button::Listener, 
                         public ThemeManager::Listener,
                         public IOverlayPanelContent
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
        
        //==================================================================
        // IOverlayPanelContent interface
        String getOverlayTitle() const override;
        bool shouldShowBackButton() const override;
        void onOverlayBackButton() override;
        void setParentOverlay(OverlayPanel* parent) override;
        
    private:
        enum class View
        {
            MainMenu,
            SpliceImport,
            GeneralImport,
            ManualImport
        };
        
        /// Returns the title for a given view
        String getTitleForView(View view) const;
        
        void showView(View view);
        void updateColors();
        
        View mCurrentView = View::MainMenu;
        
        // View titles (stored in a map for easy access)
        static const std::map<View, String> sViewTitles;
        
        // Parent overlay panel (for requesting chrome refresh)
        OverlayPanel* mParentOverlay = nullptr;
        
        // Main menu components
        TextButton mSpliceImportButton;
        TextButton mGeneralImportButton;
        TextButton mManualImportButton;
        
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
