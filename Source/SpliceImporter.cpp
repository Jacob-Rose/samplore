/*
  ==============================================================================

    SpliceImporter.cpp
    Created: 31 Dec 2025
    Author:  OpenCode

  ==============================================================================
*/

#include "SpliceImporter.h"
#include "SamplifyProperties.h"

namespace samplore
{
    SpliceImporter::SpliceImporter()
    {
        // Try to auto-find the Splice database
        mSpliceDatabasePath = findSpliceDatabaseFile();
    }
    
    SpliceImporter::~SpliceImporter()
    {
    }
    
    File SpliceImporter::findSpliceDatabaseFile()
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
                DBG("Found Splice database at: " + loc.getFullPathName());
                return loc;
            }
        }
        
        DBG("WARNING: Could not find Splice sounds.db file");
        return File();
    }
    
    void SpliceImporter::setSpliceDatabasePath(const File& dbPath)
    {
        if (dbPath.existsAsFile())
        {
            mSpliceDatabasePath = dbPath;
            DBG("Set Splice database path to: " + dbPath.getFullPathName());
        }
        else
        {
            DBG("WARNING: Splice database file does not exist: " + dbPath.getFullPathName());
        }
    }
    
    int SpliceImporter::importSpliceSamples(SampleLibrary& library)
    {
        if (!mSpliceDatabasePath.existsAsFile())
        {
            AlertWindow::showMessageBoxAsync(
                AlertWindow::WarningIcon,
                "Splice Database Not Found",
                "Could not find the Splice sounds.db file. Please select it manually.",
                "OK"
            );
            return 0;
        }
        
        // Open the Splice database
        if (!mOrganizer.openDatabase(mSpliceDatabasePath))
        {
            AlertWindow::showMessageBoxAsync(
                AlertWindow::WarningIcon,
                "Database Error",
                "Failed to open Splice database. Please check the file is valid.",
                "OK"
            );
            return 0;
        }
        
        // Create temporary output directory for organizing
        File tempDir = File::getSpecialLocation(File::tempDirectory)
            .getChildFile("samplore_splice_import");
        tempDir.createDirectory();
        
        DBG("Organizing Splice samples into tag-based structure...");
        
        // Run the organizer in append mode to preserve existing work
        OrganizeResult result = mOrganizer.organize(tempDir, true);
        
        if (!result.success)
        {
            AlertWindow::showMessageBoxAsync(
                AlertWindow::WarningIcon,
                "Organization Failed",
                "Failed to organize Splice samples: " + result.errorMessage,
                "OK"
            );
            return 0;
        }
        
        DBG("Organization complete. Processing " + String(result.tagsProcessed) + " tags, " + 
            String(result.shortcutsCreated) + " shortcuts created");
        
        // Now scan the output directory to find all samples and their tags
        int importedCount = 0;
        std::map<String, StringArray> sampleToTags; // Map of sample path to tags
        
        // Scan all tag directories
        DirectoryIterator dirIter(tempDir, false, "*", File::findDirectories);
        
        while (dirIter.next())
        {
            File tagDir = dirIter.getFile();
            String tagName = tagDir.getFileName();
            
            // Skip special directories
            if (tagName.startsWith("_"))
                continue;
            
            // Add this tag to the library
            library.addTag(tagName);
            
            // Scan files in this tag directory
            DirectoryIterator fileIter(tagDir, false, "*", File::findFiles);
            
            while (fileIter.next())
            {
                File shortcutFile = fileIter.getFile();
                
                // Try to resolve the shortcut/symlink to get the actual sample path
                File targetFile = mOrganizer.resolveShortcut(shortcutFile);
                
                if (targetFile.existsAsFile())
                {
                    String samplePath = targetFile.getFullPathName();
                    sampleToTags[samplePath].add(tagName);
                }
            }
        }
        
        DBG("Found " + String(sampleToTags.size()) + " unique samples with tags");
        
        // OPTIMIZATION: Build a set of unique parent directories first
        std::set<String> uniqueDirectories;
        for (const auto& pair : sampleToTags)
        {
            File sampleFile(pair.first);
            if (sampleFile.existsAsFile())
            {
                uniqueDirectories.insert(sampleFile.getParentDirectory().getFullPathName());
            }
        }
        
        DBG("Adding " + String(uniqueDirectories.size()) + " unique directories to library");
        
        // Add all unique directories to library first
        for (const auto& dirPath : uniqueDirectories)
        {
            File dir(dirPath);
            
            // Check if already in library
            bool alreadyInLibrary = false;
            for (const auto& existingDir : library.getDirectories())
            {
                if (dir == existingDir->getFile() || dir.isAChildOf(existingDir->getFile()))
                {
                    alreadyInLibrary = true;
                    break;
                }
            }
            
            if (!alreadyInLibrary)
            {
                library.addDirectory(dir);
            }
        }
        
        // Now get all samples from library ONCE
        DBG("Fetching all samples from library");
        auto allLibrarySamples = library.getAllSamplesInDirectories("", true);
        
        DBG("Applying tags to samples");
        // Process each sample - find it in the library samples list
        int processedCount = 0;
        for (const auto& pair : sampleToTags)
        {
            File sampleFile(pair.first);
            const StringArray& tags = pair.second;
            
            // Find this sample in the library
            Sample::Reference foundSample(nullptr);
            for (int i = 0; i < allLibrarySamples.size(); ++i)
            {
                if (!allLibrarySamples[i].isNull() && allLibrarySamples[i].getFile() == sampleFile)
                {
                    foundSample = allLibrarySamples[i];
                    break;
                }
            }
            
            // Apply tags if found
            if (!foundSample.isNull())
            {
                for (const auto& tag : tags)
                {
                    foundSample.addTag(tag);
                }
                importedCount++;
            }
            
            processedCount++;
            
            // THROTTLE: Sleep every 100 samples to avoid UI freeze
            if (processedCount % 100 == 0)
            {
                Thread::sleep(5);
                DBG("Tagged " + String(processedCount) + " of " + String(sampleToTags.size()) + " samples");
            }
        }
        
        // Close the database
        mOrganizer.closeDatabase();
        
        AlertWindow::showMessageBoxAsync(
            AlertWindow::InfoIcon,
            "Import Complete",
            "Imported and tagged " + String(importedCount) + " Splice samples.\n" +
            "Processed " + String(result.tagsProcessed) + " tags.",
            "OK"
        );
        
        return importedCount;
    }
}
