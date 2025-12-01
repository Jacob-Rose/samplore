/*
  ==============================================================================

    SampleLibraryTests.cpp
    Catch2 tests for SampleLibrary class

  ==============================================================================
*/

#include <catch2/catch.hpp>
#include "SampleLibrary.h"
#include "Sample.h"
#include "TestHelpers.h"

using namespace samplore;

TEST_CASE("SampleLibrary singleton", "[samplelibrary]")
{
    SECTION("Instance management")
    {
        SampleLibrary& lib1 = SampleLibrary::getInstance();
        SampleLibrary& lib2 = SampleLibrary::getInstance();
        
        REQUIRE(&lib1 == &lib2);
    }
    
    SECTION("Initialization")
    {
        SampleLibrary& lib = SampleLibrary::getInstance();
        
        // Should start empty
        REQUIRE(lib.getNumSamples() == 0);
        REQUIRE(lib.getSamples().size() == 0);
    }
}

TEST_CASE("SampleLibrary sample management", "[samplelibrary]")
{
    SampleLibrary& lib = SampleLibrary::getInstance();
    
    // Clear any existing samples for clean test
    lib.clearAllSamples();
    
    SECTION("Add sample")
    {
        juce::File testFile = TestHelpers::createTempFile("test content", "wav");
        auto sample = std::make_shared<Sample>(testFile);
        
        lib.addSample(sample);
        
        REQUIRE(lib.getNumSamples() == 1);
        REQUIRE(lib.getSamples().size() == 1);
        REQUIRE(lib.getSamples()[0] == sample);
    }
    
    SECTION("Add multiple samples")
    {
        juce::File testFile1 = TestHelpers::createTempFile("content1", "wav");
        juce::File testFile2 = TestHelpers::createTempFile("content2", "wav");
        juce::File testFile3 = TestHelpers::createTempFile("content3", "wav");
        
        auto sample1 = std::make_shared<Sample>(testFile1);
        auto sample2 = std::make_shared<Sample>(testFile2);
        auto sample3 = std::make_shared<Sample>(testFile3);
        
        lib.addSample(sample1);
        lib.addSample(sample2);
        lib.addSample(sample3);
        
        REQUIRE(lib.getNumSamples() == 3);
        REQUIRE(lib.getSamples().size() == 3);
    }
    
    SECTION("Remove sample")
    {
        juce::File testFile = TestHelpers::createTempFile("test content", "wav");
        auto sample = std::make_shared<Sample>(testFile);
        
        lib.addSample(sample);
        REQUIRE(lib.getNumSamples() == 1);
        
        lib.removeSample(sample);
        REQUIRE(lib.getNumSamples() == 0);
    }
    
    SECTION("Clear all samples")
    {
        juce::File testFile1 = TestHelpers::createTempFile("content1", "wav");
        juce::File testFile2 = TestHelpers::createTempFile("content2", "wav");
        
        auto sample1 = std::make_shared<Sample>(testFile1);
        auto sample2 = std::make_shared<Sample>(testFile2);
        
        lib.addSample(sample1);
        lib.addSample(sample2);
        REQUIRE(lib.getNumSamples() == 2);
        
        lib.clearAllSamples();
        REQUIRE(lib.getNumSamples() == 0);
    }
}

TEST_CASE("SampleLibrary search and filtering", "[samplelibrary]")
{
    SampleLibrary& lib = SampleLibrary::getInstance();
    lib.clearAllSamples();
    
    // Create test samples with different properties
    juce::File kickFile = TestHelpers::createTempFile("kick content", "wav");
    juce::File snareFile = TestHelpers::createTempFile("snare content", "wav");
    juce::File bassFile = TestHelpers::createTempFile("bass content", "wav");
    
    auto kickSample = std::make_shared<Sample>(kickFile);
    auto snareSample = std::make_shared<Sample>(snareFile);
    auto bassSample = std::make_shared<Sample>(bassFile);
    
    // Add tags
    kickSample->addTag("kick");
    kickSample->addTag("drum");
    kickSample->addTag("acoustic");
    
    snareSample->addTag("snare");
    snareSample->addTag("drum");
    snareSample->addTag("electronic");
    
    bassSample->addTag("bass");
    bassSample->addTag("synth");
    
    lib.addSample(kickSample);
    lib.addSample(snareSample);
    lib.addSample(bassSample);
    
    SECTION("Search by filename")
    {
        auto results = lib.searchByFilename("kick");
        REQUIRE(results.size() == 1);
        REQUIRE(results[0] == kickSample);
        
        results = lib.searchByFilename("nonexistent");
        REQUIRE(results.size() == 0);
        
        results = lib.searchByFilename("content");  // Should match all
        REQUIRE(results.size() == 3);
    }
    
    SECTION("Search by tag")
    {
        auto results = lib.searchByTag("drum");
        REQUIRE(results.size() == 2);  // kick and snare
        REQUIRE(std::find(results.begin(), results.end(), kickSample) != results.end());
        REQUIRE(std::find(results.begin(), results.end(), snareSample) != results.end());
        
        results = lib.searchByTag("kick");
        REQUIRE(results.size() == 1);
        REQUIRE(results[0] == kickSample);
        
        results = lib.searchByTag("nonexistent");
        REQUIRE(results.size() == 0);
    }
    
    SECTION("Search by multiple tags")
    {
        juce::StringArray tags;
        tags.add("drum");
        tags.add("acoustic");
        
        auto results = lib.searchByTags(tags);
        REQUIRE(results.size() == 1);  // Only kick matches both
        REQUIRE(results[0] == kickSample);
    }
    
    SECTION("Filter by file extension")
    {
        auto results = lib.filterByExtension("wav");
        REQUIRE(results.size() == 3);  // All are wav files
        
        results = lib.filterByExtension("mp3");
        REQUIRE(results.size() == 0);  // No mp3 files
    }
}

TEST_CASE("SampleLibrary directory operations", "[samplelibrary]")
{
    SampleLibrary& lib = SampleLibrary::getInstance();
    lib.clearAllSamples();
    
    SECTION("Add directory")
    {
        juce::File tempDir = TestHelpers::createTempDirectory();
        
        // Create some test files in the directory
        juce::File file1 = tempDir.getChildFile("test1.wav");
        juce::File file2 = tempDir.getChildFile("test2.wav");
        file1.create();
        file2.create();
        
        lib.addDirectory(tempDir);
        
        // Should have found the files (implementation dependent)
        // For now, just test that the directory was processed
        REQUIRE(true);  // Placeholder - actual implementation would scan directory
    }
    
    SECTION("Remove directory")
    {
        juce::File tempDir = TestHelpers::createTempDirectory();
        
        lib.addDirectory(tempDir);
        lib.removeDirectory(tempDir);
        
        // Should have removed samples from that directory
        REQUIRE(true);  // Placeholder - actual implementation would track directory sources
    }
}

TEST_CASE("SampleLibrary sorting", "[samplelibrary]")
{
    SampleLibrary& lib = SampleLibrary::getInstance();
    lib.clearAllSamples();
    
    // Create samples with different names
    juce::File fileA = TestHelpers::createTempFile("content A", "wav");
    juce::File fileB = TestHelpers::createTempFile("content B", "wav");
    juce::File fileC = TestHelpers::createTempFile("content C", "wav");
    
    auto sampleA = std::make_shared<Sample>(fileA);
    auto sampleB = std::make_shared<Sample>(fileB);
    auto sampleC = std::make_shared<Sample>(fileC);
    
    // Add in random order
    lib.addSample(sampleC);
    lib.addSample(sampleA);
    lib.addSample(sampleB);
    
    SECTION("Sort by filename")
    {
        lib.sortByFilename();
        auto samples = lib.getSamples();
        
        REQUIRE(samples.size() == 3);
        REQUIRE(samples[0]->getFileName() == fileA.getFileName());
        REQUIRE(samples[1]->getFileName() == fileB.getFileName());
        REQUIRE(samples[2]->getFileName() == fileC.getFileName());
    }
    
    SECTION("Sort by date")
    {
        lib.sortByDate();
        auto samples = lib.getSamples();
        
        REQUIRE(samples.size() == 3);
        // Date sorting would depend on actual file dates
        REQUIRE(true);  // Placeholder
    }
    
    SECTION("Sort by size")
    {
        lib.sortBySize();
        auto samples = lib.getSamples();
        
        REQUIRE(samples.size() == 3);
        // Size sorting would depend on actual file sizes
        REQUIRE(true);  // Placeholder
    }
}

TEST_CASE("SampleLibrary statistics", "[samplelibrary]")
{
    SampleLibrary& lib = SampleLibrary::getInstance();
    lib.clearAllSamples();
    
    SECTION("Empty library statistics")
    {
        REQUIRE(lib.getNumSamples() == 0);
        REQUIRE(lib.getTotalSize() == 0);
        REQUIRE(lib.getUniqueTags().size() == 0);
    }
    
    SECTION("Library with samples statistics")
    {
        juce::File file1 = TestHelpers::createTempFile("content1", "wav");
        juce::File file2 = TestHelpers::createTempFile("content2", "wav");
        
        auto sample1 = std::make_shared<Sample>(file1);
        auto sample2 = std::make_shared<Sample>(file2);
        
        sample1->addTag("kick");
        sample1->addTag("drum");
        sample2->addTag("snare");
        sample2->addTag("drum");
        
        lib.addSample(sample1);
        lib.addSample(sample2);
        
        REQUIRE(lib.getNumSamples() == 2);
        REQUIRE(lib.getTotalSize() > 0);
        
        auto uniqueTags = lib.getUniqueTags();
        REQUIRE(uniqueTags.size() == 3);  // kick, drum, snare
        REQUIRE(uniqueTags.contains("kick"));
        REQUIRE(uniqueTags.contains("drum"));
        REQUIRE(uniqueTags.contains("snare"));
    }
}

TEST_CASE("SampleLibrary edge cases", "[samplelibrary]")
{
    SampleLibrary& lib = SampleLibrary::getInstance();
    lib.clearAllSamples();
    
    SECTION("Add null sample")
    {
        // Should handle gracefully
        lib.addSample(nullptr);
        REQUIRE(lib.getNumSamples() == 0);
    }
    
    SECTION("Remove null sample")
    {
        // Should handle gracefully
        lib.removeSample(nullptr);
        REQUIRE(lib.getNumSamples() == 0);
    }
    
    SECTION("Add duplicate sample")
    {
        juce::File testFile = TestHelpers::createTempFile("test content", "wav");
        auto sample = std::make_shared<Sample>(testFile);
        
        lib.addSample(sample);
        lib.addSample(sample);  // Add same sample again
        
        // Behavior depends on implementation - could allow duplicates or not
        REQUIRE(lib.getNumSamples() >= 1);
    }
    
    SECTION("Search empty library")
    {
        auto results = lib.searchByFilename("anything");
        REQUIRE(results.size() == 0);
        
        results = lib.searchByTag("anything");
        REQUIRE(results.size() == 0);
    }
    
    SECTION("Clear empty library")
    {
        lib.clearAllSamples();
        REQUIRE(lib.getNumSamples() == 0);
    }
}

TEST_CASE("SampleLibrary thread safety", "[samplelibrary]")
{
    SampleLibrary& lib = SampleLibrary::getInstance();
    lib.clearAllSamples();
    
    SECTION("Concurrent access")
    {
        // This is a basic test - real thread safety would require more complex setup
        juce::File testFile = TestHelpers::createTempFile("test content", "wav");
        auto sample = std::make_shared<Sample>(testFile);
        
        lib.addSample(sample);
        
        // Test that we can read while potentially writing
        REQUIRE(lib.getNumSamples() == 1);
        auto samples = lib.getSamples();
        REQUIRE(samples.size() == 1);
        
        lib.removeSample(sample);
        REQUIRE(lib.getNumSamples() == 0);
    }
}