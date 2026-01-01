/*
  ==============================================================================

    SpliceImportDialog.h
    Created: 31 Dec 2025
    Author:  OpenCode
    
    Summary: Dialog for configuring Splice import settings

  ==============================================================================
*/

#ifndef SPLICEIMPORTDIALOG_H
#define SPLICEIMPORTDIALOG_H

#include "JuceHeader.h"
#include "ThemeManager.h"

namespace samplore
{
    /// Configuration for Splice import
    struct SpliceImportConfig
    {
        File spliceDatabasePath;
        File spliceInstallDirectory;
        bool addToDirectoryList = false;
    };
    
    /// Dialog for configuring Splice import settings
    class SpliceImportDialog : public Component, 
                               public Button::Listener,
                               public ThemeManager::Listener
    {
    public:
        SpliceImportDialog();
        ~SpliceImportDialog() override;
        
        void paint(Graphics& g) override;
        void resized() override;
        void buttonClicked(Button* button) override;
        
        //==================================================================
        // ThemeManager::Listener interface
        void themeChanged(ThemeManager::Theme newTheme) override;
        void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;
        
        /// Shows the dialog
        void show();
        
        /// Hides the dialog
        void hide();
        
        /// Callback when user confirms import
        std::function<void(const SpliceImportConfig&)> onConfirm;
        
        /// Callback when user cancels
        std::function<void()> onCancel;
        
    private:
        Label mTitleLabel;
        
        Label mDatabaseLabel;
        Label mDatabasePathLabel;
        TextButton mBrowseDatabaseButton;
        
        Label mInstallDirLabel;
        Label mInstallDirPathLabel;
        TextButton mBrowseInstallDirButton;
        
        ToggleButton mAddToLibraryCheckbox;
        
        TextButton mImportButton;
        TextButton mCancelButton;
        
        File mSelectedDatabasePath;
        File mSelectedInstallDir;
        
        void updateColors();
        void findDefaultPaths();
        File findSpliceDatabaseFile();
        File findSpliceInstallDirectory();
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpliceImportDialog)
    };
}

#endif
