/*
  ==============================================================================

    SpliceOrganizer.h
    Created: 31 Dec 2025
    Author:  OpenCode
    
    Summary: C++ implementation of splorganizer functionality
             Queries Splice SQLite database and creates tag-based directory structure

  ==============================================================================
*/

#ifndef SPLICEORGANIZER_H
#define SPLICEORGANIZER_H

#include "JuceHeader.h"
#include "ThirdParty/sqlite3.h"

namespace samplore
{
    /// Result of organizing Splice samples
    struct OrganizeResult
    {
        int tagsProcessed = 0;
        int tagsTotal = 0;
        int shortcutsCreated = 0;
        int shortcutsTotal = 0;
        bool success = false;
        String errorMessage;
        bool cancelled = false;
    };
    
    /// Progress callback for long-running operations
    class OrganizeProgressCallback
    {
    public:
        virtual ~OrganizeProgressCallback() = default;
        virtual void onProgress(int current, int total, const String& status) = 0;
        virtual bool shouldCancel() = 0;
    };
    
    /// Manages organization of Splice samples into tag-based directory structure
    class SpliceOrganizer
    {
    public:
        SpliceOrganizer();
        ~SpliceOrganizer();
        
        void setProgressCallback(OrganizeProgressCallback* callback) { mProgressCallback = callback; }
        
        /// Opens the Splice database file
        bool openDatabase(const File& dbPath);
        
        /// Closes the database connection
        void closeDatabase();
        
        /// Gets all unique tags from the database
        StringArray getAllTags();
        
        /// Gets all samples that have a specific tag
        /// Returns array of File objects for sample paths
        Array<File> getSamplesForTag(const String& tag);
        
        /// Creates tag-based directory structure with shortcuts
        /// If appendMode is true, preserves existing shortcuts
        OrganizeResult organize(const File& outputDir, bool appendMode = false);
        
        /// Creates a shortcut/symlink to target file
        bool createShortcut(const File& target, const File& shortcutPath);
        
        /// Resolves a shortcut/symlink to get the target file
        File resolveShortcut(const File& shortcutFile);
        
        /// Gets tags for a specific sample file by scanning organized directory
        StringArray getTagsForSample(const File& organizerOutputDir, const File& sampleFile);
        
    private:
        sqlite3* mDatabase = nullptr;
        OrganizeProgressCallback* mProgressCallback = nullptr;
        
        /// Sanitizes a filename for safe use on filesystem
        String sanitizeFilename(const String& filename);
        
        /// Creates all tag directories
        void createTagDirectories(const File& outputDir, const StringArray& tags, bool clearExisting);
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpliceOrganizer)
    };
}

#endif
