/*
  ==============================================================================

    SpliceImporter.h
    Created: 31 Dec 2025
    Author:  OpenCode
    
    Summary: Handles importing Splice samples using the SpliceOrganizer
             Organizes samples into tag-based directory structure,
             then reads the shortcuts to apply tags to samples in Samplore

  ==============================================================================
*/

#ifndef SPLICEIMPORTER_H
#define SPLICEIMPORTER_H

#include "JuceHeader.h"
#include "SampleLibrary.h"
#include "SpliceOrganizer.h"

namespace samplore
{
    /// Manages importing Splice samples and applying tags from Splice DB
    class SpliceImporter
    {
    public:
        SpliceImporter();
        ~SpliceImporter();
        
        /// Finds the Splice sounds.db file (searches common locations)
        /// Returns empty File if not found
        File findSpliceDatabaseFile();
        
        /// Sets the path to the Splice sounds.db file
        void setSpliceDatabasePath(const File& dbPath);
        
        /// Gets the current Splice database path
        File getSpliceDatabasePath() const { return mSpliceDatabasePath; }
        
        /// Imports all Splice samples into the sample library with tags
        /// Uses SpliceOrganizer to create tag-based directory structure,
        /// then reads the generated directory to apply tags
        /// Shows progress dialog during import
        /// Returns number of samples imported
        int importSpliceSamples(SampleLibrary& library);
        
    private:
        File mSpliceDatabasePath;
        SpliceOrganizer mOrganizer;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpliceImporter)
    };
}

#endif
