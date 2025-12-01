/*
  ==============================================================================

    main_test.cpp
    Catch2 test runner with JUCE initialization for Samplore

  ==============================================================================
*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "JuceHeader.h"
#include "TestHelpers.h"

// JUCE initialization for tests
struct JuceTestInitializer
{
    JuceTestInitializer()
    {
        juce::initialiseJuce_GUI();
    }
    
    ~JuceTestInitializer()
    {
        juce::shutdownJuce_GUI();
    }
};

// Global JUCE initializer - runs before all tests
static JuceTestInitializer juceInit;

// Custom Catch2 matcher for JUCE Colours
struct ColourMatcher : Catch::Matchers::Impl::MatcherBase<juce::Colour>
{
    ColourMatcher(juce::Colour expected) : expected(expected) {}
    
    bool match(juce::Colour const& colour) const override
    {
        return colour == expected;
    }
    
    std::string describe() const override
    {
        return "equals colour " + expected.toString().toStdString();
    }
    
private:
    juce::Colour expected;
};

// Helper function to create Colour matcher
inline auto EqualsColour(juce::Colour colour) -> ColourMatcher
{
    return ColourMatcher(colour);
}

// Custom StringLiteral matcher for better error messages
inline auto EqualsStringLiteral(const juce::String& expected)
{
    return Catch::Matchers::Equals(expected.toStdString());
}

// Test utilities are now in TestHelpers.h