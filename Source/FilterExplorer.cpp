#include "FilterExplorer.h"

using namespace samplore;
//==============================================================================
FilterExplorer::FilterExplorer()
{
	mSearchBar.addListener(this);

	addAndMakeVisible(mSearchBar);
	addAndMakeVisible(mTagExplorer);
}

FilterExplorer::~FilterExplorer()
{
}

void FilterExplorer::paint (Graphics& g)
{

}

void FilterExplorer::resized()
{
	mSearchBar.setBounds(0, 0, getWidth(), 30);
	mTagExplorer.setBounds(0,30,getWidth(), getHeight()-30);
}

void FilterExplorer::textEditorTextChanged(TextEditor& te)
{
	mTagExplorer.updateTags(te.getText());
}