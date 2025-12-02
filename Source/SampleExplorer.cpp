#include "SampleExplorer.h"
#include "SampleLibrary.h"
#include "SamplifyProperties.h"
#include "SamplifyMainComponent.h"
#include "SamplifyLookAndFeel.h"
#include "ThemeManager.h"

using namespace samplore;

SampleExplorer::SampleExplorer() : mViewport(&mSampleContainer)
{
    addAndMakeVisible(mViewport);
	addAndMakeVisible(mFilter);
	addAndMakeVisible(mSearchBar);
	for (int i = 1; i < sortingNames.size(); i++)
	{
		mFilter.addItem(sortingNames[i], i);
	}
	mFilter.setSelectedId(0);
	mFilter.setLookAndFeel(&getLookAndFeel());
	mViewport.addAndMakeVisible(mSampleContainer);
	mViewport.setViewedComponent(&mSampleContainer);
	mViewport.setScrollBarsShown(true, false, true, false);
	mSearchBar.addListener(this);
	mFilter.addListener(this);
	
	// Register with ThemeManager
	ThemeManager::getInstance().addListener(this);
}

SampleExplorer::~SampleExplorer()
{
	// Remove from ThemeManager
	ThemeManager::getInstance().removeListener(this);
	
	// Remove from SampleLibrary to prevent dangling pointer
	if (auto lib = SamplifyProperties::getInstance()->getSampleLibrary())
		lib->removeChangeListener(this);
	
	mFilter.setLookAndFeel(nullptr);
}

void SampleExplorer::paint (Graphics& g)
{
	if (mIsUpdating)
	{
		float size = getWidth() / 5;
		getLookAndFeel().drawSpinningWaitAnimation(g, getLookAndFeel().findColour(loadingWheelColorId), (getWidth() / 2) - (size / 2), size, size, size);
		repaint();
	}
	else
	{
		// Check if there are no directories
		auto sampleLib = SamplifyProperties::getInstance()->getSampleLibrary();
		if (sampleLib->getDirectories().empty())
		{
			auto& theme = ThemeManager::getInstance();
			g.fillAll(theme.getColorForRole(ThemeManager::ColorRole::Background));
			
			g.setColour(theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
			g.setFont(16.0f);
			
			String message = "No directories added.\n\n";
			message += "Go to File -> Preferences -> Add Directory\n";
			message += "to add sample directories.";
			
			g.drawFittedText(message, getLocalBounds().reduced(40), Justification::centred, 3);
		}
	}
}

void SampleExplorer::resized()
{
    mSearchBar.setBounds(0, 0, getWidth() - 120, 30);
	mFilter.setBounds(getWidth() - 120, 0, 120, 30);
	mViewport.setBounds(0,30,getWidth(), getHeight()-30);
	mSampleContainer.setBounds(mViewport.getBounds().withRight(mViewport.getWidth() - mViewport.getScrollBarThickness()));
}

void SampleExplorer::textEditorTextChanged(TextEditor& e)
{
	SamplifyProperties::getInstance()->getSampleLibrary()->updateCurrentSamples(e.getText());
}

void SampleExplorer::changeListenerCallback(ChangeBroadcaster* source)
{
	if (SampleLibrary* sl = dynamic_cast<SampleLibrary*>(source))
	{
		if (sl->isAsyncValid())
		{
			mSampleContainer.setSampleItems(Sample::List()); //set to empty
			mIsUpdating = true;
			repaint();
		}
		else
		{
			mIsUpdating = false;
			mSampleContainer.setSampleItems(sl->getCurrentSamples());
			
			// Trigger repaint to update viewport scrollbars
			repaint();
		}
	}
}

void SampleExplorer::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
	if (comboBoxThatHasChanged->getSelectedId() == (int)SortingMethod::Newest) //Newest
	{
		SamplifyProperties::getInstance()->getSampleLibrary()->sortSamples(SortingMethod::Newest);
	}
	else if (comboBoxThatHasChanged->getSelectedId() == (int)SortingMethod::Oldest) //Oldest
	{
		SamplifyProperties::getInstance()->getSampleLibrary()->sortSamples(SortingMethod::Newest);
	}
	else if (comboBoxThatHasChanged->getSelectedId() == (int)SortingMethod::Random)
	{
		SamplifyProperties::getInstance()->getSampleLibrary()->sortSamples(SortingMethod::Random);
	}
}

SampleExplorer::SampleViewport::SampleViewport(SampleContainer* container)
{
	mSampleContainer = container;
}

void SampleExplorer::SampleViewport::visibleAreaChanged(const Rectangle<int>& newVisibleArea)
{
	// Update visible items based on current viewport position
	int viewportTop = newVisibleArea.getY();
	int viewportHeight = newVisibleArea.getHeight();
	
	mSampleContainer->updateVisibleItems(viewportTop, viewportHeight);
}

SampleExplorer::SampleSearchbar::SampleSearchbar()
{
	addAndMakeVisible(mEraseSearchButton);
	mEraseSearchButton.setButtonText("Clear");
	mEraseSearchButton.addListener(this);
}

void SampleExplorer::SampleSearchbar::resized()
{
	mEraseSearchButton.setBoundsRelative(0.8f, 0.2f, 0.1f, 0.6f);
	TextEditor::resized();
}

//==============================================================================
// ThemeManager::Listener implementation
void SampleExplorer::themeChanged(ThemeManager::Theme newTheme)
{
	repaint();
}

void SampleExplorer::colorChanged(ThemeManager::ColorRole role, Colour newColor)
{
	// Only repaint if the loading wheel color changed
	if (role == ThemeManager::ColorRole::AccentPrimary)
	{
		repaint();
	}
}
