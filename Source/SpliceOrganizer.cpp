/*
  ==============================================================================

    SpliceOrganizer.cpp
    Created: 31 Dec 2025
    Author:  OpenCode

  ==============================================================================
*/

#include "SpliceOrganizer.h"

namespace samplore
{
    SpliceOrganizer::SpliceOrganizer()
    {
    }
    
    SpliceOrganizer::~SpliceOrganizer()
    {
        closeDatabase();
    }
    
    bool SpliceOrganizer::openDatabase(const File& dbPath)
    {
        if (mDatabase)
        {
            closeDatabase();
        }
        
        int rc = sqlite3_open(dbPath.getFullPathName().toRawUTF8(), &mDatabase);
        
        if (rc != SQLITE_OK)
        {
            DBG("Failed to open database: " + String(sqlite3_errmsg(mDatabase)));
            sqlite3_close(mDatabase);
            mDatabase = nullptr;
            return false;
        }
        
        DBG("Opened Splice database: " + dbPath.getFullPathName());
        return true;
    }
    
    void SpliceOrganizer::closeDatabase()
    {
        if (mDatabase)
        {
            sqlite3_close(mDatabase);
            mDatabase = nullptr;
        }
    }
    
    StringArray SpliceOrganizer::getAllTags()
    {
        StringArray tags;
        
        if (!mDatabase)
        {
            DBG("Database not open");
            return tags;
        }
        
        const char* query = "SELECT tags FROM samples WHERE tags IS NOT NULL";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(mDatabase, query, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK)
        {
            DBG("Failed to prepare query: " + String(sqlite3_errmsg(mDatabase)));
            return tags;
        }
        
        std::set<String> uniqueTags;
        
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char* tagsText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            if (tagsText)
            {
                String tagsString(tagsText);
                StringArray tagArray;
                tagArray.addTokens(tagsString, ",", "\"");
                
                for (auto& tag : tagArray)
                {
                    tag = tag.trim();
                    if (tag.isNotEmpty())
                    {
                        uniqueTags.insert(tag);
                    }
                }
            }
        }
        
        sqlite3_finalize(stmt);
        
        // Convert set to StringArray
        for (const auto& tag : uniqueTags)
        {
            tags.add(tag);
        }
        
        tags.sort(true); // case-insensitive sort
        
        DBG("Found " + String(tags.size()) + " unique tags");
        return tags;
    }
    
    Array<File> SpliceOrganizer::getSamplesForTag(const String& tag)
    {
        Array<File> samples;
        
        if (!mDatabase)
        {
            DBG("Database not open");
            return samples;
        }
        
        // Sanitize tag - remove any null characters or invalid characters
        String cleanTag = tag.removeCharacters(String::fromUTF8("\0", 1));
        cleanTag = cleanTag.trim();
        
        if (cleanTag.isEmpty())
        {
            DBG("Empty tag after sanitization, skipping");
            return samples;
        }
        
        // Use LIKE query to find samples with this tag
        String queryStr = "SELECT local_path FROM samples WHERE tags LIKE ? AND local_path IS NOT NULL";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(mDatabase, queryStr.toRawUTF8(), -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK)
        {
            DBG("Failed to prepare query: " + String(sqlite3_errmsg(mDatabase)));
            return samples;
        }
        
        // Escape SQL LIKE special characters in the tag
        String escapedTag = cleanTag.replace("%", "\\%").replace("_", "\\_");
        
        // Bind the tag parameter with wildcards
        String searchPattern = "%" + escapedTag + "%";
        sqlite3_bind_text(stmt, 1, searchPattern.toRawUTF8(), -1, SQLITE_TRANSIENT);
        
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char* path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            if (path)
            {
                // Create string from path, handling potential encoding issues
                String pathStr = String::fromUTF8(path);
                
                // Skip if path is empty or contains null characters
                if (pathStr.isEmpty() || pathStr.containsChar('\0'))
                {
                    continue;
                }
                
                File sampleFile(pathStr);
                if (sampleFile.existsAsFile())
                {
                    samples.add(sampleFile);
                }
                else
                {
                    // Try to find the file with different encoding if original doesn't exist
                    // This handles cases where database has encoding issues
                    String fileName = sampleFile.getFileName();
                    File parentDir = sampleFile.getParentDirectory();
                    
                    if (parentDir.exists())
                    {
                        // Search for a file with similar name (case-insensitive)
                        auto iter = RangedDirectoryIterator(parentDir, false, "*", File::findFiles);
                        for (const auto& entry : iter)
                        {
                            if (entry.getFile().getFileName().equalsIgnoreCase(fileName))
                            {
                                samples.add(entry.getFile());
                                break;
                            }
                        }
                    }
                }
            }
        }
        
        sqlite3_finalize(stmt);
        
        return samples;
    }
    
    std::map<String, StringArray> SpliceOrganizer::getAllSamplesWithTags()
    {
        std::map<String, StringArray> sampleToTags;
        
        if (!mDatabase)
        {
            DBG("Database not open");
            return sampleToTags;
        }
        
        // Get all samples with their tags in one query
        const char* query = "SELECT local_path, tags FROM samples WHERE local_path IS NOT NULL AND tags IS NOT NULL";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(mDatabase, query, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK)
        {
            DBG("Failed to prepare query: " + String(sqlite3_errmsg(mDatabase)));
            return sampleToTags;
        }
        
        int rowCount = 0;
        int encodingErrorCount = 0;
        
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char* path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            const char* tagsText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            
            if (path && tagsText)
            {
                // Create string from path, handling potential encoding issues
                String pathStr = String::fromUTF8(path);
                
                // Skip if path is empty or contains null characters
                if (pathStr.isEmpty() || pathStr.containsChar('\0'))
                {
                    encodingErrorCount++;
                    continue;
                }
                
                File sampleFile(pathStr);
                
                // Only include if file exists on disk
                if (sampleFile.existsAsFile())
                {
                    String samplePath = sampleFile.getFullPathName();
                    String tagsString = String::fromUTF8(tagsText);
                    
                    StringArray tags;
                    tags.addTokens(tagsString, ",", "\"");
                    
                    // Trim whitespace and remove invalid tags
                    StringArray cleanTags;
                    for (int i = 0; i < tags.size(); ++i)
                    {
                        String cleanTag = tags[i].trim();
                        cleanTag = cleanTag.removeCharacters(String::fromUTF8("\0", 1));
                        
                        if (cleanTag.isNotEmpty())
                        {
                            cleanTags.add(cleanTag);
                        }
                    }
                    
                    if (cleanTags.size() > 0)
                    {
                        sampleToTags[samplePath] = cleanTags;
                        rowCount++;
                    }
                }
            }
        }
        
        if (encodingErrorCount > 0)
        {
            DBG("Skipped " + String(encodingErrorCount) + " samples with encoding errors");
        }
        
        sqlite3_finalize(stmt);
        
        DBG("getAllSamplesWithTags: Found " + String(rowCount) + " samples with tags");
        
        return sampleToTags;
    }
    
    String SpliceOrganizer::sanitizeFilename(const String& filename)
    {
        // Use JUCE's built-in legal filename creator
        // This removes invalid characters: " # @ , ; : < > * ^ | ? \ /
        // and handles long filenames (max 128 chars)
        String safe = File::createLegalFileName(filename);
        
        // Additional cleanup: remove leading/trailing dots and spaces
        safe = safe.trim();
        while (safe.startsWith("."))
            safe = safe.substring(1);
        
        // Ensure we have something if the filename became empty
        if (safe.isEmpty())
            safe = "unnamed";
        
        return safe;
    }
    
    void SpliceOrganizer::createTagDirectories(const File& outputDir, const StringArray& tags, bool clearExisting)
    {
        if (clearExisting)
        {
            // Remove all existing content
            if (outputDir.exists())
            {
                auto iter = RangedDirectoryIterator(outputDir, false, "*", File::findFilesAndDirectories);
                for (const auto& entry : iter)
                {
                    File item = entry.getFile();
                    if (item.isDirectory())
                        item.deleteRecursively();
                    else
                        item.deleteFile();
                }
            }
        }
        
        // Create output directory if it doesn't exist
        outputDir.createDirectory();
        
        // Create all tag directories
        for (const auto& tag : tags)
        {
            File tagDir = outputDir.getChildFile(tag);
            tagDir.createDirectory();
        }
    }
    
    bool SpliceOrganizer::createShortcut(const File& target, const File& shortcutPath)
    {
        #if JUCE_WINDOWS
            // On Windows, create a .lnk file using COM
            // For now, we'll use a simpler approach - just create a text file
            // A full .lnk implementation would require COM/WinAPI
            FileOutputStream stream(shortcutPath);
            if (stream.openedOk())
            {
                stream.writeText(target.getFullPathName(), false, false, nullptr);
                return true;
            }
            return false;
        #else
            // On Unix, create a symbolic link
            return target.createSymbolicLink(shortcutPath, true);
        #endif
    }
    
    File SpliceOrganizer::resolveShortcut(const File& shortcutFile)
    {
        #if JUCE_WINDOWS
            // On Windows, read our text file format
            if (shortcutFile.getFileExtension() == ".lnk" || shortcutFile.getFileExtension() == ".txt")
            {
                String content = shortcutFile.loadFileAsString();
                content = content.trim();
                if (content.isNotEmpty())
                {
                    return File(content);
                }
            }
            return File();
        #else
            // On Unix, follow symlink
            if (shortcutFile.isSymbolicLink())
            {
                return shortcutFile.getLinkedTarget();
            }
            return shortcutFile;
        #endif
    }
    
    OrganizeResult SpliceOrganizer::organize(const File& outputDir, bool appendMode)
    {
        OrganizeResult result;
        
        if (!mDatabase)
        {
            result.errorMessage = "Database not open";
            return result;
        }
        
        // Get all tags
        StringArray tags = getAllTags();
        
        if (tags.isEmpty())
        {
            result.errorMessage = "No tags found in database";
            return result;
        }
        
        DBG("Creating directory structure in: " + outputDir.getFullPathName());
        DBG("Append mode: " + String(appendMode ? "YES" : "NO"));
        DBG("Found " + String(tags.size()) + " tags");
        
        // Create tag directories
        createTagDirectories(outputDir, tags, !appendMode);
        
        // Process ALL tags (no limits)
        int tagsToProcess = tags.size();
        result.tagsTotal = tagsToProcess;
        
        // Process each tag
        for (int tagIdx = 0; tagIdx < tagsToProcess; ++tagIdx)
        {
            const auto& tag = tags[tagIdx];
            File tagDir = outputDir.getChildFile(tag);
            Array<File> samples = getSamplesForTag(tag);
            
            // Process ALL samples per tag (no limits)
            int samplesToProcess = samples.size();
            
            // Report progress
            String status = "Processing tag '" + tag + "' (" + String(tagIdx + 1) + "/" + String(tagsToProcess) + ")";
            if (mProgressCallback)
            {
                // Estimate total shortcuts (rough calculation)
                int totalShortcuts = tagsToProcess * 100; // Rough estimate
                int currentShortcut = tagIdx * 100;
                mProgressCallback->onProgress(currentShortcut, totalShortcuts, status);
                
                if (mProgressCallback->shouldCancel())
                {
                    result.cancelled = true;
                    result.success = true;
                    return result;
                }
            }
            
            DBG(status + " - " + String(samplesToProcess) + " samples");
            
            for (int sampleIdx = 0; sampleIdx < samplesToProcess; ++sampleIdx)
            {
                const auto& sampleFile = samples[sampleIdx];
                String filename = sampleFile.getFileName();
                String safeFilename = sanitizeFilename(filename);
                
                #if JUCE_WINDOWS
                    File shortcutFile = tagDir.getChildFile(safeFilename + ".lnk");
                #else
                    File shortcutFile = tagDir.getChildFile(safeFilename);
                #endif
                
                // Only create if doesn't exist (for append mode)
                if (!shortcutFile.exists())
                {
                    if (createShortcut(sampleFile, shortcutFile))
                    {
                        result.shortcutsCreated++;
                    }
                }
                
                // THROTTLE: Sleep every 100 shortcuts to avoid overwhelming filesystem
                if (result.shortcutsCreated % 100 == 0)
                {
                    Thread::sleep(10);
                }
            }
            
            result.tagsProcessed++;
        }
        
        // Final progress report
        if (mProgressCallback)
        {
            mProgressCallback->onProgress(result.shortcutsCreated, result.shortcutsCreated, "Complete!");
        }
        
        result.success = true;
        DBG("Organization complete. Processed " + String(result.tagsProcessed) + 
            " tags, created " + String(result.shortcutsCreated) + " shortcuts");
        
        return result;
    }
    
    StringArray SpliceOrganizer::getTagsForSampleFromDatabase(const File& sampleFile)
    {
        StringArray tags;
        
        if (!mDatabase)
        {
            DBG("Database not open");
            return tags;
        }
        
        // Query the database for this specific sample's tags
        String queryStr = "SELECT tags FROM samples WHERE local_path = ?";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(mDatabase, queryStr.toRawUTF8(), -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK)
        {
            DBG("Failed to prepare query: " + String(sqlite3_errmsg(mDatabase)));
            return tags;
        }
        
        // Bind the sample path
        String samplePath = sampleFile.getFullPathName();
        sqlite3_bind_text(stmt, 1, samplePath.toRawUTF8(), -1, SQLITE_TRANSIENT);
        
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char* tagsText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            if (tagsText)
            {
                String tagsString(tagsText);
                tags.addTokens(tagsString, ",", "\"");
                
                // Trim whitespace from each tag
                for (int i = 0; i < tags.size(); ++i)
                {
                    tags.set(i, tags[i].trim());
                }
            }
        }
        
        sqlite3_finalize(stmt);
        
        return tags;
    }
    
    StringArray SpliceOrganizer::getTagsForSample(const File& organizerOutputDir, const File& sampleFile)
    {
        StringArray tags;
        
        if (!organizerOutputDir.exists())
            return tags;
        
        // Scan all tag directories
        auto dirIter = RangedDirectoryIterator(organizerOutputDir, false, "*", File::findDirectories);
        
        for (const auto& dirEntry : dirIter)
        {
            File tagDir = dirEntry.getFile();
            String tagName = tagDir.getFileName();
            
            // Skip special directories
            if (tagName.startsWith("_"))
                continue;
            
            // Check if this tag directory contains a shortcut to our sample
            auto fileIter = RangedDirectoryIterator(tagDir, false, "*", File::findFiles);
            
            for (const auto& fileEntry : fileIter)
            {
                File shortcutFile = fileEntry.getFile();
                
                // Try to resolve the shortcut/symlink
                File targetFile = resolveShortcut(shortcutFile);
                
                // If we can't resolve, try matching by filename
                if (!targetFile.exists())
                {
                    String shortcutName = shortcutFile.getFileNameWithoutExtension();
                    String sampleName = sampleFile.getFileNameWithoutExtension();
                    
                    if (shortcutName == sampleName)
                    {
                        tags.add(tagName);
                        break;
                    }
                }
                else if (targetFile == sampleFile)
                {
                    tags.add(tagName);
                    break;
                }
            }
        }
        
        return tags;
    }
}
