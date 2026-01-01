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
    /// Content view for importing samples with multiple import methods
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
        
        /// Callback when Splice Import is selected
        std::function<void()> onSpliceImport;
        
        /// Callback when General Import is selected
        std::function<void()> onGeneralImport;
        
        /// Callback when Manual Import is selected
        std::function<void()> onManualImport;
        
    private:
        TextButton mSpliceImportButton;
        TextButton mGeneralImportButton;
        TextButton mManualImportButton;
        
        void updateColors();
    };
}
#endif
