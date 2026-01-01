/*
  ==============================================================================

    SpliceImportTask.h
    Created: 31 Dec 2025
    Author:  OpenCode
    
    Summary: Background task for Splice import with progress reporting

  ==============================================================================
*/

#ifndef SPLICEIMPORTTASK_H
#define SPLICEIMPORTTASK_H

#include "JuceHeader.h"
#include "SpliceOrganizer.h"
#include "SpliceImportConfig.h"
#include "SampleLibrary.h"

namespace samplore
{
    /// Background task for importing Splice samples
    class SpliceImportTask : public Thread,
                             public OrganizeProgressCallback
    {
    public:
        SpliceImportTask(const SpliceImportConfig& config, SampleLibrary& library);
        ~SpliceImportTask() override;
        
        void run() override;
        
        // OrganizeProgressCallback
        void onProgress(int current, int total, const String& status) override;
        bool shouldCancel() override;
        
        // Status access (const - reads cached values)
        bool isComplete() const { return mComplete; }
        bool wasSuccessful() const { return mSuccess; }
        String getErrorMessage() const { return mErrorMessage; }
        int getSamplesImported() const { return mSamplesImported; }
        
        // Progress access (const - reads cached values)
        float getProgress() const { return mCachedProgress.load(); }
        String getCurrentStatus() const { return mCachedStatus; }
        int getCurrentCount() const { return mCachedCurrent; }
        int getTotalCount() const { return mCachedTotal; }
        
        /// Request cancellation of the import
        void cancel();
        
        /// Copies live values to cached values (called by UI timer)
        void cacheProgress();
        
    private:
        SpliceImportConfig mConfig;
        SampleLibrary& mLibrary;
        
        // Live values (updated by onProgress)
        std::atomic<float> mLiveProgress{0.0f};
        std::atomic<int> mCurrent{0};
        std::atomic<int> mTotal{1};
        std::atomic<bool> mCancelled{false};
        String mLiveStatus;
        std::mutex mLiveStatusMutex;
        
        // Cached values (updated by cacheProgress(), read by const getters)
        std::atomic<float> mCachedProgress{0.0f};
        std::atomic<int> mCachedCurrent{0};
        std::atomic<int> mCachedTotal{1};
        String mCachedStatus;
        
        // Results
        bool mComplete = false;
        bool mSuccess = false;
        String mErrorMessage;
        int mSamplesImported = 0;
        
        // For cancel button
        std::atomic<bool> mUserCancelled{false};
        
        // For rollback
        struct SampleModification
        {
            Sample::Reference sample;
            StringArray addedTags;
        };
        std::vector<SampleModification> mModifiedSamples;
        std::vector<File> mAddedDirectories;
        
        /// Roll back all changes made during import
        void rollbackChanges();
        
        /// Clean up temporary directory
        void cleanupTempDirectory(const File& tempDir);
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpliceImportTask)
    };
}

#endif
