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
        // Create log file for detailed import diagnostics in app data folder
        File appDataDir = File::getSpecialLocation(File::userApplicationDataDirectory)
            .getChildFile("Samplore");
        appDataDir.createDirectory();
        
        File logFile = appDataDir.getChildFile("SpliceImport_Log.txt");
        
        // Delete old log if it exists
        if (logFile.exists())
        {
            logFile.deleteFile();
        }
        
        // Create new log file
        logFile.create();
        
        FileOutputStream logStream(logFile);
        
        auto LOG = [&logStream, &logFile](const String& message) {
            DBG(message);
            if (logStream.openedOk())
            {
                logStream.writeText(message + "\n", false, false, nullptr);
                logStream.flush();
            }
        };
        
        if (!logStream.openedOk())
        {
            DBG("WARNING: Could not create log file at: " + logFile.getFullPathName());
        }
        else
        {
            DBG("Log file created at: " + logFile.getFullPathName());
        }
        
        LOG("=== Samplore Splice Import Log ===");
        LOG("Timestamp: " + Time::getCurrentTime().toString(true, true));
        LOG("Log file location: " + logFile.getFullPathName());
        LOG("");
        LOG("=== STARTING SPLICE IMPORT ===");
        LOG("Database path: " + mSpliceDatabasePath.getFullPathName());
        
        if (!mSpliceDatabasePath.existsAsFile())
        {
            LOG("ERROR: Database file not found!");
            AlertWindow::showMessageBoxAsync(
                AlertWindow::WarningIcon,
                "Splice Database Not Found",
                "Could not find the Splice sounds.db file. Please select it manually.",
                "OK"
            );
            return 0;
        }
        
        // Open the Splice database
        LOG("Opening Splice database...");
        if (!mOrganizer.openDatabase(mSpliceDatabasePath))
        {
            LOG("ERROR: Failed to open database!");
            AlertWindow::showMessageBoxAsync(
                AlertWindow::WarningIcon,
                "Database Error",
                "Failed to open Splice database. Please check the file is valid.",
                "OK"
            );
            return 0;
        }
        LOG("Database opened successfully");
        
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
        
        // Get ALL samples with their tags from the database in one efficient query
        LOG("\n=== QUERYING DATABASE ===");
        LOG("Querying database for all samples and their tags...");
        std::map<String, StringArray> sampleToTags = mOrganizer.getAllSamplesWithTags();
        
        LOG("Found " + String(sampleToTags.size()) + " unique samples with tags from database");
        
        // Log first few samples for debugging
        LOG("\nFirst 10 samples from database:");
        int sampleCount = 0;
        for (const auto& pair : sampleToTags)
        {
            if (sampleCount < 10)
            {
                LOG("  Sample: " + pair.first);
                LOG("  Tags (" + String(pair.second.size()) + "): " + pair.second.joinIntoString(", "));
            }
            sampleCount++;
        }
        
        // Collect all unique tags and add them to the library
        std::set<String> uniqueTags;
        for (const auto& pair : sampleToTags)
        {
            for (const auto& tag : pair.second)
            {
                if (tag.isNotEmpty())
                {
                    uniqueTags.insert(tag);
                }
            }
        }
        
        LOG("\n=== ADDING TAGS TO LIBRARY ===");
        LOG("Found " + String(uniqueTags.size()) + " unique tags");
        
        // Convert set to array and log first 50 tags
        StringArray tagArray;
        for (const auto& tag : uniqueTags)
        {
            tagArray.add(tag);
        }
        
        LOG("First 50 tags: " + tagArray.joinIntoString(", ", 0, 50));
        
        // DIAGNOSTIC: Write tag list to app data folder for easy inspection
        DBG("AppData directory: " + appDataDir.getFullPathName());
        DBG("AppData exists: " + String(appDataDir.exists() ? "YES" : "NO"));
        
        File tagListFile = appDataDir.getChildFile("SpliceImport_TagsFound.txt");
        DBG("Tag file path: " + tagListFile.getFullPathName());
        
        String tagFileContents = "Samplore Splice Import - Tags Found\n";
        tagFileContents += "Timestamp: " + Time::getCurrentTime().toString(true, true) + "\n";
        tagFileContents += "Total unique tags: " + String(uniqueTags.size()) + "\n\n";
        tagFileContents += tagArray.joinIntoString("\n");
        
        bool writeSuccess = tagListFile.replaceWithText(tagFileContents);
        DBG("Tag file write success: " + String(writeSuccess ? "YES" : "NO"));
        DBG("Tag file exists after write: " + String(tagListFile.exists() ? "YES" : "NO"));
        DBG("Tag file size: " + String(tagListFile.getSize()) + " bytes");
        
        for (const auto& tag : uniqueTags)
        {
            library.addTag(tag);
        }
        
        int importedCount = 0;
        
        // Find the common root directory for all samples to avoid adding hundreds of leaf directories
        LOG("\n=== FINDING COMMON ROOT DIRECTORY ===");
        File commonRoot;
        std::set<String> allPaths;
        int nonExistentCount = 0;
        
        for (const auto& pair : sampleToTags)
        {
            File sampleFile(pair.first);
            if (sampleFile.existsAsFile())
            {
                allPaths.insert(sampleFile.getFullPathName());
            }
            else
            {
                nonExistentCount++;
                // Log first few non-existent files
                if (nonExistentCount <= 10)
                {
                    LOG("WARNING: Sample file does not exist: " + pair.first);
                }
            }
        }
        
        if (nonExistentCount > 0)
        {
            LOG("Total non-existent files: " + String(nonExistentCount));
        }
        LOG("Files that exist: " + String(allPaths.size()));
        
        if (!allPaths.empty())
        {
            // Get first path as starting point
            String firstPath = *allPaths.begin();
            File currentFile(firstPath);
            
            // Walk up the directory tree to find a common ancestor
            // Typically Splice stores samples in a structure like:
            // /Users/name/Music/Splice/samples/... (Mac)
            // C:\Users\name\Documents\Splice\samples\... (Windows)
            
            // Start with parent directory of first sample
            commonRoot = currentFile.getParentDirectory();
            
            // Keep going up until we find a directory that contains "splice" in its name
            // or we reach a reasonable depth (to avoid going to root)
            int maxDepth = 10;
            int depth = 0;
            
            while (depth < maxDepth && commonRoot != File())
            {
                String dirName = commonRoot.getFileName().toLowerCase();
                
                // If we find a "Splice" or "samples" directory, use it as root
                if (dirName.contains("splice") || dirName == "samples")
                {
                    break;
                }
                
                commonRoot = commonRoot.getParentDirectory();
                depth++;
            }
            
            LOG("Found common root directory: " + commonRoot.getFullPathName());
        }
        else
        {
            LOG("WARNING: Could not determine common root directory!");
        }
        
        // Check if common root is already in library or is a child of existing directory
        LOG("\n=== ADDING DIRECTORY TO LIBRARY ===");
        bool alreadyInLibrary = false;
        for (const auto& existingDir : library.getDirectories())
        {
            if (commonRoot == existingDir->getFile() || commonRoot.isAChildOf(existingDir->getFile()))
            {
                alreadyInLibrary = true;
                LOG("Common root already in library or is child of existing directory: " + existingDir->getFile().getFullPathName());
                break;
            }
        }
        
        if (!alreadyInLibrary && commonRoot != File())
        {
            LOG("Adding common root directory to library: " + commonRoot.getFullPathName());
            library.addDirectory(commonRoot);
        }
        else if (alreadyInLibrary)
        {
            LOG("Skipping - directory already in library");
        }
        
        // Now get all samples from library ONCE
        LOG("\n=== FETCHING SAMPLES FROM LIBRARY ===");
        LOG("Fetching all samples from library...");
        auto allLibrarySamples = library.getAllSamplesInDirectories("", true);
        LOG("Library contains " + String(allLibrarySamples.size()) + " total samples");
        
        LOG("\n=== APPLYING TAGS TO SAMPLES ===");
        LOG("Matching database samples to library samples and applying tags...");
        // Process each sample - find it in the library samples list
        int processedCount = 0;
        int notFoundCount = 0;
        int generatedPropertiesCount = 0;
        int generatedThumbnailsCount = 0;
        
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
                // Check if properties file exists/is valid
                bool needsPropertiesGeneration = !foundSample.isPropertiesFileValid();
                
                // Generate thumbnail if not already created (this also creates properties file if needed)
                if (foundSample.getThumbnail() == nullptr)
                {
                    foundSample.generateThumbnailAndCache();
                    generatedThumbnailsCount++;
                    
                    // Only count properties generation if it was actually needed
                    if (needsPropertiesGeneration)
                    {
                        generatedPropertiesCount++;
                    }
                }
                else if (needsPropertiesGeneration)
                {
                    // Thumbnail exists but properties file doesn't - this shouldn't happen normally
                    // but handle it by creating an empty properties file
                    generatedPropertiesCount++;
                }
                
                // Apply all tags for this sample
                for (const auto& tag : tags)
                {
                    foundSample.addTag(tag);
                }
                importedCount++;
            }
            else
            {
                // Sample not found in library
                notFoundCount++;
                
                // Log first few not-found samples for debugging
                if (notFoundCount <= 5)
                {
                    DBG("WARNING: Sample not found in library: " + sampleFile.getFullPathName());
                }
            }
            
            processedCount++;
            
            // THROTTLE: Sleep every 100 samples to avoid UI freeze
            if (processedCount % 100 == 0)
            {
                Thread::sleep(5);
                DBG("Processed " + String(processedCount) + " of " + String(sampleToTags.size()) + 
                    " samples (found: " + String(importedCount) + ", not found: " + String(notFoundCount) + 
                    ", generated " + String(generatedThumbnailsCount) + " thumbnails)");
            }
        }
        
        DBG("Generation complete: " + String(generatedPropertiesCount) + " properties files, " + 
            String(generatedThumbnailsCount) + " thumbnails created");
        DBG("Import summary: Found " + String(importedCount) + " samples, " + 
            String(notFoundCount) + " samples not found in library");
        
        // Close the database
        mOrganizer.closeDatabase();
        
        LOG("\n=== IMPORT COMPLETE ===");
        LOG("Total samples in database: " + String(sampleToTags.size()));
        LOG("Samples found in library: " + String(importedCount));
        LOG("Samples NOT found in library: " + String(notFoundCount));
        LOG("Unique tags discovered: " + String(uniqueTags.size()));
        LOG("Properties files generated: " + String(generatedPropertiesCount));
        LOG("Thumbnails generated: " + String(generatedThumbnailsCount));
        LOG("\nLog saved to: " + logFile.getFullPathName());
        
        // Ensure log is flushed and written
        logStream.flush();
        
        // Verify log file exists
        bool logExists = logFile.exists();
        int64 logSize = logFile.getSize();
        DBG("LOG FILE STATUS: Exists=" + String(logExists ? "YES" : "NO") + ", Size=" + String(logSize) + " bytes");
        DBG("LOG FILE PATH: " + logFile.getFullPathName());
        
        String completionMessage = 
            "Imported and tagged " + String(importedCount) + " Splice samples.\n" +
            "Discovered " + String(uniqueTags.size()) + " unique tags.\n" +
            "Generated " + String(generatedPropertiesCount) + " properties files and " + 
            String(generatedThumbnailsCount) + " thumbnails.\n\n" +
            "Detailed log saved to:\n" + logFile.getFullPathName() +
            "\n(" + String(logSize) + " bytes)";
        
        if (notFoundCount > 0)
        {
            completionMessage += "\n\nWARNING: " + String(notFoundCount) + 
                " samples were not found in library.\nCheck log file for details.";
        }
        
        AlertWindow::showMessageBoxAsync(
            AlertWindow::InfoIcon,
            "Import Complete",
            completionMessage,
            "OK"
        );
        
        return importedCount;
    }
}
