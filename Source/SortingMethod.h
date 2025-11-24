/*
  ==============================================================================

    SortingMethod.h
    Created: 11 Jun 2020 2:08:20pm
    Author:  Jake Rose

  ==============================================================================
*/

#ifndef SORTINGMETHODS_H
#define SORTINGMETHODS_H

#include <JuceHeader.h>

enum class SortingMethod
{
	None,//just append all additions
	Newest,
	Oldest,
	Recent,
	Popular,
	Random
};

const std::vector<juce::String> sortingNames = {
	"None",
	"Newest",
	"Oldest",
	"Recent",
	"Popular",
	"Randomize" };
#endif