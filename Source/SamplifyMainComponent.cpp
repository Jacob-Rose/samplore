#include "SamplifyMainComponent.h"
#include "SamplifyLookAndFeel.h"
#include "ThemeManager.h"
#include "CueManager.h"

using namespace samplore;

SamplifyMainComponent* SamplifyMainComponent::mInstance = nullptr;

SamplifyMainComponent::SamplifyMainComponent() :
	mResizableEdgeDirectoryExplorer(&mLeftPanel, &mResizableEdgeDirectoryExplorerBounds, ResizableEdgeComponent::Edge::rightEdge),
	mResizableEdgeFilterExplorer(&mFilterExplorer, &mResizableEdgeDirectoryExplorerBounds, ResizableEdgeComponent::Edge::leftEdge),
	mResizableEdgeAudioPlayer(&mSamplePlayerComponent, &mResizableEdgeAudioPlayerBounds, ResizableEdgeComponent::Edge::topEdge)
{
	setupLookAndFeel(getLookAndFeel());
	mInstance = this;
	addKeyListener(this);

	mAudioPlayer = std::make_shared<AudioPlayer>();
	SamplifyProperties::getInstance()->setAudioPlayer(mAudioPlayer);

	// Configure tooltip to show faster
	mTooltip->setMillisecondsBeforeTipAppears(300); // Show after 300ms instead of default 700ms

	mResizableEdgeDirectoryExplorerBounds.setMinimumWidth(100);
	mResizableEdgeFilterExplorerBounds.setMinimumWidth(100);
	mResizableEdgeAudioPlayerBounds.setMinimumHeight(100);
	mResizableEdgeAudioPlayerBounds.setMaximumHeight(400);
	mLeftPanel.setSize(200, 1000); //todo make these save
	mFilterExplorer.setSize(200, 1000); //todo make these save
	mSamplePlayerComponent.setSize(200, 200); //todo make these save
	mResizableEdgeDirectoryExplorer.addMouseListener(this, false);
	mResizableEdgeFilterExplorer.addMouseListener(this, false);
	mResizableEdgeAudioPlayer.addMouseListener(this, false);
	addAndMakeVisible(mResizableEdgeFilterExplorer);
	addAndMakeVisible(mResizableEdgeDirectoryExplorer);
	addAndMakeVisible(mResizableEdgeAudioPlayer);

	addAndMakeVisible(mLeftPanel);
	addAndMakeVisible(mSampleExplorer);
	addAndMakeVisible(mFilterExplorer);
	addAndMakeVisible(mSamplePlayerComponent);
	
	// Setup central overlay panel
	addChildComponent(mOverlayPanel);

	//addAndMakeVisible(unlockForm);
    
	//Setup Audio
	AudioDeviceManager::AudioDeviceSetup adsetup;
	deviceManager.getAudioDeviceSetup(adsetup);
	adsetup.bufferSize = 512;
	adsetup.sampleRate = 48000;
	deviceManager.setAudioDeviceSetup(adsetup, true);
	//deviceManager.initialise(2,2,0,true,juce::String(), &adsetup);
	setAudioChannels(0, 2);

	SamplifyProperties::getInstance()->getSampleLibrary()->addChangeListener(&mSampleExplorer);
	SamplifyProperties::getInstance()->getAudioPlayer()->addChangeListener(&mSamplePlayerComponent);
	//startTimer(100);
	setSize(AppValues::getInstance().WINDOW_WIDTH, AppValues::getInstance().WINDOW_HEIGHT);
	//initial load
	SamplifyProperties::getInstance()->getSampleLibrary()->updateCurrentSamples({});
	
	// Register with ThemeManager
	ThemeManager::getInstance().addListener(this);

	// Register callbacks for global key bindings
	registerKeyBindingCallbacks();

	// Enable performance profiling in debug builds
	#if JUCE_DEBUG
		PerformanceProfiler::getInstance().setEnabled(true);
		DBG("Performance profiling enabled. Press F5 to view stats, F6 to reset.");
	#endif
}

SamplifyMainComponent::~SamplifyMainComponent()
{
	// CRITICAL: Remove member components as listeners BEFORE they're destroyed
	// Member variables are destroyed in reverse order of declaration
	if (auto lib = SamplifyProperties::getInstance()->getSampleLibrary())
		lib->removeChangeListener(&mSampleExplorer);
	if (auto player = SamplifyProperties::getInstance()->getAudioPlayer())
		player->removeChangeListener(&mSamplePlayerComponent);
	
	ThemeManager::getInstance().removeListener(this);
	shutdownAudio();
	if (mInstance == this)
	{
		mInstance = nullptr;
	}
}

bool SamplifyMainComponent::keyPressed(const KeyPress& key, Component* originatingComponent)
{
	// Performance profiling shortcuts (debug only) - check before context system
	#if JUCE_DEBUG
		if (key == KeyPress::F4Key)
		{
			auto& profiler = PerformanceProfiler::getInstance();
			profiler.setEnabled(!profiler.isEnabled());
			return true;
		}
		else if (key == KeyPress::F5Key)
		{
			PerformanceProfiler::getInstance().printStatistics();
			return true;
		}
		else if (key == KeyPress::F6Key)
		{
			PerformanceProfiler::getInstance().reset();
			return true;
		}
	#endif

	// Use the layered input context system - checks all contexts by priority
	// Cue context (priority 100) is checked before Global (priority 0)
	return InputContextManager::getInstance().handleKeyPress(key);
}

void SamplifyMainComponent::changeListenerCallback(ChangeBroadcaster* source)
{
	SamplifyProperties::getInstance()->savePropertiesFile();
}

void SamplifyMainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	if (mAudioPlayer != nullptr)
	{
		mAudioPlayer->prepareToPlay(samplesPerBlockExpected, sampleRate);
	}
}

void SamplifyMainComponent::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
	if (mAudioPlayer != nullptr)
	{
		mAudioPlayer->getNextAudioBlock(bufferToFill);
	}
}

void SamplifyMainComponent::releaseResources()
{
	if (mAudioPlayer != nullptr)
	{
		mAudioPlayer->releaseResources();
	}
}


void samplore::SamplifyMainComponent::setupLookAndFeel(LookAndFeel& laf)
{
	auto& theme = ThemeManager::getInstance();
	using CR = ThemeManager::ColorRole;

	// Window background
	laf.setColour(ResizableWindow::backgroundColourId, theme.getColorForRole(CR::Background));

	// SampleTile colors
	laf.setColour(SampleTile::backgroundDefaultColorID, theme.getColorForRole(CR::BackgroundTertiary));
	laf.setColour(SampleTile::backgroundHoverColorID, theme.getColorForRole(CR::SurfaceHover));
	laf.setColour(SampleTile::foregroundDefaultColorID, theme.getColorForRole(CR::AccentPrimary));
	laf.setColour(SampleTile::foregroundHoverColorID, theme.getColorForRole(CR::AccentPrimary).brighter(0.1f));

	// SampleExplorer
	laf.setColour(SampleExplorer::loadingWheelColorId, theme.getColorForRole(CR::AccentPrimary));

	// DirectoryExplorerTreeViewItem
	laf.setColour(DirectoryExplorerTreeViewItem::defaultBackgroundId, Colours::transparentBlack);
	laf.setColour(DirectoryExplorerTreeViewItem::selectedBackgroundId, theme.getColorForRole(CR::AccentPrimary).withAlpha(0.15f));
	laf.setColour(DirectoryExplorerTreeViewItem::checkboxActiveBackgroundId, theme.getColorForRole(CR::AccentPrimary));
	laf.setColour(DirectoryExplorerTreeViewItem::checkboxMixedBackgroundId, theme.getColorForRole(CR::AccentPrimary).withSaturation(0.3f));
	laf.setColour(DirectoryExplorerTreeViewItem::checkboxDisabledBackgroundId, theme.getColorForRole(CR::TextDisabled));
	laf.setColour(DirectoryExplorerTreeViewItem::checkboxNotLoadedBackgroundId, theme.getColorForRole(CR::Warning));

	// TextEditor
	laf.setColour(TextEditor::backgroundColourId, theme.getColorForRole(CR::BackgroundTertiary));
	laf.setColour(TextEditor::textColourId, theme.getColorForRole(CR::TextPrimary));
	laf.setColour(TextEditor::outlineColourId, theme.getColorForRole(CR::Border));
	laf.setColour(TextEditor::focusedOutlineColourId, theme.getColorForRole(CR::BorderFocus));

	// TextButton
	laf.setColour(TextButton::textColourOnId, Colours::white);
	laf.setColour(TextButton::buttonOnColourId, theme.getColorForRole(CR::AccentPrimary));
	laf.setColour(TextButton::buttonColourId, theme.getColorForRole(CR::Surface));
	laf.setColour(TextButton::textColourOffId, theme.getColorForRole(CR::TextPrimary));

	// ScrollBar
	laf.setColour(ScrollBar::thumbColourId, theme.getColorForRole(CR::TextSecondary).withAlpha(0.4f));
	laf.setColour(ScrollBar::trackColourId, Colours::transparentBlack);

	// ComboBox
	laf.setColour(ComboBox::backgroundColourId, theme.getColorForRole(CR::Surface));
	laf.setColour(ComboBox::textColourId, theme.getColorForRole(CR::TextPrimary));
	laf.setColour(ComboBox::arrowColourId, theme.getColorForRole(CR::TextSecondary));
	laf.setColour(ComboBox::outlineColourId, theme.getColorForRole(CR::Border));
	laf.setColour(ComboBox::buttonColourId, theme.getColorForRole(CR::AccentPrimary));
	laf.setColour(ComboBox::focusedOutlineColourId, theme.getColorForRole(CR::BorderFocus));

	// SamplePlayerComponent
	laf.setColour(SamplePlayerComponent::waveformColourId, theme.getColorForRole(CR::WaveformPrimary));

	// LookAndFeel_V4 default colors
	laf.setColour(LookAndFeel_V4::ColourScheme::UIColour::defaultFill, theme.getColorForRole(CR::Surface));
	laf.setColour(LookAndFeel_V4::ColourScheme::UIColour::defaultText, theme.getColorForRole(CR::TextPrimary));
	laf.setColour(LookAndFeel_V4::ColourScheme::UIColour::highlightedFill, theme.getColorForRole(CR::AccentPrimary));
	laf.setColour(LookAndFeel_V4::ColourScheme::UIColour::highlightedText, Colours::white);

	// PopupMenu
	laf.setColour(PopupMenu::backgroundColourId, theme.getColorForRole(CR::BackgroundSecondary));
	laf.setColour(PopupMenu::headerTextColourId, theme.getColorForRole(CR::TextPrimary));
	laf.setColour(PopupMenu::highlightedBackgroundColourId, theme.getColorForRole(CR::AccentPrimary));
	laf.setColour(PopupMenu::highlightedTextColourId, Colours::white);
	laf.setColour(PopupMenu::textColourId, theme.getColorForRole(CR::TextPrimary));
}

//==============================================================================
void SamplifyMainComponent::paint (Graphics& g)
{
	PROFILE_PAINT("SamplifyMainComponent::paint");
    g.fillAll (getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}
const int edgeSize = 8;
void SamplifyMainComponent::resized()
{
	int lWidth = mLeftPanel.getWidth(); //set by dragger
	mLeftPanel.setBounds(0, 0, lWidth, getHeight());
	mResizableEdgeDirectoryExplorer.setBounds(lWidth, 0, edgeSize, getHeight());
	lWidth += mResizableEdgeDirectoryExplorer.getWidth();
	
	int rWidth = mFilterExplorer.getWidth(); //set by dragger
	mFilterExplorer.setBounds(getWidth() - rWidth, 0, rWidth, getHeight()); 
	
	mResizableEdgeFilterExplorer.setBounds(getWidth() - rWidth - edgeSize, 0, edgeSize, getHeight());
	rWidth += mResizableEdgeFilterExplorer.getWidth();
	
	float bHeight = mSamplePlayerComponent.getHeight();
	mSamplePlayerComponent.setBounds(lWidth, getHeight() - bHeight, getWidth() - (lWidth + rWidth), bHeight);
	mResizableEdgeAudioPlayer.setBounds(lWidth, getHeight() - (bHeight + edgeSize), getWidth() - (lWidth + rWidth), edgeSize);
	bHeight += mResizableEdgeAudioPlayer.getHeight();
	
	mSampleExplorer.setBounds(lWidth, 0, getWidth() - (rWidth + lWidth), getHeight() - bHeight);
	
	// Overlay panel covers the entire component
	mOverlayPanel.setBounds(getLocalBounds());
	
	mResizableEdgeDirectoryExplorerBounds.setMaximumWidth(getWidth() - (rWidth));
	mResizableEdgeFilterExplorerBounds.setMaximumWidth(getWidth() - (lWidth));
}

SamplifyMainComponent* SamplifyMainComponent::getInstance()
{
	return mInstance;
}

void SamplifyMainComponent::timerCallback()
{
	/*
	if (!isUnlocked && authorizationStatus.isUnlocked())
	{
		isUnlocked = true;
		unlockApp();
	}
	*/
}

void SamplifyMainComponent::mouseDrag(const MouseEvent& e)
{
	resized();
}

void SamplifyMainComponent::showImportWizard()
{
	mImportWizard.showMainMenu(); // Reset to main menu
	mOverlayPanel.setContentComponent(&mImportWizard, false);
	mOverlayPanel.show();
}

void SamplifyMainComponent::showPreferences()
{
	mPreferencePanel.setSize(600, 1070); // 50px header + 20px spacing + 1000px content
	mOverlayPanel.setContentComponent(&mPreferencePanel, false);
	mOverlayPanel.show();
}

void SamplifyMainComponent::showCueBindingsWindow()
{
	if (mCueBindingsWindow == nullptr)
	{
		mCueBindingsWindow = std::make_unique<CueBindingsWindow>();
	}
	mCueBindingsWindow->setVisible(true);
	mCueBindingsWindow->toFront(true);
}

void SamplifyMainComponent::showKeyCaptureOverlay()
{
	mKeyCaptureOverlay.prepareForDisplay();
	mOverlayPanel.setContentComponent(&mKeyCaptureOverlay, false);
	mOverlayPanel.show();
}

//==============================================================================
// ThemeManager::Listener implementation
void SamplifyMainComponent::themeChanged(ThemeManager::Theme newTheme)
{
	setupLookAndFeel(getLookAndFeel());
	mSamplePlayerComponent.updateThemeColors();
	repaint();
}

void SamplifyMainComponent::colorChanged(ThemeManager::ColorRole role, Colour newColor)
{
	setupLookAndFeel(getLookAndFeel());
	mSamplePlayerComponent.updateThemeColors();
	repaint();
}

void SamplifyMainComponent::registerKeyBindingCallbacks()
{
	using Action = KeyBindingManager::Action;
	auto& keyManager = KeyBindingManager::getInstance();

	keyManager.setCallback(Action::PlayAudio, [this]() {
		SamplifyProperties::getInstance()->getAudioPlayer()->play();
	});

	keyManager.setCallback(Action::StopAudio, [this]() {
		SamplifyProperties::getInstance()->getAudioPlayer()->stop();
	});

	keyManager.setCallback(Action::TogglePlayerWindow, [this]() {
		mSamplePlayerComponent.setVisible(!mSamplePlayerComponent.isVisible());
	});

	keyManager.setCallback(Action::ToggleFilterWindow, [this]() {
		mFilterExplorer.setVisible(!mFilterExplorer.isVisible());
	});

	keyManager.setCallback(Action::OpenPreferences, [this]() {
		showPreferences();
	});

	keyManager.setCallback(Action::ExitApplication, []() {
		JUCEApplication::getInstance()->systemRequestedQuit();
	});

	keyManager.setCallback(Action::ToggleCueBindings, [this]() {
		showCueBindingsWindow();
	});
}
