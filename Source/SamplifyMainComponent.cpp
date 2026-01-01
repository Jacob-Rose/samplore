#include "SamplifyMainComponent.h"
#include "SamplifyLookAndFeel.h"
#include "ThemeManager.h"

using namespace samplore;

SamplifyMainComponent* SamplifyMainComponent::mInstance = nullptr;

SamplifyMainComponent::SamplifyMainComponent() : 
	mResizableEdgeDirectoryExplorer(&mDirectoryExplorer, &mResizableEdgeDirectoryExplorerBounds, ResizableEdgeComponent::Edge::rightEdge),
	mResizableEdgeFilterExplorer(&mFilterExplorer, &mResizableEdgeDirectoryExplorerBounds, ResizableEdgeComponent::Edge::leftEdge),
	mResizableEdgeAudioPlayer(&mSamplePlayerComponent, &mResizableEdgeAudioPlayerBounds, ResizableEdgeComponent::Edge::topEdge)
{
	setupLookAndFeel(getLookAndFeel());
	mInstance = this;
	addKeyListener(this);

	mAudioPlayer = std::make_shared<AudioPlayer>();
	SamplifyProperties::getInstance()->setAudioPlayer(mAudioPlayer);

	mResizableEdgeDirectoryExplorerBounds.setMinimumWidth(100);
	mResizableEdgeFilterExplorerBounds.setMinimumWidth(100);
	mResizableEdgeAudioPlayerBounds.setMinimumHeight(100);
	mResizableEdgeAudioPlayerBounds.setMaximumHeight(400);
	mDirectoryExplorer.setSize(200, 1000); //todo make these save
	mFilterExplorer.setSize(200, 1000); //todo make these save
	mSamplePlayerComponent.setSize(200, 200); //todo make these save
	mResizableEdgeDirectoryExplorer.addMouseListener(this, false);
	mResizableEdgeFilterExplorer.addMouseListener(this, false);
	mResizableEdgeAudioPlayer.addMouseListener(this, false);
	addAndMakeVisible(mResizableEdgeFilterExplorer);
	addAndMakeVisible(mResizableEdgeDirectoryExplorer);
	addAndMakeVisible(mResizableEdgeAudioPlayer);

	addAndMakeVisible(mDirectoryExplorer);
	addAndMakeVisible(mSampleExplorer);
	addAndMakeVisible(mFilterExplorer);
	addAndMakeVisible(mSamplePlayerComponent);
	
	// Setup import wizard overlay
	addChildComponent(mImportWizard);
	addChildComponent(mSpliceImportDialog);
	
	// Setup Splice import dialog callbacks
	mSpliceImportDialog.onImportComplete = [this](bool success, int samplesImported) {
		if (success)
		{
			DBG("Splice import completed successfully: " + String(samplesImported) + " samples");
		}
		else
		{
			DBG("Splice import failed or was cancelled");
		}
	};
	
	mImportWizard.onSpliceImport = [this]() {
		DBG("Splice Import selected");
		mImportWizard.hide();
		mSpliceImportDialog.show();
	};
	mImportWizard.onGeneralImport = [this]() {
		DBG("General Import selected");
		mImportWizard.hide();
		// TODO: Implement General Import
	};
	mImportWizard.onManualImport = [this]() {
		DBG("Manual Import selected");
		mImportWizard.hide();
		
		// Show directory chooser for manual import
		auto chooser = std::make_shared<FileChooser>("Select directory to import...",
			File::getSpecialLocation(File::userMusicDirectory),
			"",
			true);
		
		auto chooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;
		
		chooser->launchAsync(chooserFlags, [this, chooser](const FileChooser&) {
			File selectedDir = chooser->getResult();
			if (selectedDir.isDirectory())
			{
				auto library = SamplifyProperties::getInstance()->getSampleLibrary();
				if (library)
				{
					library->addDirectory(selectedDir);
					DBG("Added directory: " + selectedDir.getFullPathName());
				}
			}
		});
	};
	
	// Setup preference window overlay
	addChildComponent(mPreferenceWindow);

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
	SamplifyProperties::getInstance()->getSampleLibrary()->updateCurrentSamples("");
	
	// Register with ThemeManager
	ThemeManager::getInstance().addListener(this);
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
	auto& keyManager = KeyBindingManager::getInstance();
	
	if (keyManager.matchesAction(key, KeyBindingManager::Action::PlayAudio))
	{
		SamplifyProperties::getInstance()->getAudioPlayer()->play();
		return true;
	}
	else if (keyManager.matchesAction(key, KeyBindingManager::Action::StopAudio))
	{
		SamplifyProperties::getInstance()->getAudioPlayer()->stop();
		return true;
	}
	else if (keyManager.matchesAction(key, KeyBindingManager::Action::TogglePlayerWindow))
	{
		mSamplePlayerComponent.setVisible(!mSamplePlayerComponent.isVisible());
		return true;
	}
	else if (keyManager.matchesAction(key, KeyBindingManager::Action::ToggleFilterWindow))
	{
		mFilterExplorer.setVisible(!mFilterExplorer.isVisible());
		return true;
	}
	else if (keyManager.matchesAction(key, KeyBindingManager::Action::OpenPreferences))
	{
		showPreferences();
		return true;
	}
	else if (keyManager.matchesAction(key, KeyBindingManager::Action::ExitApplication))
	{
		JUCEApplication::getInstance()->systemRequestedQuit();
		return true;
	}
	
	return false;
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
    g.fillAll (getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}
const int edgeSize = 8;
void SamplifyMainComponent::resized()
{
	int lWidth = mDirectoryExplorer.getWidth(); //set by dragger
	mDirectoryExplorer.setBounds(0, 0, lWidth, getHeight());
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
	
	// Overlay panels cover the entire component
	mImportWizard.setBounds(getLocalBounds());
	mPreferenceWindow.setBounds(getLocalBounds());
	mSpliceImportDialog.setBounds(getLocalBounds());
	
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
	mImportWizard.show();
}

void SamplifyMainComponent::showPreferences()
{
	mPreferenceWindow.show();
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
