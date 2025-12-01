/*
  ==============================================================================

    TestHelpers.h
    Test helper utilities for Samplore unit tests

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include <cmath>

namespace TestHelpers
{
    /// Create a temporary test file
    inline juce::File createTempFile(const juce::String& content, const juce::String& extension = "txt")
    {
        juce::File tempFile = juce::File::createTempFile(extension);
        tempFile.replaceWithText(content);
        return tempFile;
    }
    
    /// Create a temporary test directory
    inline juce::File createTempDirectory()
    {
        auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("samplore_test_" + juce::String(juce::Random::getSystemRandom().nextInt()));
        tempDir.createDirectory();
        return tempDir;
    }
    
    /// Check if two audio buffers are approximately equal (for floating point)
    inline bool buffersEqual(const juce::AudioBuffer<float>& a, 
                            const juce::AudioBuffer<float>& b, 
                            float tolerance = 0.0001f)
    {
        if (a.getNumChannels() != b.getNumChannels() ||
            a.getNumSamples() != b.getNumSamples())
            return false;
        
        for (int ch = 0; ch < a.getNumChannels(); ++ch)
        {
            for (int i = 0; i < a.getNumSamples(); ++i)
            {
                if (std::abs(a.getSample(ch, i) - b.getSample(ch, i)) > tolerance)
                    return false;
            }
        }
        return true;
    }
    
    /// Generate test audio data
    inline void generateSineWave(juce::AudioBuffer<float>& buffer, 
                                 float frequency = 440.0f, 
                                 float sampleRate = 44100.0f)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* channel = buffer.getWritePointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                channel[i] = std::sin(2.0f * juce::MathConstants<float>::pi * frequency * i / sampleRate);
            }
        }
    }
}
