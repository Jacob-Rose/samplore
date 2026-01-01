/*
  ==============================================================================

    SpliceImportDialog.cpp
    Created: 31 Dec 2025
    Author:  OpenCode

  ==============================================================================
*/

#include "SpliceImportDialog.h"

namespace samplore
{
    SpliceImportDialog::SpliceImportDialog()
    {
        // Setup title
        mTitleLabel.setText("Splice Import Configuration", dontSendNotification);
        mTitleLabel.setFont(Font(24.0f, Font::bold));
        mTitleLabel.setJustificationType(Justification::centred);
        addAndMakeVisible(mTitleLabel);
        
        // Database path section
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
        mInstallDirLabel.setText("Splice Install Directory:", dontSendNotification);
        mInstallDirLabel.setFont(Font(14.0f, Font::bold));
        addAndMakeVisible(mInstallDirLabel);
        
        mInstallDirPathLabel.setText("(not selected)", dontSendNotification);
        mInstallDirPathLabel.setFont(Font(12.0f));
        addAndMakeVisible(mInstallDirPathLabel);
        
        mBrowseInstallDirButton.setButtonText("Browse...");
        mBrowseInstallDirButton.addListener(this);
        addAndMakeVisible(mBrowseInstallDirButton);
        
        // Checkbox for adding to library
        mAddToLibraryCheckbox.setButtonText("Add Splice directory to sample library");
        mAddToLibraryCheckbox.setToggleState(true, dontSendNotification);
        addAndMakeVisible(mAddToLibraryCheckbox);
        
        // Action buttons
        mImportButton.setButtonText("Import");
        mImportButton.addListener(this);
        addAndMakeVisible(mImportButton);
        
        mCancelButton.setButtonText("Cancel");
        mCancelButton.addListener(this);
        addAndMakeVisible(mCancelButton);
        
        // Find default paths
        findDefaultPaths();
        
        // Register with theme manager
        ThemeManager::getInstance().addListener(this);
        updateColors();
        
        // Start hidden
        setVisible(false);
    }
    
    SpliceImportDialog::~SpliceImportDialog()
    {
        ThemeManager::getInstance().removeListener(this);
    }
    
    void SpliceImportDialog::paint(Graphics& g)
    {
        // Draw semi-transparent dark overlay background
        g.fillAll(Colour(0, 0, 0).withAlpha(0.75f));
        
        // Calculate the content panel bounds
        auto bounds = getLocalBounds();
        auto padding = 60;
        auto panelBounds = bounds.reduced(padding);
        
        // Draw shadow effect for depth
        g.setColour(Colours::black.withAlpha(0.3f));
        g.fillRoundedRectangle(panelBounds.toFloat().translated(0, 4), 12.0f);
        
        // Draw the main panel background
        auto bgColor = ThemeManager::getInstance().getColorForRole(ThemeManager::ColorRole::BackgroundSecondary);
        g.setColour(bgColor.brighter(0.1f));
        g.fillRoundedRectangle(panelBounds.toFloat(), 12.0f);
        
        // Draw subtle panel border
        g.setColour(ThemeManager::getInstance().getColorForRole(ThemeManager::ColorRole::Border).brighter(0.2f));
        g.drawRoundedRectangle(panelBounds.toFloat(), 12.0f, 1.0f);
    }
    
    void SpliceImportDialog::resized()
    {
        auto bounds = getLocalBounds();
        auto padding = 60;
        auto panelBounds = bounds.reduced(padding);
        auto contentBounds = panelBounds.reduced(30);
        
        // Title at the top
        mTitleLabel.setBounds(contentBounds.removeFromTop(40));
        contentBounds.removeFromTop(20); // spacing
        
        // Database section
        mDatabaseLabel.setBounds(contentBounds.removeFromTop(25));
        
        auto dbRow = contentBounds.removeFromTop(30);
        mBrowseDatabaseButton.setBounds(dbRow.removeFromRight(100));
        dbRow.removeFromRight(10); // spacing
        mDatabasePathLabel.setBounds(dbRow);
        
        contentBounds.removeFromTop(20); // spacing
        
        // Install directory section
        mInstallDirLabel.setBounds(contentBounds.removeFromTop(25));
        
        auto dirRow = contentBounds.removeFromTop(30);
        mBrowseInstallDirButton.setBounds(dirRow.removeFromRight(100));
        dirRow.removeFromRight(10); // spacing
        mInstallDirPathLabel.setBounds(dirRow);
        
        contentBounds.removeFromTop(20); // spacing
        
        // Checkbox
        mAddToLibraryCheckbox.setBounds(contentBounds.removeFromTop(30));
        
        contentBounds.removeFromTop(30); // spacing
        
        // Buttons at the bottom
        auto buttonRow = contentBounds.removeFromBottom(40);
        mCancelButton.setBounds(buttonRow.removeFromRight(120));
        buttonRow.removeFromRight(10); // spacing
        mImportButton.setBounds(buttonRow.removeFromRight(120));
    }
    
    void SpliceImportDialog::buttonClicked(Button* button)
    {
        if (button == &mBrowseDatabaseButton)
        {
            auto chooser = std::make_shared<FileChooser>("Select Splice Database File",
                                                          mSelectedDatabasePath.exists() ? mSelectedDatabasePath : File(),
                                                          "*.db");
            
            auto flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;
            
            chooser->launchAsync(flags, [this, chooser](const FileChooser& fc) {
                auto result = fc.getResult();
                if (result.existsAsFile())
                {
                    mSelectedDatabasePath = result;
                    mDatabasePathLabel.setText(mSelectedDatabasePath.getFullPathName(), dontSendNotification);
                }
            });
        }
        else if (button == &mBrowseInstallDirButton)
        {
            auto chooser = std::make_shared<FileChooser>("Select Splice Install Directory",
                                                          mSelectedInstallDir.exists() ? mSelectedInstallDir : File());
            
            auto flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;
            
            chooser->launchAsync(flags, [this, chooser](const FileChooser& fc) {
                auto result = fc.getResult();
                if (result.isDirectory())
                {
                    mSelectedInstallDir = result;
                    mInstallDirPathLabel.setText(mSelectedInstallDir.getFullPathName(), dontSendNotification);
                }
            });
        }
        else if (button == &mImportButton)
        {
            // Validate selections
            if (!mSelectedDatabasePath.existsAsFile())
            {
                AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                    "Invalid Database",
                    "Please select a valid Splice database file.",
                    "OK");
                return;
            }
            
            if (!mSelectedInstallDir.isDirectory())
            {
                AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                    "Invalid Directory",
                    "Please select a valid Splice install directory.",
                    "OK");
                return;
            }
            
            // Create config and call callback
            SpliceImportConfig config;
            config.spliceDatabasePath = mSelectedDatabasePath;
            config.spliceInstallDirectory = mSelectedInstallDir;
            config.addToDirectoryList = mAddToLibraryCheckbox.getToggleState();
            
            hide();
            
            if (onConfirm)
                onConfirm(config);
        }
        else if (button == &mCancelButton)
        {
            hide();
            
            if (onCancel)
                onCancel();
        }
    }
    
    void SpliceImportDialog::themeChanged(ThemeManager::Theme newTheme)
    {
        updateColors();
        repaint();
    }
    
    void SpliceImportDialog::colorChanged(ThemeManager::ColorRole role, Colour newColor)
    {
        updateColors();
        repaint();
    }
    
    void SpliceImportDialog::show()
    {
        setVisible(true);
        toFront(true);
    }
    
    void SpliceImportDialog::hide()
    {
        setVisible(false);
    }
    
    void SpliceImportDialog::updateColors()
    {
        auto& tm = ThemeManager::getInstance();
        
        // Update label colors
        auto textColor = tm.getColorForRole(ThemeManager::ColorRole::TextPrimary);
        mTitleLabel.setColour(Label::textColourId, textColor);
        mDatabaseLabel.setColour(Label::textColourId, textColor);
        mDatabasePathLabel.setColour(Label::textColourId, textColor.withAlpha(0.8f));
        mInstallDirLabel.setColour(Label::textColourId, textColor);
        mInstallDirPathLabel.setColour(Label::textColourId, textColor.withAlpha(0.8f));
        
        // Update button colors
        auto primaryColor = tm.getColorForRole(ThemeManager::ColorRole::AccentPrimary);
        
        for (auto* button : {&mBrowseDatabaseButton, &mBrowseInstallDirButton, &mImportButton, &mCancelButton})
        {
            button->setColour(TextButton::buttonColourId, primaryColor);
            button->setColour(TextButton::textColourOffId, textColor);
            button->setColour(TextButton::textColourOnId, textColor);
        }
        
        mAddToLibraryCheckbox.setColour(ToggleButton::textColourId, textColor);
    }
    
    void SpliceImportDialog::findDefaultPaths()
    {
        // Try to find default database path
        File dbPath = findSpliceDatabaseFile();
        if (dbPath.existsAsFile())
        {
            mSelectedDatabasePath = dbPath;
            mDatabasePathLabel.setText(dbPath.getFullPathName(), dontSendNotification);
        }
        
        // Try to find default install directory
        File installDir = findSpliceInstallDirectory();
        if (installDir.isDirectory())
        {
            mSelectedInstallDir = installDir;
            mInstallDirPathLabel.setText(installDir.getFullPathName(), dontSendNotification);
        }
    }
    
    File SpliceImportDialog::findSpliceDatabaseFile()
    {
        // Common locations for Splice database on different platforms
        Array<File> possibleLocations;
        
        #if JUCE_WINDOWS
            File appData = File::getSpecialLocation(File::userApplicationDataDirectory);
            possibleLocations.add(appData.getChildFile("Splice/sounds.db"));
            possibleLocations.add(appData.getChildFile("Splice/splice.db"));
        #elif JUCE_MAC
            File appSupport = File::getSpecialLocation(File::userApplicationDataDirectory);
            possibleLocations.add(appSupport.getChildFile("Splice/sounds.db"));
            possibleLocations.add(appSupport.getChildFile("Splice/splice.db"));
        #elif JUCE_LINUX
            File home = File::getSpecialLocation(File::userHomeDirectory);
            possibleLocations.add(home.getChildFile(".splice/sounds.db"));
            possibleLocations.add(home.getChildFile(".config/Splice/sounds.db"));
        #endif
        
        for (const auto& loc : possibleLocations)
        {
            if (loc.existsAsFile())
            {
                return loc;
            }
        }
        
        return File();
    }
    
    File SpliceImportDialog::findSpliceInstallDirectory()
    {
        // Common locations for Splice install directory
        Array<File> possibleLocations;
        
        #if JUCE_WINDOWS
            File userMusic = File::getSpecialLocation(File::userMusicDirectory);
            File userDocs = File::getSpecialLocation(File::userDocumentsDirectory);
            possibleLocations.add(userMusic.getChildFile("Splice"));
            possibleLocations.add(userDocs.getChildFile("Splice"));
            possibleLocations.add(File("C:/Program Files/Splice"));
        #elif JUCE_MAC
            File userMusic = File::getSpecialLocation(File::userMusicDirectory);
            File userDocs = File::getSpecialLocation(File::userDocumentsDirectory);
            possibleLocations.add(userMusic.getChildFile("Splice"));
            possibleLocations.add(userDocs.getChildFile("Splice"));
        #elif JUCE_LINUX
            File home = File::getSpecialLocation(File::userHomeDirectory);
            possibleLocations.add(home.getChildFile("Splice"));
            possibleLocations.add(home.getChildFile(".splice"));
        #endif
        
        for (const auto& loc : possibleLocations)
        {
            if (loc.isDirectory())
            {
                return loc;
            }
        }
        
        return File();
    }
}
