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

namespace samplore
{
    /// Overlay panel for importing samples with multiple import methods
    class ImportWizard : public Component, public Button::Listener, public ThemeManager::Listener
    {
    public:
        ImportWizard();
        ~ImportWizard() override;
        
        void paint(Graphics& g) override;
        void resized() override;
        void buttonClicked(Button* button) override;
        
        //==================================================================
        // ThemeManager::Listener interface
        void themeChanged(ThemeManager::Theme newTheme) override;
        void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;
        
        /// Shows the import wizard overlay
        void show();
        
        /// Hides the import wizard overlay
        void hide();
        
        /// Callback when close is requested
        std::function<void()> onClose;
        
        /// Callback when Splice Import is selected
        std::function<void()> onSpliceImport;
        
        /// Callback when General Import is selected
        std::function<void()> onGeneralImport;
        
        /// Callback when Manual Import is selected
        std::function<void()> onManualImport;
        
    private:
        Label mTitleLabel;
        TextButton mSpliceImportButton;
        TextButton mGeneralImportButton;
        TextButton mManualImportButton;
        TextButton mCloseButton;
        
        void updateColors();
    };
}
#endif
