/*
  ==============================================================================

    SpliceImportDialog.h
    Created: 31 Dec 2025
    Author:  OpenCode
    
    Summary: Dialog for configuring Splice import settings with progress

  ==============================================================================
*/

#ifndef SPLICEIMPORTDIALOG_H
#define SPLICEIMPORTDIALOG_H

#include "JuceHeader.h"
#include "ThemeManager.h"
#include "SpliceImportTask.h"
#include "SpliceImportConfig.h"

namespace samplore
{
    /// Dialog for configuring Splice import settings
    class SpliceImportDialog : public Component, 
                               public Button::Listener,
                               public Timer,
                               public ThemeManager::Listener
    {
    public:
        SpliceImportDialog();
        ~SpliceImportDialog() override;
        
        void paint(Graphics& g) override;
        void resized() override;
        void buttonClicked(Button* button) override;
        
        // Timer for updating progress
        void timerCallback() override;
        
        //==================================================================
        // ThemeManager::Listener interface
        void themeChanged(ThemeManager::Theme newTheme) override;
        void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;
        
        /// Shows the dialog
        void show();
        
        /// Hides the dialog
        void hide();
        
        /// Callback when import is complete (success, samplesImported)
        std::function<void(bool, int)> onImportComplete;
        
    private:
        enum class State
        {
            Configuring,
            Importing,
            Complete
        };
        
        void updateState(State newState);
        void startImport();
        void findDefaultPaths();
        File findSpliceDatabaseFile();
        File findSpliceInstallDirectory();
        void applyColorScheme();
        
        State mState = State::Configuring;
        
        Label mInstructionsLabel;
        Label mDatabaseLabel;
        Label mDatabasePathLabel;
        TextButton mBrowseDatabaseButton;
        Label mInstallDirLabel;
        Label mInstallDirPathLabel;
        TextButton mBrowseInstallDirButton;
        ToggleButton mAddToLibraryCheckbox;
        
        double mProgressValue = 0.0;
        ProgressBar mProgressBar;
        Label mProgressLabel;
        Label mProgressStatusLabel;
        
        TextButton mImportButton;
        TextButton mCancelButton;
        TextButton mCloseButton;
        
        File mSelectedDatabasePath;
        File mSelectedInstallDir;
        
        std::unique_ptr<SpliceImportTask> mImportTask;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpliceImportDialog)
    };
}

#endif
