/*
  ==============================================================================

    SpliceImportConfig.h
    Created: 31 Dec 2025
    Author:  OpenCode
    
    Summary: Shared configuration struct for Splice import

  ==============================================================================
*/

#ifndef SPLICEIMPORTCONFIG_H
#define SPLICEIMPORTCONFIG_H

#include "JuceHeader.h"

namespace samplore
{
    /// Configuration for Splice import
    struct SpliceImportConfig
    {
        File spliceDatabasePath;
        File spliceInstallDirectory;
        bool addToDirectoryList = false;
    };
}

#endif
