#include "SampleExplorer.h"
#include "SampleLibrary.h"
#include "SamplifyProperties.h"
#include "SamplifyMainComponent.h"
#include "SamplifyLookAndFeel.h"
#include "ThemeManager.h"
#include "PerformanceProfiler.h"

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
	PROFILE_PAINT("SampleExplorer::paint");
	
	auto& theme = ThemeManager::getInstance();
	auto sampleLib = SamplifyProperties::getInstance()->getSampleLibrary();
	
	if (mIsUpdating)
	{
		float size = getWidth() / 5;
		getLookAndFeel().drawSpinningWaitAnimation(g, getLookAndFeel().findColour(loadingWheelColorId), (getWidth() / 2) - (size / 2), size, size, size);
		repaint();
	}
	else if (sampleLib->getDirectories().empty())
	{
		// No directories added - show helpful message
		g.fillAll(theme.getColorForRole(ThemeManager::ColorRole::Background));
		
		// Draw jumbotron-style container
		Rectangle<int> messageBox = getLocalBounds().reduced(60).withSizeKeepingCentre(
			jmin(500, getWidth() - 120),
			jmin(300, getHeight() - 120)
		);
		
		// Draw rounded background
		g.setColour(theme.getColorForRole(ThemeManager::ColorRole::Surface).withAlpha(0.5f));
		g.fillRoundedRectangle(messageBox.toFloat(), 12.0f);
		
		// Draw border
		g.setColour(theme.getColorForRole(ThemeManager::ColorRole::Border));
		g.drawRoundedRectangle(messageBox.toFloat().reduced(1), 12.0f, 2.0f);
		
		// Draw icon/emoji at top
		g.setColour(theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
		g.setFont(48.0f);
		g.drawText("📁", messageBox.removeFromTop(80), Justification::centred);
		
		// Draw title
		g.setColour(theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
		g.setFont(FontOptions(24.0f, Font::bold));
		g.drawText("No Directories Added", messageBox.removeFromTop(40), Justification::centred);
		
		messageBox.removeFromTop(20); // Spacing
		
		// Draw message
		g.setColour(theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
		g.setFont(16.0f);
		String message = "Add sample directories to get started.\n\nGo to File -> Preferences to add directories.";
		g.drawFittedText(message, messageBox, Justification::centred, 4);
	}
	else if (sampleLib->getCurrentSamples().size() == 0)
	{
		// Directories exist but no samples found
		g.fillAll(theme.getColorForRole(ThemeManager::ColorRole::Background));
		
		// Draw jumbotron-style container
		Rectangle<int> messageBox = getLocalBounds().reduced(60).withSizeKeepingCentre(
			jmin(500, getWidth() - 120),
			jmin(300, getHeight() - 120)
		);
		
		// Draw rounded background
		g.setColour(theme.getColorForRole(ThemeManager::ColorRole::Surface).withAlpha(0.5f));
		g.fillRoundedRectangle(messageBox.toFloat(), 12.0f);
		
		// Draw border
		g.setColour(theme.getColorForRole(ThemeManager::ColorRole::Border));
		g.drawRoundedRectangle(messageBox.toFloat().reduced(1), 12.0f, 2.0f);
		
		// Draw icon/emoji at top
		g.setColour(theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
		g.setFont(48.0f);
		g.drawText(",0", messageBox.removeFromTop(80), Justification::centred);
		
		// Draw title
		g.setColour(theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
		g.setFont(FontOptions(24.0f, Font::bold));
		
		String title = mSearchBar.getText().isEmpty() ? "No Samples Found" : "No Matching Samples";
		g.drawText(title, messageBox.removeFromTop(40), Justification::centred);
		
		messageBox.removeFromTop(20); // Spacing
		
		// Draw message
		g.setColour(theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
		g.setFont(16.0f);
		
		String message;
		if (!mSearchBar.getText().isEmpty())
		{
			message = "No samples match your search.\n\nTry a different search term or clear the search.";
		}
		else
		{
			message = "No audio files found in your directories.\n\nMake sure your directories contain audio files,\nor add more directories in File -> Preferences.";
		}
		
		g.drawFittedText(message, messageBox, Justification::centred, 4);
	}
}

void SampleExplorer::resized()
{
	auto sampleLib = SamplifyProperties::getInstance()->getSampleLibrary();
	bool hasDirectories = !sampleLib->getDirectories().empty();
	bool hasSamples = sampleLib->getCurrentSamples().size() > 0;
	
	// Hide search/filter UI when showing empty state
	bool showUI = hasDirectories && (hasSamples || !mSearchBar.getText().isEmpty());
	mSearchBar.setVisible(showUI);
	mFilter.setVisible(showUI);
	mViewport.setVisible(showUI);
	
	mSearchBar.setBounds(0, 0, getWidth() - 120, 30);
	mFilter.setBounds(getWidth() - 120, 0, 120, 30);
	mViewport.setBounds(0, 30, getWidth(), getHeight() - 30);
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
			
			// Update UI visibility based on current state
			resized();
			
			// Trigger repaint to update viewport scrollbars and empty state
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
