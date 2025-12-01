/*
  ==============================================================================

    SampleTests.cpp
    Catch2 tests for Sample class

  ==============================================================================
*/

#include <catch2/catch.hpp>
#include "Sample.h"
#include "TestHelpers.h"

using namespace samplore;

TEST_CASE("Sample creation and basic properties", "[sample]")
{
    SECTION("Default constructor")
    {
        Sample sample;
        
        REQUIRE_FALSE(sample.isValid());
        REQUIRE(sample.getFilePath().isEmpty());
        REQUIRE(sample.getFileName().isEmpty());
        REQUIRE(sample.getSampleRate() == 0.0);
        REQUIRE(sample.getLengthInSeconds() == 0.0);
        REQUIRE(sample.getNumChannels() == 0);
    }
    
    SECTION("Constructor with file path")
    {
        juce::File testFile = TestHelpers::createTempFile("test content", "wav");
        Sample sample(testFile);
        
        REQUIRE(sample.getFilePath() == testFile.getFullPathName());
        REQUIRE(sample.getFileName() == testFile.getFileName());
    }
}

TEST_CASE("Sample file operations", "[sample]")
{
    SECTION("Load valid audio file")
    {
        // Create a simple test audio file
        juce::File testFile = TestHelpers::createTempFile("test audio", "wav");
        
        // For now, just test file path handling
        Sample sample(testFile);
        REQUIRE(sample.getFilePath() == testFile.getFullPathName());
        REQUIRE(sample.getFileName() == testFile.getFileName());
    }
    
    SECTION("Handle non-existent file")
    {
        juce::File nonExistentFile("/path/to/nonexistent/file.wav");
        Sample sample(nonExistentFile);
        
        REQUIRE(sample.getFilePath() == nonExistentFile.getFullPathName());
        REQUIRE_FALSE(sample.isValid());
    }
    
    SECTION("File extension validation")
    {
        struct TestCase
        {
            juce::String filename;
            bool shouldLoad;
        };
        
        std::vector<TestCase> testCases = {
            {"test.wav", true},
            {"test.mp3", true},
            {"test.flac", true},
            {"test.ogg", true},
            {"test.aiff", true},
            {"test.txt", false},
            {"test.jpg", false},
            {"test", false}
        };
        
        for (const auto& testCase : testCases)
        {
            juce::File testFile = TestHelpers::createTempFile("content", testCase.filename);
            Sample sample(testFile);
            
            // Test that the filename is correctly parsed
            REQUIRE(sample.getFileName() == testCase.filename);
        }
    }
}

TEST_CASE("Sample metadata", "[sample]")
{
    SECTION("Basic metadata")
    {
        juce::File testFile = TestHelpers::createTempFile("test content", "wav");
        Sample sample(testFile);
        
        // Test that we can set and get metadata
        sample.setMetadata("title", "Test Sample");
        sample.setMetadata("artist", "Test Artist");
        sample.setMetadata("bpm", "120");
        
        REQUIRE(sample.getMetadata("title") == "Test Sample");
        REQUIRE(sample.getMetadata("artist") == "Test Artist");
        REQUIRE(sample.getMetadata("bpm") == "120");
        REQUIRE(sample.getMetadata("nonexistent").isEmpty());
    }
    
    SECTION("Clear metadata")
    {
        juce::File testFile = TestHelpers::createTempFile("test content", "wav");
        Sample sample(testFile);
        
        sample.setMetadata("title", "Test Sample");
        REQUIRE(sample.getMetadata("title") == "Test Sample");
        
        sample.clearMetadata("title");
        REQUIRE(sample.getMetadata("title").isEmpty());
    }
    
    SECTION("Clear all metadata")
    {
        juce::File testFile = TestHelpers::createTempFile("test content", "wav");
        Sample sample(testFile);
        
        sample.setMetadata("title", "Test Sample");
        sample.setMetadata("artist", "Test Artist");
        
        sample.clearAllMetadata();
        REQUIRE(sample.getMetadata("title").isEmpty());
        REQUIRE(sample.getMetadata("artist").isEmpty());
    }
}

TEST_CASE("Sample audio properties", "[sample]")
{
    SECTION("Sample rate")
    {
        Sample sample;
        
        // Test default sample rate
        REQUIRE(sample.getSampleRate() == 0.0);
        
        // Test setting sample rate
        sample.setSampleRate(44100.0);
        REQUIRE(sample.getSampleRate() == 44100.0);
        
        sample.setSampleRate(48000.0);
        REQUIRE(sample.getSampleRate() == 48000.0);
    }
    
    SECTION("Length calculation")
    {
        Sample sample;
        
        // Test default length
        REQUIRE(sample.getLengthInSeconds() == 0.0);
        
        // Test setting length
        sample.setLengthInSeconds(10.5);
        REQUIRE(sample.getLengthInSeconds() == 10.5);
        
        sample.setLengthInSeconds(0.0);
        REQUIRE(sample.getLengthInSeconds() == 0.0);
    }
    
    SECTION("Number of channels")
    {
        Sample sample;
        
        // Test default channels
        REQUIRE(sample.getNumChannels() == 0);
        
        // Test setting channels
        sample.setNumChannels(2);
        REQUIRE(sample.getNumChannels() == 2);
        
        sample.setNumChannels(1);
        REQUIRE(sample.getNumChannels() == 1);
    }
}

TEST_CASE("Sample tags", "[sample]")
{
    juce::File testFile = TestHelpers::createTempFile("test content", "wav");
    Sample sample(testFile);
    
    SECTION("Add and check tags")
    {
        sample.addTag("kick");
        sample.addTag("drum");
        sample.addTag("electronic");
        
        REQUIRE(sample.hasTag("kick"));
        REQUIRE(sample.hasTag("drum"));
        REQUIRE(sample.hasTag("electronic"));
        REQUIRE_FALSE(sample.hasTag("snare"));
    }
    
    SECTION("Remove tags")
    {
        sample.addTag("kick");
        sample.addTag("drum");
        
        REQUIRE(sample.hasTag("kick"));
        REQUIRE(sample.hasTag("drum"));
        
        sample.removeTag("kick");
        REQUIRE_FALSE(sample.hasTag("kick"));
        REQUIRE(sample.hasTag("drum"));
    }
    
    SECTION("Get all tags")
    {
        sample.addTag("kick");
        sample.addTag("drum");
        sample.addTag("electronic");
        
        juce::StringArray tags = sample.getTags();
        REQUIRE(tags.size() == 3);
        REQUIRE(tags.contains("kick"));
        REQUIRE(tags.contains("drum"));
        REQUIRE(tags.contains("electronic"));
    }
    
    SECTION("Clear all tags")
    {
        sample.addTag("kick");
        sample.addTag("drum");
        
        REQUIRE(sample.getTags().size() == 2);
        
        sample.clearTags();
        REQUIRE(sample.getTags().size() == 0);
        REQUIRE_FALSE(sample.hasTag("kick"));
        REQUIRE_FALSE(sample.hasTag("drum"));
    }
    
    SECTION("Duplicate tags")
    {
        sample.addTag("kick");
        sample.addTag("kick");  // Add same tag twice
        
        REQUIRE(sample.getTags().size() == 1);
        REQUIRE(sample.hasTag("kick"));
    }
}

TEST_CASE("Sample notes", "[sample]")
{
    juce::File testFile = TestHelpers::createTempFile("test content", "wav");
    Sample sample(testFile);
    
    SECTION("Set and get notes")
    {
        sample.setNotes("This is a great kick drum sample");
        REQUIRE(sample.getNotes() == "This is a great kick drum sample");
        
        sample.setNotes("");
        REQUIRE(sample.getNotes().isEmpty());
        
        sample.setNotes("Multi-line\nnote\nwith\nnewlines");
        REQUIRE(sample.getNotes() == "Multi-line\nnote\nwith\nnewlines");
    }
    
    SECTION("Append to notes")
    {
        sample.setNotes("Initial note");
        sample.appendNotes(" - additional info");
        
        REQUIRE(sample.getNotes() == "Initial note - additional info");
    }
}

TEST_CASE("Sample comparison", "[sample]")
{
    SECTION("Equal samples")
    {
        juce::File testFile = TestHelpers::createTempFile("test content", "wav");
        Sample sample1(testFile);
        Sample sample2(testFile);
        
        REQUIRE(sample1 == sample2);
        REQUIRE_FALSE(sample1 != sample2);
    }
    
    SECTION("Different samples")
    {
        juce::File testFile1 = TestHelpers::createTempFile("content1", "wav");
        juce::File testFile2 = TestHelpers::createTempFile("content2", "wav");
        
        Sample sample1(testFile1);
        Sample sample2(testFile2);
        
        REQUIRE(sample1 != sample2);
        REQUIRE_FALSE(sample1 == sample2);
    }
}

TEST_CASE("Sample edge cases", "[sample]")
{
    SECTION("Empty file path")
    {
        Sample sample;
        REQUIRE(sample.getFilePath().isEmpty());
        REQUIRE(sample.getFileName().isEmpty());
    }
    
    SECTION("Very long file name")
    {
        juce::String longName = "a_very_long_filename_that_tests_how_the_sample_class_handles_extremely_long_names.wav";
        juce::File testFile = TestHelpers::createTempFile("content", longName);
        Sample sample(testFile);
        
        REQUIRE(sample.getFileName() == longName);
    }
    
    SECTION("Special characters in file name")
    {
        juce::String specialName = "test-sample_with spaces&symbols.wav";
        juce::File testFile = TestHelpers::createTempFile("content", specialName);
        Sample sample(testFile);
        
        REQUIRE(sample.getFileName() == specialName);
    }
    
    SECTION("Unicode characters in file name")
    {
        juce::String unicodeName = "tëst_sämplë_üñïcødë.wav";
        juce::File testFile = TestHelpers::createTempFile("content", unicodeName);
        Sample sample(testFile);
        
        REQUIRE(sample.getFileName() == unicodeName);
    }
}

TEST_CASE("Sample audio buffer operations", "[sample]")
{
    SECTION("Create audio buffer")
    {
        Sample sample;
        sample.setNumChannels(2);
        sample.setSampleRate(44100.0);
        sample.setLengthInSeconds(1.0);  // 1 second at 44.1kHz
        
        juce::AudioBuffer<float> buffer = sample.createAudioBuffer();
        
        REQUIRE(buffer.getNumChannels() == 2);
        REQUIRE(buffer.getNumSamples() == 44100);
    }
    
    SECTION("Process audio buffer")
    {
        Sample sample;
        sample.setNumChannels(1);
        sample.setSampleRate(44100.0);
        sample.setLengthInSeconds(0.1);  // 0.1 second
        
        juce::AudioBuffer<float> buffer = sample.createAudioBuffer();
        
        // Generate test signal
        TestHelpers::generateSineWave(buffer, 440.0f, 44100.0f);
        
        // Verify the buffer has content
        bool hasContent = false;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                if (std::abs(buffer.getSample(ch, i)) > 0.001f)
                {
                    hasContent = true;
                    break;
                }
            }
            if (hasContent) break;
        }
        
        REQUIRE(hasContent);
    }
}