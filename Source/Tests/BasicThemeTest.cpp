/*
  ==============================================================================

    BasicThemeTest.cpp
    Simple ThemeManager test without dependencies

  ==============================================================================
*/

#include <catch2/catch.hpp>
#include "JuceHeader.h"
#include "TestHelpers.h"

// Simple standalone test that doesn't require ThemeManager
TEST_CASE("JUCE initialization works", "[basic]")
{
    SECTION("Can create JUCE colours")
    {
        juce::Colour red = juce::Colours::red;
        REQUIRE(red.getRed() == 255);
        REQUIRE(red.getGreen() == 0);
        REQUIRE(red.getBlue() == 0);
    }
    
    SECTION("Can create JUCE strings")
    {
        juce::String test = "Hello Samplore!";
        REQUIRE(test.length() == 15);
        REQUIRE(test.startsWith("Hello"));
        REQUIRE(test.endsWith("Samplore!"));
    }
}

TEST_CASE("Catch2 framework works", "[basic]")
{
    SECTION("Basic assertions")
    {
        REQUIRE(1 + 1 == 2);
        REQUIRE(true == true);
        REQUIRE_FALSE(false);
    }
    
    SECTION("String comparisons")
    {
        std::string hello = "hello";
        REQUIRE(hello == "hello");
        REQUIRE(hello != "world");
    }
}
