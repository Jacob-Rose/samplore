/*
  ==============================================================================

    SpliceImportTask.cpp
    Created: 31 Dec 2025
    Author:  OpenCode

  ==============================================================================
*/

#include "SpliceImportTask.h"

namespace samplore
{
    SpliceImportTask::SpliceImportTask(const SpliceImportConfig& config, SampleLibrary& library)
        : Thread("SpliceImport")
        , mConfig(config)
        , mLibrary(library)
    {
    }
    
    SpliceImportTask::~SpliceImportTask()
    {
        stopThread(5000);
    }
    
    void SpliceImportTask::cacheProgress()
    {
        // Copy live values to cached values (thread-safe copy)
        mCachedProgress = mLiveProgress.load();
        mCachedCurrent = mCurrent.load();
        mCachedTotal = mTotal.load();
        
        {
            std::lock_guard<std::mutex> lock(mLiveStatusMutex);
            mCachedStatus = mLiveStatus;
        }
    }
    
    void SpliceImportTask::run()
    {
        DBG("SpliceImportTask: Starting background import");
        
        // Step 1: Organize database
        SpliceOrganizer organizer;
        
        if (!organizer.openDatabase(mConfig.spliceDatabasePath))
        {
            mErrorMessage = "Failed to open Splice database";
            mComplete = true;
            mSuccess = false;
            return;
        }
        
        // Set progress callback
        organizer.setProgressCallback(this);
        
        // Create temp directory
        File tempDir = File::getSpecialLocation(File::tempDirectory)
            .getChildFile("samplore_splice_import");
        tempDir.createDirectory();
        
        // Run organization
        mLiveStatus = "Organizing Splice samples...";
        cacheProgress();
        
        OrganizeResult result = organizer.organize(tempDir, true);
        
        if (result.cancelled || mUserCancelled)
        {
            mLiveStatus = "Cancelled by user";
            cacheProgress();
            mComplete = true;
            mSuccess = false;
            organizer.closeDatabase();
            cleanupTempDirectory(tempDir);
            return;
        }
        
        if (!result.success)
        {
            mErrorMessage = result.errorMessage;
            mComplete = true;
            mSuccess = false;
            organizer.closeDatabase();
            cleanupTempDirectory(tempDir);
            return;
        }
        
        organizer.closeDatabase();
        
        // Step 2: Apply tags to samples
        mLiveStatus = "Preparing to apply tags";
        mLiveProgress = 0.0f;
        mCurrent = 0;
        mTotal = 1;
        cacheProgress();
        
        // Build set of unique directories
        std::map<String, StringArray> sampleToTags;
        
        auto dirIterator = RangedDirectoryIterator(tempDir, false, "*", File::findDirectories);
        for (auto dirEntry : dirIterator)
        {
            if (threadShouldExit() || mUserCancelled)
            {
                cleanupTempDirectory(tempDir);
                rollbackChanges();
                mLiveStatus = "Cancelled - changes rolled back";
                cacheProgress();
                mComplete = true;
                mSuccess = false;
                return;
            }
            
            File tagDir = dirEntry.getFile();
            String tagName = tagDir.getFileName();
            
            if (tagName.startsWith("_"))
                continue;
            
            auto fileIterator = RangedDirectoryIterator(tagDir, false, "*", File::findFiles);
            for (auto fileEntry : fileIterator)
            {
                if (threadShouldExit() || mUserCancelled)
                {
                    cleanupTempDirectory(tempDir);
                    rollbackChanges();
                    mLiveStatus = "Cancelled - changes rolled back";
                    cacheProgress();
                    mComplete = true;
                    mSuccess = false;
                    return;
                }
                
                File shortcutFile = fileEntry.getFile();
                File targetFile = organizer.resolveShortcut(shortcutFile);
                
                if (targetFile.existsAsFile())
                {
                    sampleToTags[targetFile.getFullPathName()].add(tagName);
                }
            }
        }
        
        // Add the Splice/Samples/packs directory to library (instead of individual subdirectories)
        if (mConfig.addToDirectoryList && mConfig.spliceInstallDirectory.isDirectory())
        {
            if (threadShouldExit() || mUserCancelled)
            {
                cleanupTempDirectory(tempDir);
                rollbackChanges();
                mLiveStatus = "Cancelled - changes rolled back";
                cacheProgress();
                mComplete = true;
                mSuccess = false;
                return;
            }
            
            // Look for Samples/packs subdirectory
            File packsDir = mConfig.spliceInstallDirectory.getChildFile("Samples").getChildFile("packs");
            
            if (!packsDir.isDirectory())
            {
                // Fallback: maybe the selected dir IS the packs dir already
                packsDir = mConfig.spliceInstallDirectory;
            }
            
            // Check if not already in library
            bool alreadyInLibrary = false;
            for (const auto& existingDir : mLibrary.getDirectories())
            {
                if (packsDir == existingDir->getFile() || packsDir.isAChildOf(existingDir->getFile()))
                {
                    alreadyInLibrary = true;
                    break;
                }
            }
            
            if (!alreadyInLibrary)
            {
                // Must lock message manager to safely add change listeners
                const MessageManagerLock mmLock;
                if (mmLock.lockWasGained())
                {
                    mLibrary.addDirectory(packsDir);
                    mAddedDirectories.push_back(packsDir);
                    DBG("Added Splice packs directory to library: " + packsDir.getFullPathName());
                }
            }
        }

        // Get all library samples for lookup
        auto allLibrarySamples = mLibrary.getAllSamplesInDirectories("", true);
        
        // Apply tags
        int processed = 0;
        int total = (int)sampleToTags.size();
        
        mLiveStatus = "Applying tags";
        mCurrent = 0;
        mTotal = total;
        cacheProgress();
        
        for (const auto& pair : sampleToTags)
        {
            if (threadShouldExit() || mUserCancelled)
            {
                cleanupTempDirectory(tempDir);
                rollbackChanges();
                mLiveStatus = "Cancelled - changes rolled back";
                cacheProgress();
                mComplete = true;
                mSuccess = false;
                return;
            }
            
            const String& samplePath = pair.first;
            const StringArray& tags = pair.second;
            
            // Find sample in library (linear search since we can't use map with weak_ptr)
            Sample::Reference foundSample(nullptr);
            for (int i = 0; i < allLibrarySamples.size(); ++i)
            {
                if (!allLibrarySamples[i].isNull() && allLibrarySamples[i].getFile().getFullPathName() == samplePath)
                {
                    foundSample = allLibrarySamples[i];
                    break;
                }
            }
            
            // Apply tags if found
            if (!foundSample.isNull())
            {
                // Track tags added for rollback
                mModifiedSamples.push_back({foundSample, tags});
                
                for (const auto& tag : tags)
                {
                    foundSample.addTag(tag);
                }
                mSamplesImported++;
            }
            
            processed++;
            
            // Update progress
            mLiveProgress = (float)processed / (float)total;
            mCurrent = processed;
            mTotal = total;
            
            {
                std::lock_guard<std::mutex> lock(mLiveStatusMutex);
                mLiveStatus = "Applying tags";
            }
            
            // Cache every 5 samples for UI updates
            if (processed % 5 == 0)
            {
                cacheProgress();
                Thread::sleep(1);
            }
        }
        
        // Cleanup temp directory on success
        cleanupTempDirectory(tempDir);
        
        // Final cache
        mLiveProgress = 1.0f;
        mLiveStatus = "Import complete!";
        cacheProgress();
        
        mComplete = true;
        mSuccess = !mUserCancelled && mSamplesImported > 0;
        
        DBG("SpliceImportTask: Complete. Imported " + String(mSamplesImported) + " samples");
    }
    
    void SpliceImportTask::onProgress(int current, int total, const String& status)
    {
        if (total > 0)
        {
            mLiveProgress = (float)current / (float)total;
        }
        else
        {
            mLiveProgress = 0.0f;
        }
        mCurrent = current;
        mTotal = total;
        
        {
            std::lock_guard<std::mutex> lock(mLiveStatusMutex);
            mLiveStatus = status;
        }
    }
    
    bool SpliceImportTask::shouldCancel()
    {
        return threadShouldExit() || mUserCancelled;
    }
    
    void SpliceImportTask::cancel()
    {
        mUserCancelled = true;
    }
    
    void SpliceImportTask::rollbackChanges()
    {
        DBG("SpliceImportTask: Rolling back changes");
        
        // Remove all tags we added
        for (auto& modified : mModifiedSamples)
        {
            if (!modified.sample.isNull())
            {
                for (const auto& tag : modified.addedTags)
                {
                    modified.sample.removeTag(tag);
                }
            }
        }
        mModifiedSamples.clear();
        
        // Remove all directories we added (requires message manager lock)
        if (!mAddedDirectories.empty())
        {
            const MessageManagerLock mmLock;
            if (mmLock.lockWasGained())
            {
                for (const auto& dir : mAddedDirectories)
                {
                    mLibrary.removeDirectory(dir);
                }
            }
        }
        mAddedDirectories.clear();
        
        mSamplesImported = 0;
        
        DBG("SpliceImportTask: Rollback complete");
    }
    
    void SpliceImportTask::cleanupTempDirectory(const File& tempDir)
    {
        if (tempDir.exists())
        {
            DBG("SpliceImportTask: Cleaning up temp directory: " + tempDir.getFullPathName());
            tempDir.deleteRecursively();
        }
    }
}
