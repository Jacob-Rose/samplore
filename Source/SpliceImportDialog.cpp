/*
  ==============================================================================

    SpliceImportDialog.cpp
    Created: 31 Dec 2025
    Author:  OpenCode

  ==============================================================================
*/

#include "SpliceImportDialog.h"
#include "SampleLibrary.h"
#include "SamplifyProperties.h"

namespace samplore
{
    SpliceImportDialog::SpliceImportDialog()
        : mProgressBar(mProgressValue)
    {
        // Title
        mTitleLabel.setText("Splice Import", dontSendNotification);
        mTitleLabel.setFont(Font(24.0f, Font::bold));
        mTitleLabel.setJustificationType(Justification::centred);
        addAndMakeVisible(mTitleLabel);
        
        // Instructions
        mInstructionsLabel.setText(
            "To find sounds.db: Open Splice > Preferences > Utilities > Download Logs.\n"
            "Then navigate to your user folder to find sounds.db. Select your Splice library root folder (contains Samples/packs).",
            dontSendNotification);
        mInstructionsLabel.setFont(Font(11.0f, Font::italic));
        mInstructionsLabel.setJustificationType(Justification::centredLeft);
        mInstructionsLabel.setMinimumHorizontalScale(1.0f);
        addAndMakeVisible(mInstructionsLabel);
        
        // Database section
        mDatabaseLabel.setText("Splice Database:", dontSendNotification);
        mDatabaseLabel.setFont(Font(14.0f, Font::bold));
        addAndMakeVisible(mDatabaseLabel);
        
        mDatabasePathLabel.setText("(not selected)", dontSendNotification);
        mDatabasePathLabel.setFont(Font(12.0f));
        addAndMakeVisible(mDatabasePathLabel);
        
        mBrowseDatabaseButton.setButtonText("Browse...");
        mBrowseDatabaseButton.addListener(this);
        addAndMakeVisible(mBrowseDatabaseButton);
        
        // Install directory section
        mInstallDirLabel.setText("Splice Library Root (contains Samples/packs):", dontSendNotification);
        mInstallDirLabel.setFont(Font(14.0f, Font::bold));
        addAndMakeVisible(mInstallDirLabel);
        
        mInstallDirPathLabel.setText("(not selected)", dontSendNotification);
        mInstallDirPathLabel.setFont(Font(12.0f));
        addAndMakeVisible(mInstallDirPathLabel);
        
        mBrowseInstallDirButton.setButtonText("Browse...");
        mBrowseInstallDirButton.addListener(this);
        addAndMakeVisible(mBrowseInstallDirButton);
        
        // Checkbox
        mAddToLibraryCheckbox.setButtonText("Add Splice/Samples/packs directory to library");
        mAddToLibraryCheckbox.setToggleState(true, dontSendNotification);
        addAndMakeVisible(mAddToLibraryCheckbox);
        
        // Progress
        mProgressBar.setPercentageDisplay(false);
        mProgressBar.setVisible(false);
        addAndMakeVisible(mProgressBar);
        
        mProgressLabel.setText("Progress:", dontSendNotification);
        mProgressLabel.setFont(Font(14.0f, Font::bold));
        mProgressLabel.setVisible(false);
        addAndMakeVisible(mProgressLabel);
        
        mProgressStatusLabel.setText("Ready", dontSendNotification);
        mProgressStatusLabel.setFont(Font(12.0f));
        mProgressStatusLabel.setVisible(false);
        addAndMakeVisible(mProgressStatusLabel);
        
        // Buttons
        mImportButton.setButtonText("Start Import");
        mImportButton.addListener(this);
        addAndMakeVisible(mImportButton);
        
        mCancelButton.setButtonText("Cancel");
        mCancelButton.addListener(this);
        mCancelButton.setVisible(false);
        addAndMakeVisible(mCancelButton);
        
        mCloseButton.setButtonText("Close");
        mCloseButton.addListener(this);
        mCloseButton.setVisible(false);
        addAndMakeVisible(mCloseButton);
        
        // Find default paths
        findDefaultPaths();
        
        // Theme
        ThemeManager::getInstance().addListener(this);
        applyColorScheme();
        
        setVisible(false);
    }
    
    SpliceImportDialog::~SpliceImportDialog()
    {
        ThemeManager::getInstance().removeListener(this);
        stopTimer();
    }
    
    void SpliceImportDialog::paint(Graphics& g)
    {
        // Background is now handled by ImportWizard/OverlayPanel
        auto& tm = ThemeManager::getInstance();
        g.fillAll(tm.getColorForRole(ThemeManager::ColorRole::Background));
    }
    
    void SpliceImportDialog::resized()
    {
        auto bounds = getLocalBounds();
        auto contentBounds = bounds.reduced(20); // Less padding since we're in ImportWizard
        
        // Title
        mTitleLabel.setBounds(contentBounds.removeFromTop(40));
        contentBounds.removeFromTop(10);
        
        // Instructions
        mInstructionsLabel.setBounds(contentBounds.removeFromTop(35));
        contentBounds.removeFromTop(15);
        
        // Database
        mDatabaseLabel.setBounds(contentBounds.removeFromTop(25));
        auto row = contentBounds.removeFromTop(30);
        mBrowseDatabaseButton.setBounds(row.removeFromRight(100));
        row.removeFromRight(10);
        mDatabasePathLabel.setBounds(row);
        contentBounds.removeFromTop(15);
        
        // Install dir
        mInstallDirLabel.setBounds(contentBounds.removeFromTop(25));
        row = contentBounds.removeFromTop(30);
        mBrowseInstallDirButton.setBounds(row.removeFromRight(100));
        row.removeFromRight(10);
        mInstallDirPathLabel.setBounds(row);
        contentBounds.removeFromTop(15);
        
        // Checkbox
        mAddToLibraryCheckbox.setBounds(contentBounds.removeFromTop(30));
        contentBounds.removeFromTop(15);
        
        // Progress (if visible)
        if (mState != State::Configuring)
        {
            mProgressLabel.setBounds(contentBounds.removeFromTop(25));
            mProgressBar.setBounds(contentBounds.removeFromTop(25));
            mProgressStatusLabel.setBounds(contentBounds.removeFromTop(25));
            contentBounds.removeFromTop(15);
        }
        
        contentBounds.removeFromTop(20);
        
        // Buttons
        auto buttonRow = contentBounds.removeFromBottom(40);
        mCloseButton.setBounds(buttonRow.removeFromRight(120));
        buttonRow.removeFromRight(10);
        mCancelButton.setBounds(buttonRow.removeFromRight(120));
        buttonRow.removeFromRight(10);
        mImportButton.setBounds(buttonRow.removeFromRight(120));
    }
    
    void SpliceImportDialog::buttonClicked(Button* button)
    {
        if (button == &mBrowseDatabaseButton)
        {
            auto chooser = std::make_shared<FileChooser>("Select Splice Database",
                               mSelectedDatabasePath.exists() ? mSelectedDatabasePath : File(),
                               "*.db");
            
            chooser->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                [this, chooser](const FileChooser& fc) {
                    if (fc.getResult().existsAsFile())
                    {
                        mSelectedDatabasePath = fc.getResult();
                        mDatabasePathLabel.setText(mSelectedDatabasePath.getFullPathName(), dontSendNotification);
                    }
                });
        }
        else if (button == &mBrowseInstallDirButton)
        {
            auto chooser = std::make_shared<FileChooser>("Select Splice Directory",
                               mSelectedInstallDir.exists() ? mSelectedInstallDir : File());
            
            chooser->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories,
                [this, chooser](const FileChooser& fc) {
                    if (fc.getResult().isDirectory())
                    {
                        mSelectedInstallDir = fc.getResult();
                        mInstallDirPathLabel.setText(mSelectedInstallDir.getFullPathName(), dontSendNotification);
                    }
                });
        }
        else if (button == &mImportButton)
        {
            if (!mSelectedDatabasePath.existsAsFile())
            {
                AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                    "Invalid Database", "Please select a valid Splice database file.", "OK");
                return;
            }
            
            if (!mSelectedInstallDir.isDirectory())
            {
                AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                    "Invalid Directory", "Please select a valid Splice directory.", "OK");
                return;
            }
            
            startImport();
        }
        else if (button == &mCancelButton)
        {
            if (mImportTask)
            {
                mCancelButton.setEnabled(false);
                mProgressStatusLabel.setText("Cancelling...", dontSendNotification);
                
                // Signal cancellation
                mImportTask->cancel();
                
                // Don't wait here - the timer will detect completion
            }
        }
        else if (button == &mCloseButton)
        {
            hide();
            if (mImportTask && onImportComplete)
                onImportComplete(mImportTask->wasSuccessful(), mImportTask->getSamplesImported());
        }
    }
    
    void SpliceImportDialog::startImport()
    {
        updateState(State::Importing);
        
        SpliceImportConfig config;
        config.spliceDatabasePath = mSelectedDatabasePath;
        config.spliceInstallDirectory = mSelectedInstallDir;
        config.addToDirectoryList = mAddToLibraryCheckbox.getToggleState();
        
        auto library = SamplifyProperties::getInstance()->getSampleLibrary();
        if (!library)
        {
            AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                "Error", "Sample library not available.", "OK");
            updateState(State::Configuring);
            return;
        }
        
        mImportTask = std::make_unique<SpliceImportTask>(config, *library);
        startTimer(100);
        mImportTask->startThread();
    }
    
    void SpliceImportDialog::updateState(State newState)
    {
        mState = newState;
        
        bool importing = (newState == State::Importing);
        bool complete = (newState == State::Complete);
        
        mProgressLabel.setVisible(importing || complete);
        mProgressBar.setVisible(importing || complete);
        mProgressStatusLabel.setVisible(importing || complete);
        mImportButton.setVisible(!importing);
        mCancelButton.setVisible(importing);
        mCloseButton.setVisible(complete);
        
        if (!importing)
        {
            mImportButton.setEnabled(true);
            mCancelButton.setEnabled(true);
        }
        
        resized();
    }
    
    void SpliceImportDialog::timerCallback()
    {
        if (mState == State::Importing)
        {
            if (!mImportTask)
                return;
            
            // Cache progress from background thread
            mImportTask->cacheProgress();
            
            // Update progress
            mProgressValue = mImportTask->getProgress();
            
            // Update status with x/y count
            int current = mImportTask->getCurrentCount();
            int total = mImportTask->getTotalCount();
            String baseStatus = mImportTask->getCurrentStatus();
            
            // Add x/y if we have counts
            if (total > 0)
            {
                mProgressStatusLabel.setText(baseStatus + " (" + String(current) + "/" + String(total) + ")", 
                                            dontSendNotification);
            }
            else
            {
                mProgressStatusLabel.setText(baseStatus, dontSendNotification);
            }
            
            // Force progress bar to repaint
            mProgressBar.repaint();
            
            if (mImportTask->isComplete())
            {
                stopTimer();
                
                // Wait for thread to finish before transitioning state
                mImportTask->stopThread(5000);
                
                updateState(State::Complete);
                
                mProgressStatusLabel.setText(
                    mImportTask->wasSuccessful() 
                        ? "Complete! Imported " + String(mImportTask->getSamplesImported()) + " samples."
                        : "Cancelled",
                    dontSendNotification);
                
                startTimer(3000);
            }
        }
        else if (mState == State::Complete)
        {
            // Auto-close after showing completion for 3 seconds
            stopTimer();
            hide();
            if (mImportTask && onImportComplete)
                onImportComplete(mImportTask->wasSuccessful(), mImportTask->getSamplesImported());
        }
    }
    
    void SpliceImportDialog::show()
    {
        updateState(State::Configuring);
        setVisible(true);
        toFront(true);
    }
    
    void SpliceImportDialog::hide()
    {
        stopTimer();
        
        // Make sure thread is stopped before hiding
        if (mImportTask && mImportTask->isThreadRunning())
        {
            mImportTask->stopThread(5000);
        }
        
        setVisible(false);
    }
    
    void SpliceImportDialog::findDefaultPaths()
    {
        // Database
        File dbPath = findSpliceDatabaseFile();
        if (dbPath.existsAsFile())
        {
            mSelectedDatabasePath = dbPath;
            mDatabasePathLabel.setText(dbPath.getFullPathName(), dontSendNotification);
        }
        
        // Install dir
        File installDir = findSpliceInstallDirectory();
        if (installDir.isDirectory())
        {
            mSelectedInstallDir = installDir;
            mInstallDirPathLabel.setText(installDir.getFullPathName(), dontSendNotification);
        }
    }
    
    File SpliceImportDialog::findSpliceDatabaseFile()
    {
        Array<File> locations;
        
    #if JUCE_WINDOWS
        File appData = File::getSpecialLocation(File::userApplicationDataDirectory);
        locations.add(appData.getChildFile("Splice/sounds.db"));
    #elif JUCE_MAC
        File appSupport = File::getSpecialLocation(File::userApplicationDataDirectory);
        locations.add(appSupport.getChildFile("Splice/sounds.db"));
    #elif JUCE_LINUX
        File home = File::getSpecialLocation(File::userHomeDirectory);
        locations.add(home.getChildFile(".splice/sounds.db"));
    #endif
        
        for (const auto& loc : locations)
            if (loc.existsAsFile())
                return loc;
        
        return File();
    }
    
    File SpliceImportDialog::findSpliceInstallDirectory()
    {
        Array<File> locations;
        
    #if JUCE_WINDOWS
        File userMusic = File::getSpecialLocation(File::userMusicDirectory);
        locations.add(userMusic.getChildFile("Splice"));
    #elif JUCE_MAC
        File userMusic = File::getSpecialLocation(File::userMusicDirectory);
        locations.add(userMusic.getChildFile("Splice"));
    #elif JUCE_LINUX
        File home = File::getSpecialLocation(File::userHomeDirectory);
        locations.add(home.getChildFile("Splice"));
    #endif
        
        for (const auto& loc : locations)
            if (loc.isDirectory())
                return loc;
        
        return File();
    }
    
    void SpliceImportDialog::applyColorScheme()
    {
        auto& tm = ThemeManager::getInstance();
        auto textColor = tm.getColorForRole(ThemeManager::ColorRole::TextPrimary);
        auto primaryColor = tm.getColorForRole(ThemeManager::ColorRole::AccentPrimary);
        
        mTitleLabel.setColour(Label::textColourId, textColor);
        mInstructionsLabel.setColour(Label::textColourId, textColor.withAlpha(0.7f));
        mDatabaseLabel.setColour(Label::textColourId, textColor);
        mDatabasePathLabel.setColour(Label::textColourId, textColor.withAlpha(0.8f));
        mInstallDirLabel.setColour(Label::textColourId, textColor);
        mInstallDirPathLabel.setColour(Label::textColourId, textColor.withAlpha(0.8f));
        mProgressLabel.setColour(Label::textColourId, textColor);
        mProgressStatusLabel.setColour(Label::textColourId, textColor.withAlpha(0.8f));
        
        for (auto* btn : {&mBrowseDatabaseButton, &mBrowseInstallDirButton, &mImportButton, &mCancelButton, &mCloseButton})
        {
            btn->setColour(TextButton::buttonColourId, primaryColor);
            btn->setColour(TextButton::textColourOffId, textColor);
        }
        
        mAddToLibraryCheckbox.setColour(ToggleButton::textColourId, textColor);
    }
    
    void SpliceImportDialog::themeChanged(ThemeManager::Theme)
    {
        applyColorScheme();
        repaint();
    }
    
    void SpliceImportDialog::colorChanged(ThemeManager::ColorRole, Colour)
    {
        applyColorScheme();
        repaint();
    }
}
