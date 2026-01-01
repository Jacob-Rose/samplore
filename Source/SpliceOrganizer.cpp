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
        
        // Use LIKE query to find samples with this tag
        String queryStr = "SELECT local_path FROM samples WHERE tags LIKE ? AND local_path IS NOT NULL";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(mDatabase, queryStr.toRawUTF8(), -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK)
        {
            DBG("Failed to prepare query: " + String(sqlite3_errmsg(mDatabase)));
            return samples;
        }
        
        // Bind the tag parameter with wildcards
        String searchPattern = "%" + tag + "%";
        sqlite3_bind_text(stmt, 1, searchPattern.toRawUTF8(), -1, SQLITE_TRANSIENT);
        
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char* path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            if (path)
            {
                File sampleFile(path);
                if (sampleFile.existsAsFile())
                {
                    samples.add(sampleFile);
                }
            }
        }
        
        sqlite3_finalize(stmt);
        
        return samples;
    }
    
    String SpliceOrganizer::sanitizeFilename(const String& filename)
    {
        String safe = filename;
        
        // Replace invalid characters with underscore
        safe = safe.replaceCharacters("<>:\"|?*", "________");
        
        // Remove leading/trailing dots and spaces
        safe = safe.trim();
        while (safe.startsWith("."))
            safe = safe.substring(1);
        
        return safe;
    }
    
    void SpliceOrganizer::createTagDirectories(const File& outputDir, const StringArray& tags, bool clearExisting)
    {
        if (clearExisting)
        {
            // Remove all existing content
            if (outputDir.exists())
            {
                DirectoryIterator iter(outputDir, false, "*", File::findFilesAndDirectories);
                while (iter.next())
                {
                    File item = iter.getFile();
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
        
        // LIMIT: Only process first 50 tags to avoid overwhelming the system
        const int MAX_TAGS_TO_PROCESS = 50;
        int tagsToProcess = jmin(tags.size(), MAX_TAGS_TO_PROCESS);
        
        if (tags.size() > MAX_TAGS_TO_PROCESS)
        {
            DBG("WARNING: Limiting to first " + String(MAX_TAGS_TO_PROCESS) + " tags to avoid system overload");
        }
        
        // Process each tag
        for (int tagIdx = 0; tagIdx < tagsToProcess; ++tagIdx)
        {
            const auto& tag = tags[tagIdx];
            File tagDir = outputDir.getChildFile(tag);
            Array<File> samples = getSamplesForTag(tag);
            
            // LIMIT: Only process first 100 samples per tag
            const int MAX_SAMPLES_PER_TAG = 100;
            int samplesToProcess = jmin(samples.size(), MAX_SAMPLES_PER_TAG);
            
            DBG("Processing tag '" + tag + "' - " + String(samplesToProcess) + " of " + 
                String(samples.size()) + " samples");
            
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
        
        result.success = true;
        DBG("Organization complete. Processed " + String(result.tagsProcessed) + 
            " tags, created " + String(result.shortcutsCreated) + " shortcuts");
        
        return result;
    }
    
    StringArray SpliceOrganizer::getTagsForSample(const File& organizerOutputDir, const File& sampleFile)
    {
        StringArray tags;
        
        if (!organizerOutputDir.exists())
            return tags;
        
        // Scan all tag directories
        DirectoryIterator dirIter(organizerOutputDir, false, "*", File::findDirectories);
        
        while (dirIter.next())
        {
            File tagDir = dirIter.getFile();
            String tagName = tagDir.getFileName();
            
            // Skip special directories
            if (tagName.startsWith("_"))
                continue;
            
            // Check if this tag directory contains a shortcut to our sample
            DirectoryIterator fileIter(tagDir, false, "*", File::findFiles);
            
            while (fileIter.next())
            {
                File shortcutFile = fileIter.getFile();
                
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
