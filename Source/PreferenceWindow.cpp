/*
  ==============================================================================

    PreferenceWindow.cpp
    Created: 13 Jun 2020 3:51:18pm
    Author:  Jake Rose

  ==============================================================================
*/

#include "PreferenceWindow.h"
#include "SamplifyLookAndFeel.h"
#include "SamplifyMainComponent.h"
#include "ThemeManager.h"
#include "SampleDirectory.h"
#include "UI/OverlayPanel.h"

using namespace samplore;

PreferencePanel::PreferencePanel()
{
    auto& theme = ThemeManager::getInstance();
    
    // ===== THEME SECTION =====
    mThemeLabel.setText("Theme", dontSendNotification);
    mThemeLabel.setFont(FontOptions(18.0f, Font::bold));
    mThemeLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    addAndMakeVisible(mThemeLabel);

    mThemeSelector.setName("Theme Selector");
    mThemeSelector.addItem("Dark Theme", 1);
    mThemeSelector.addItem("Light Theme", 2);
    mThemeSelector.setSelectedId(
        theme.getCurrentTheme() == ThemeManager::Theme::Dark ? 1 : 2,
        dontSendNotification
    );
    mThemeSelector.addListener(this);
    addAndMakeVisible(mThemeSelector);

    // ===== COLOR CUSTOMIZATION SECTION =====
    mColorCustomizationLabel.setText("Custom Colors", dontSendNotification);
    mColorCustomizationLabel.setFont(FontOptions(18.0f, Font::bold));
    mColorCustomizationLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    addAndMakeVisible(mColorCustomizationLabel);

    mPrimaryColorLabel.setText("Primary Color:", dontSendNotification);
    mPrimaryColorLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
    addAndMakeVisible(mPrimaryColorLabel);

    mPrimaryColorButton.setName("Primary Color");
    mPrimaryColorButton.setButtonText("Choose Color");
    mPrimaryColorButton.addListener(this);
    addAndMakeVisible(mPrimaryColorButton);

    mAccentColorLabel.setText("Accent Color:", dontSendNotification);
    mAccentColorLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
    addAndMakeVisible(mAccentColorLabel);

    mAccentColorButton.setName("Accent Color");
    mAccentColorButton.setButtonText("Choose Color");
    mAccentColorButton.addListener(this);
    addAndMakeVisible(mAccentColorButton);

    mResetColorsButton.setName("Reset Colors");
    mResetColorsButton.setButtonText("Reset to Default");
    mResetColorsButton.addListener(this);
    addAndMakeVisible(mResetColorsButton);

    // ===== COLOR PRESETS SECTION =====
    mColorPresetsLabel.setText("Color Presets", dontSendNotification);
    mColorPresetsLabel.setFont(FontOptions(18.0f, Font::bold));
    mColorPresetsLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    addAndMakeVisible(mColorPresetsLabel);

    mPresetStudioDark.setName("Preset Studio Dark");
    mPresetStudioDark.setButtonText("Studio Dark");
    mPresetStudioDark.addListener(this);
    addAndMakeVisible(mPresetStudioDark);

    mPresetStudioLight.setName("Preset Studio Light");
    mPresetStudioLight.setButtonText("Studio Light");
    mPresetStudioLight.addListener(this);
    addAndMakeVisible(mPresetStudioLight);

    mPresetAbleton.setName("Preset Ableton");
    mPresetAbleton.setButtonText("Warm Orange");
    mPresetAbleton.addListener(this);
    addAndMakeVisible(mPresetAbleton);

    mPresetProTools.setName("Preset ProTools");
    mPresetProTools.setButtonText("Cool Blue");
    mPresetProTools.addListener(this);
    addAndMakeVisible(mPresetProTools);

    mPresetHighContrast.setName("Preset High Contrast");
    mPresetHighContrast.setButtonText("High Contrast");
    mPresetHighContrast.addListener(this);
    addAndMakeVisible(mPresetHighContrast);

    // ===== APPEARANCE SECTION =====
    mAppearanceLabel.setText("Appearance", dontSendNotification);
    mAppearanceLabel.setFont(FontOptions(18.0f, Font::bold));
    mAppearanceLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    addAndMakeVisible(mAppearanceLabel);

    mTileSizeLabel.setText("Min Tile Size (px):", dontSendNotification);
    mTileSizeLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
    addAndMakeVisible(mTileSizeLabel);

    mSampleMinSizeValue.setName("Tile Size");
    mSampleMinSizeValue.setInputRestrictions(3, "0123456789");
    mSampleMinSizeValue.setText(String(AppValues::getInstance().SAMPLE_TILE_MIN_WIDTH));
    mSampleMinSizeValue.addListener(this);
    addAndMakeVisible(mSampleMinSizeValue);

    mThumbnailLinesLabel.setText("Tile Waveform Lines:", dontSendNotification);
    mThumbnailLinesLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
    addAndMakeVisible(mThumbnailLinesLabel);

    mThumbnailLineCount.setName("Waveform Lines");
    mThumbnailLineCount.setInputRestrictions(3, "0123456789");
    mThumbnailLineCount.setText(String(AppValues::getInstance().AUDIO_THUMBNAIL_LINE_COUNT));
    mThumbnailLineCount.addListener(this);
    addAndMakeVisible(mThumbnailLineCount);


    mThumbnailLinesPlayerLabel.setText("Player Waveform Lines:", dontSendNotification);
    mThumbnailLinesPlayerLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
    addAndMakeVisible(mThumbnailLinesPlayerLabel);

    mThumbnailLineCountPlayer.setName("Player Waveform Lines");
    mThumbnailLineCountPlayer.setInputRestrictions(3, "0123456789");
    mThumbnailLineCountPlayer.setText(String(AppValues::getInstance().AUDIO_THUMBNAIL_LINE_COUNT_PLAYER));
    mThumbnailLineCountPlayer.addListener(this);
    addAndMakeVisible(mThumbnailLineCountPlayer);

    mPlaybackIndicatorLabel.setText("Playback Indicator:", dontSendNotification);
    mPlaybackIndicatorLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
    addAndMakeVisible(mPlaybackIndicatorLabel);

    mPlaybackIndicatorModeSelector.setName("Playback Indicator Mode");
    mPlaybackIndicatorModeSelector.addItem("Animated Rainbow", 1);
    mPlaybackIndicatorModeSelector.addItem("Static Rainbow", 2);
    mPlaybackIndicatorModeSelector.addItem("Static Color", 3);
    mPlaybackIndicatorModeSelector.setSelectedId(
        static_cast<int>(AppValues::getInstance().PLAYBACK_INDICATOR_MODE) + 1,
        dontSendNotification
    );
    mPlaybackIndicatorModeSelector.addListener(this);
    addAndMakeVisible(mPlaybackIndicatorModeSelector);

    mPlaybackIndicatorColorButton.setName("Playback Indicator Color");
    mPlaybackIndicatorColorButton.setButtonText("Choose Color");
    mPlaybackIndicatorColorButton.addListener(this);
    addAndMakeVisible(mPlaybackIndicatorColorButton);
    updatePlaybackIndicatorColorButton();

    // ===== KEY BINDINGS SECTION =====
    mKeyBindingsLabel.setText("Key Bindings", dontSendNotification);
    mKeyBindingsLabel.setFont(FontOptions(18.0f, Font::bold));
    mKeyBindingsLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    addAndMakeVisible(mKeyBindingsLabel);

    mEditKeyBindingsButton.setName("Edit Key Bindings");
    mEditKeyBindingsButton.setButtonText("Edit Key Bindings");
    mEditKeyBindingsButton.addListener(this);
    addAndMakeVisible(mEditKeyBindingsButton);
    
    // ===== DIRECTORY MANAGEMENT SECTION =====
    mDirectoryManagementLabel.setText("Directory Management", dontSendNotification);
    mDirectoryManagementLabel.setFont(FontOptions(18.0f, Font::bold));
    mDirectoryManagementLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    addAndMakeVisible(mDirectoryManagementLabel);

    mAddDirectoryButton.setName("Add Directory");
    mAddDirectoryButton.setButtonText("Add Directory");
    mAddDirectoryButton.addListener(this);
    addAndMakeVisible(mAddDirectoryButton);

    // Setup directory list viewport
    addAndMakeVisible(mDirectoryViewport);
    mDirectoryViewport.setViewedComponent(&mDirectoryListContainer, false);
    mDirectoryViewport.setScrollBarsShown(true, false, true, false);

    // Initialize all component colors
    updateAllComponentColors();
    updateColorButtons();
    updateDirectoryList();
    
    // Register with ThemeManager
    ThemeManager::getInstance().addListener(this);
}

PreferencePanel::~PreferencePanel()
{
    // Explicitly delete all directory list items to release SampleDirectory shared_ptrs
    mDirectoryListContainer.deleteAllChildren();
    
    ThemeManager::getInstance().removeListener(this);
}

void PreferencePanel::buttonClicked(Button* button)
{
    auto& theme = ThemeManager::getInstance();
    String buttonName = button->getName();

    // Color picker buttons
    if (buttonName == "Primary Color")
    {
        mColorEditMode = ColorEditMode::Primary;
        mColourSelector = std::make_unique<ColourSelector>();
        mColourSelector->setSize(300, 300);
        mColourSelector->setCurrentColour(theme.getColorForRole(ThemeManager::ColorRole::AccentPrimary));
        mColourSelector->addChangeListener(this);
        CallOutBox::launchAsynchronously(std::move(mColourSelector), button->getScreenBounds(), nullptr);
    }
    else if (buttonName == "Accent Color")
    {
        mColorEditMode = ColorEditMode::Accent;
        mColourSelector = std::make_unique<ColourSelector>();
        mColourSelector->setSize(300, 300);
        mColourSelector->setCurrentColour(theme.getColorForRole(ThemeManager::ColorRole::AccentSecondary));
        mColourSelector->addChangeListener(this);
        CallOutBox::launchAsynchronously(std::move(mColourSelector), button->getScreenBounds(), nullptr);
    }
    else if (buttonName == "Reset Colors")
    {
        theme.resetToDefaultColors();
        SamplifyMainComponent::setupLookAndFeel(getLookAndFeel());
        updateColorButtons();
        if (auto* mainComp = SamplifyMainComponent::getInstance())
        {
            mainComp->getSamplePlayerComponent().updateThemeColors();
            mainComp->repaint();
        }
        repaint();
    }
    // Color preset buttons
    else if (buttonName == "Preset Studio Dark")
    {
        applyColorPreset("Studio Dark");
    }
    else if (buttonName == "Preset Studio Light")
    {
        applyColorPreset("Studio Light");
    }
    else if (buttonName == "Preset Ableton")
    {
        applyColorPreset("Ableton");
    }
    else if (buttonName == "Preset ProTools")
    {
        applyColorPreset("ProTools");
    }
    else if (buttonName == "Preset High Contrast")
    {
        applyColorPreset("High Contrast");
    }
    else if (buttonName == "Edit Key Bindings")
    {
        // Launch key binding editor dialog
        DialogWindow::LaunchOptions options;
        auto* editor = new KeyBindingEditor();
        options.content.setOwned(editor);
        options.dialogTitle = "Edit Key Bindings";
        options.dialogBackgroundColour = ThemeManager::getInstance().getColorForRole(ThemeManager::ColorRole::Background);
        options.escapeKeyTriggersCloseButton = true;
        options.useNativeTitleBar = false;
        options.resizable = false;
        
        auto* dialog = options.launchAsync();
        dialog->centreWithSize(500, 400);
    }
    else if (buttonName == "Add Directory")
    {
        SamplifyProperties::getInstance()->browseForDirectory([](const File& dir)
        {
            if (dir.exists())
            {
                SamplifyProperties::getInstance()->getSampleLibrary()->addDirectory(dir);
            }
        });
    }
    else if (buttonName == "Playback Indicator Color")
    {
        mColorEditMode = ColorEditMode::PlaybackIndicator;
        mColourSelector = std::make_unique<ColourSelector>();
        mColourSelector->setSize(300, 300);
        mColourSelector->setCurrentColour(AppValues::getInstance().PLAYBACK_INDICATOR_COLOR);
        mColourSelector->addChangeListener(this);
        CallOutBox::launchAsynchronously(std::move(mColourSelector), button->getScreenBounds(), nullptr);
    }
}

void PreferencePanel::changeListenerCallback(ChangeBroadcaster* source)
{
    if (auto* selector = dynamic_cast<ColourSelector*>(source))
    {
        Colour newColour = selector->getCurrentColour();
        auto& theme = ThemeManager::getInstance();

        if (mColorEditMode == ColorEditMode::Primary)
        {
            theme.setCustomColor(ThemeManager::ColorRole::AccentPrimary, newColour);
            theme.setCustomColor(ThemeManager::ColorRole::WaveformPrimary, newColour);
        }
        else if (mColorEditMode == ColorEditMode::Accent)
        {
            theme.setCustomColor(ThemeManager::ColorRole::AccentSecondary, newColour);
            theme.setCustomColor(ThemeManager::ColorRole::WaveformSecondary, newColour);
        }
        else // PlaybackIndicator
        {
            AppValues::getInstance().PLAYBACK_INDICATOR_COLOR = newColour;
            updatePlaybackIndicatorColorButton();
            return; // No need to update other colors
        }

        SamplifyMainComponent::setupLookAndFeel(getLookAndFeel());
        updateColorButtons();

        if (auto* mainComp = SamplifyMainComponent::getInstance())
        {
            mainComp->getSamplePlayerComponent().updateThemeColors();
            mainComp->repaint();
        }
        repaint();
    }
}

void PreferencePanel::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &mThemeSelector)
    {
        int selectedId = mThemeSelector.getSelectedId();
        ThemeManager::Theme newTheme = (selectedId == 1) ?
            ThemeManager::Theme::Dark : ThemeManager::Theme::Light;

        ThemeManager::getInstance().setTheme(newTheme);
        SamplifyMainComponent::setupLookAndFeel(getLookAndFeel());

        // Update all components to reflect new theme
        if (auto* mainComp = SamplifyMainComponent::getInstance())
        {
            mainComp->getSamplePlayerComponent().updateThemeColors();
            mainComp->repaint();
        }
        repaint();
    }
    else if (comboBoxThatHasChanged == &mPlaybackIndicatorModeSelector)
    {
        int selectedId = mPlaybackIndicatorModeSelector.getSelectedId();
        AppValues::getInstance().PLAYBACK_INDICATOR_MODE = static_cast<PlaybackIndicatorMode>(selectedId - 1);
        updatePlaybackIndicatorColorButton();
    }
}

void PreferencePanel::textEditorTextChanged(TextEditor& editor)
{
    if (editor.getName() == "Tile Size")
    {
        if (editor.getText().length() > 0)
        {
            AppValues::getInstance().SAMPLE_TILE_MIN_WIDTH = std::stoi(editor.getText().toStdString());
        }
    }
    else if (editor.getName() == "Waveform Lines")
    {
        if (editor.getText().length() > 0)
        {
            AppValues::getInstance().AUDIO_THUMBNAIL_LINE_COUNT = std::stoi(editor.getText().toStdString());
        }
    }
    else if (editor.getName() == "Player Waveform Lines")
    {
        if (editor.getText().length() > 0)
        {
            AppValues::getInstance().AUDIO_THUMBNAIL_LINE_COUNT_PLAYER = std::stoi(editor.getText().toStdString());
        }
    }
}
void PreferencePanel::updateColorButtons()
{
    auto& theme = ThemeManager::getInstance();

    Colour primaryColor = theme.getColorForRole(ThemeManager::ColorRole::AccentPrimary);
    mPrimaryColorButton.setColour(TextButton::buttonColourId, primaryColor);
    mPrimaryColorButton.setColour(TextButton::textColourOffId,
        primaryColor.getPerceivedBrightness() > 0.5f ? Colours::black : Colours::white);

    Colour accentColor = theme.getColorForRole(ThemeManager::ColorRole::AccentSecondary);
    mAccentColorButton.setColour(TextButton::buttonColourId, accentColor);
    mAccentColorButton.setColour(TextButton::textColourOffId,
        accentColor.getPerceivedBrightness() > 0.5f ? Colours::black : Colours::white);
}

void PreferencePanel::updatePlaybackIndicatorColorButton()
{
    // Show/hide color button based on selected mode
    bool showColorButton = AppValues::getInstance().PLAYBACK_INDICATOR_MODE == PlaybackIndicatorMode::StaticColor;
    mPlaybackIndicatorColorButton.setVisible(showColorButton);

    if (showColorButton)
    {
        Colour indicatorColor = AppValues::getInstance().PLAYBACK_INDICATOR_COLOR;
        mPlaybackIndicatorColorButton.setColour(TextButton::buttonColourId, indicatorColor);
        mPlaybackIndicatorColorButton.setColour(TextButton::textColourOffId,
            indicatorColor.getPerceivedBrightness() > 0.5f ? Colours::black : Colours::white);
    }

    resized(); // Update layout
}

void PreferencePanel::applyColorPreset(const String& presetName)
{
    auto& theme = ThemeManager::getInstance();

    if (presetName == "Studio Dark")
    {
        theme.setCustomColor(ThemeManager::ColorRole::AccentPrimary, Colour(0xFF4A9EFF));
        theme.setCustomColor(ThemeManager::ColorRole::AccentSecondary, Colour(0xFF7B61FF));
    }
    else if (presetName == "Studio Light")
    {
        theme.setCustomColor(ThemeManager::ColorRole::AccentPrimary, Colour(0xFF007AFF));
        theme.setCustomColor(ThemeManager::ColorRole::AccentSecondary, Colour(0xFF5E5CE6));
    }
    else if (presetName == "Ableton")
    {
        // Warm orange/amber theme
        theme.setCustomColor(ThemeManager::ColorRole::AccentPrimary, Colour(0xFFFF7A3D));
        theme.setCustomColor(ThemeManager::ColorRole::AccentSecondary, Colour(0xFFFFA500));
    }
    else if (presetName == "ProTools")
    {
        // Cool blue/cyan theme
        theme.setCustomColor(ThemeManager::ColorRole::AccentPrimary, Colour(0xFF00A3E0));
        theme.setCustomColor(ThemeManager::ColorRole::AccentSecondary, Colour(0xFF00CED1));
    }
    else if (presetName == "High Contrast")
    {
        // Maximum contrast yellow on dark
        theme.setCustomColor(ThemeManager::ColorRole::AccentPrimary, Colour(0xFFFFFF00));
        theme.setCustomColor(ThemeManager::ColorRole::AccentSecondary, Colour(0xFFFF00FF));
    }

    // Update waveform colors to match
    theme.setCustomColor(ThemeManager::ColorRole::WaveformPrimary,
        theme.getColorForRole(ThemeManager::ColorRole::AccentPrimary));
    theme.setCustomColor(ThemeManager::ColorRole::WaveformSecondary,
        theme.getColorForRole(ThemeManager::ColorRole::AccentSecondary));

    SamplifyMainComponent::setupLookAndFeel(getLookAndFeel());
    updateColorButtons();

    if (auto* mainComp = SamplifyMainComponent::getInstance())
    {
        mainComp->getSamplePlayerComponent().updateThemeColors();
        mainComp->repaint();
    }
    repaint();
}

void PreferencePanel::paint(Graphics& g)
{
    auto& theme = ThemeManager::getInstance();
    g.fillAll(theme.getColorForRole(ThemeManager::ColorRole::Background));

    // Draw section separators
    g.setColour(theme.getColorForRole(ThemeManager::ColorRole::Border));
    g.drawLine(16, 80, getWidth() - 16, 80, 1.0f);      // After theme section
    g.drawLine(16, 280, getWidth() - 16, 280, 1.0f);   // After color customization
    g.drawLine(16, 430, getWidth() - 16, 430, 1.0f);   // After color presets
    g.drawLine(16, 580, getWidth() - 16, 580, 1.0f);   // After appearance section
    g.drawLine(16, 730, getWidth() - 16, 730, 1.0f);   // After key bindings section
}

void PreferencePanel::resized()
{
    auto bounds = getLocalBounds();
    
    const int margin = 16;
    const int labelHeight = 24;
    const int controlHeight = 36;
    const int sectionSpacing = 24;
    const int itemSpacing = 8;

    int y = bounds.getY() + margin;
    int componentWidth = bounds.getWidth();

    // ===== THEME SECTION =====
    mThemeLabel.setBounds(margin, y, componentWidth - 2 * margin, labelHeight);
    y += labelHeight + itemSpacing;

    mThemeSelector.setBounds(margin, y, componentWidth - 2 * margin, controlHeight);
    y += controlHeight + sectionSpacing;

    // Separator line drawn in paint()
    y += itemSpacing;

    // ===== COLOR CUSTOMIZATION SECTION =====
    mColorCustomizationLabel.setBounds(margin, y, componentWidth - 2 * margin, labelHeight);
    y += labelHeight + itemSpacing;

    // Primary color row
    mPrimaryColorLabel.setBounds(margin, y, 120, controlHeight);
    mPrimaryColorButton.setBounds(margin + 130, y, componentWidth - margin - 130 - margin, controlHeight);
    y += controlHeight + itemSpacing;

    // Accent color row
    mAccentColorLabel.setBounds(margin, y, 120, controlHeight);
    mAccentColorButton.setBounds(margin + 130, y, componentWidth - margin - 130 - margin, controlHeight);
    y += controlHeight + itemSpacing;

    // Reset button
    mResetColorsButton.setBounds(margin, y, componentWidth - 2 * margin, controlHeight);
    y += controlHeight + sectionSpacing;

    // Separator line drawn in paint()
    y += itemSpacing;

    // ===== COLOR PRESETS SECTION =====
    mColorPresetsLabel.setBounds(margin, y, componentWidth - 2 * margin, labelHeight);
    y += labelHeight + itemSpacing;

    // Preset buttons in a grid (2 columns)
    int presetWidth = (componentWidth - 3 * margin) / 2;
    mPresetStudioDark.setBounds(margin, y, presetWidth, controlHeight);
    mPresetStudioLight.setBounds(margin + presetWidth + margin, y, presetWidth, controlHeight);
    y += controlHeight + itemSpacing;

    mPresetAbleton.setBounds(margin, y, presetWidth, controlHeight);
    mPresetProTools.setBounds(margin + presetWidth + margin, y, presetWidth, controlHeight);
    y += controlHeight + itemSpacing;

    mPresetHighContrast.setBounds(margin, y, componentWidth - 2 * margin, controlHeight);
    y += controlHeight + sectionSpacing;

    // Separator line drawn in paint()
    y += itemSpacing;

    // ===== APPEARANCE SECTION =====
    mAppearanceLabel.setBounds(margin, y, componentWidth - 2 * margin, labelHeight);
    y += labelHeight + itemSpacing;

    // Tile size row
    mTileSizeLabel.setBounds(margin, y, 140, controlHeight);
    mSampleMinSizeValue.setBounds(margin + 150, y, 100, controlHeight);
    y += controlHeight + itemSpacing;

    // Waveform lines row (Tile)
    mThumbnailLinesLabel.setBounds(margin, y, 180, controlHeight);
    mThumbnailLineCount.setBounds(margin + 190, y, 100, controlHeight);
    y += controlHeight + itemSpacing;

    // Player waveform lines row
    mThumbnailLinesPlayerLabel.setBounds(margin, y, 180, controlHeight);
    mThumbnailLineCountPlayer.setBounds(margin + 190, y, 100, controlHeight);
    y += controlHeight + itemSpacing;

    // Playback indicator mode row
    mPlaybackIndicatorLabel.setBounds(margin, y, 140, controlHeight);
    mPlaybackIndicatorModeSelector.setBounds(margin + 150, y, componentWidth - margin - 150 - margin, controlHeight);
    y += controlHeight + itemSpacing;

    // Playback indicator color button (only visible for static color mode)
    if (mPlaybackIndicatorColorButton.isVisible())
    {
        mPlaybackIndicatorColorButton.setBounds(margin + 150, y, componentWidth - margin - 150 - margin, controlHeight);
        y += controlHeight + itemSpacing;
    }

    y += margin - itemSpacing; // Add spacing before separator

    // Separator line drawn in paint()
    y += itemSpacing;

    // ===== KEY BINDINGS SECTION =====
    mKeyBindingsLabel.setBounds(margin, y, componentWidth - 2 * margin, labelHeight);
    y += labelHeight + itemSpacing;
    
    mEditKeyBindingsButton.setBounds(margin, y, componentWidth - 2 * margin, controlHeight);
    y += controlHeight + sectionSpacing;
    
    // Separator line drawn in paint()
    y += itemSpacing;

    // ===== DIRECTORY MANAGEMENT SECTION =====
    mDirectoryManagementLabel.setBounds(margin, y, componentWidth - 2 * margin, labelHeight);
    y += labelHeight + itemSpacing;
    
    mAddDirectoryButton.setBounds(margin, y, componentWidth - 2 * margin, controlHeight);
    y += controlHeight + itemSpacing;
    
    // Directory list viewport
    int listHeight = 120; // Fixed height for directory list
    mDirectoryViewport.setBounds(margin, y, componentWidth - 2 * margin, listHeight);
    y += listHeight + itemSpacing;

    // Total height for scrolling
    y += sectionSpacing;
    setSize(componentWidth, y);
}

void PreferencePanel::updateAllComponentColors()
{
    auto& theme = ThemeManager::getInstance();
    
    // Update all labels
    mThemeLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    mColorCustomizationLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    mPrimaryColorLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
    mAccentColorLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
    mColorPresetsLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    mAppearanceLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    mTileSizeLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
    mThumbnailLinesLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
    mThumbnailLinesPlayerLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
    mKeyBindingsLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    
    // Update text editors
    mSampleMinSizeValue.setColour(TextEditor::backgroundColourId, theme.getColorForRole(ThemeManager::ColorRole::Surface));
    mSampleMinSizeValue.setColour(TextEditor::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    mSampleMinSizeValue.setColour(TextEditor::outlineColourId, theme.getColorForRole(ThemeManager::ColorRole::Border));
    mSampleMinSizeValue.applyColourToAllText(theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    mSampleMinSizeValue.repaint();
    
    mThumbnailLineCount.setColour(TextEditor::backgroundColourId, theme.getColorForRole(ThemeManager::ColorRole::Surface));
    mThumbnailLineCount.setColour(TextEditor::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    mThumbnailLineCount.setColour(TextEditor::outlineColourId, theme.getColorForRole(ThemeManager::ColorRole::Border));
    mThumbnailLineCount.applyColourToAllText(theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    mThumbnailLineCount.repaint();
    
    mThumbnailLineCountPlayer.setColour(TextEditor::backgroundColourId, theme.getColorForRole(ThemeManager::ColorRole::Surface));
    mThumbnailLineCountPlayer.setColour(TextEditor::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    mThumbnailLineCountPlayer.setColour(TextEditor::outlineColourId, theme.getColorForRole(ThemeManager::ColorRole::Border));
    mThumbnailLineCountPlayer.applyColourToAllText(theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    mThumbnailLineCountPlayer.repaint();

    // Update playback indicator controls
    mPlaybackIndicatorLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
    mPlaybackIndicatorModeSelector.setColour(ComboBox::backgroundColourId, theme.getColorForRole(ThemeManager::ColorRole::Surface));
    mPlaybackIndicatorModeSelector.setColour(ComboBox::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    mPlaybackIndicatorModeSelector.setColour(ComboBox::outlineColourId, theme.getColorForRole(ThemeManager::ColorRole::Border));
    mPlaybackIndicatorModeSelector.setColour(ComboBox::arrowColourId, theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
    mPlaybackIndicatorModeSelector.repaint();
    updatePlaybackIndicatorColorButton();

    // Update key bindings button
    mEditKeyBindingsButton.setColour(TextButton::buttonColourId, theme.getColorForRole(ThemeManager::ColorRole::AccentSecondary));
    mEditKeyBindingsButton.setColour(TextButton::textColourOnId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    
    // Update directory management components
    mDirectoryManagementLabel.setColour(Label::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    mAddDirectoryButton.setColour(TextButton::buttonColourId, theme.getColorForRole(ThemeManager::ColorRole::AccentSecondary));
    mAddDirectoryButton.setColour(TextButton::textColourOnId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    // Viewport colors will be handled by theme system
    
    // Update combo box
    mThemeSelector.setColour(ComboBox::backgroundColourId, theme.getColorForRole(ThemeManager::ColorRole::Surface));
    mThemeSelector.setColour(ComboBox::textColourId, theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    mThemeSelector.setColour(ComboBox::outlineColourId, theme.getColorForRole(ThemeManager::ColorRole::Border));
    mThemeSelector.setColour(ComboBox::arrowColourId, theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
    mThemeSelector.repaint();
}

//==============================================================================
// ThemeManager::Listener implementation
void PreferencePanel::themeChanged(ThemeManager::Theme newTheme)
{
    // Update all component colors
    updateAllComponentColors();
    
    // Update theme selector dropdown
    mThemeSelector.setSelectedId(
        newTheme == ThemeManager::Theme::Dark ? 1 : 2,
        dontSendNotification
    );
    
    updateColorButtons();
    updateDirectoryList();
    repaint();
}

void PreferencePanel::colorChanged(ThemeManager::ColorRole role, Colour newColor)
{
    // Update all component colors when any color changes
    updateAllComponentColors();
    updateColorButtons();
    updateDirectoryList();
    repaint();
}

void PreferencePanel::updateDirectoryList()
{
    // Clear existing list items (deleteChildComponents = true to actually delete them)
    mDirectoryListContainer.deleteAllChildren();
    
    const auto& dirs = SamplifyProperties::getInstance()->getSampleLibrary()->getDirectories();
    int y = 0;
    const int itemHeight = 40;
    int width = 550; // Fixed width based on viewport
    
    for (int i = 0; i < dirs.size(); i++)
    {
        bool isActive = (dirs[i]->getCheckStatus() == CheckStatus::Enabled || 
                        dirs[i]->getCheckStatus() == CheckStatus::Mixed);
        auto* item = new DirectoryListItem(dirs[i], isActive, this);
        item->setBounds(0, y, width, itemHeight);
        mDirectoryListContainer.addAndMakeVisible(item);
        y += itemHeight;
    }
    
    // Update container height
    mDirectoryListContainer.setSize(width, y);
}

// DirectoryListItem implementation
PreferencePanel::DirectoryListItem::DirectoryListItem(std::shared_ptr<SampleDirectory> dir, bool isActive, PreferencePanel* parent)
    : mDirectory(dir), mParentView(parent)
{
    addAndMakeVisible(mActiveCheckbox);
    mActiveCheckbox.setToggleState(isActive, dontSendNotification);
    mActiveCheckbox.addListener(this);
    
    addAndMakeVisible(mDeleteButton);
    mDeleteButton.setButtonText("X");
    mDeleteButton.addListener(this);
}

PreferencePanel::DirectoryListItem::~DirectoryListItem()
{
    mActiveCheckbox.removeListener(this);
    mDeleteButton.removeListener(this);
}

void PreferencePanel::DirectoryListItem::paint(Graphics& g)
{
    auto& theme = ThemeManager::getInstance();
    g.fillAll(theme.getColorForRole(ThemeManager::ColorRole::Surface));
    
    // Draw border
    g.setColour(theme.getColorForRole(ThemeManager::ColorRole::Border));
    g.drawRect(getLocalBounds(), 1);
    
    // Draw directory path
    g.setColour(theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
    g.setFont(13.0f);
    if (mDirectory)
    {
        int textX = 30; // After checkbox
        int textWidth = getWidth() - textX - 40; // Before delete button
        g.drawText(mDirectory->getFile().getFullPathName(), textX, 0, textWidth, getHeight(), Justification::centredLeft);
    }
}

void PreferencePanel::DirectoryListItem::resized()
{
    mActiveCheckbox.setBounds(5, (getHeight() - 20) / 2, 20, 20);
    mDeleteButton.setBounds(getWidth() - 30, (getHeight() - 25) / 2, 25, 25);
}

void PreferencePanel::DirectoryListItem::buttonClicked(Button* button)
{
    if (button == &mDeleteButton)
    {
        // Show confirmation dialog
        if (mDirectory)
        {
            String message = "Are you sure you want to remove this directory?\n\n";
            message += mDirectory->getFile().getFullPathName();
            message += "\n\nThis will not delete any files from your computer.";
            
            auto options = MessageBoxOptions()
                .withIconType(MessageBoxIconType::QuestionIcon)
                .withTitle("Remove Directory")
                .withMessage(message)
                .withButton("Remove")
                .withButton("Cancel");
            
            AlertWindow::showAsync(options, [this](int result)
            {
                if (result == 1 && mDirectory) // "Remove" button clicked
                {
                    SamplifyProperties::getInstance()->getSampleLibrary()->removeDirectory(mDirectory->getFile());
                }
            });
        }
    }
    else if (button == &mActiveCheckbox)
    {
        // Toggle directory active/inactive state
        if (mDirectory)
        {
            CheckStatus newStatus = mActiveCheckbox.getToggleState() ? 
                CheckStatus::Enabled : CheckStatus::Disabled;
            mDirectory->setCheckStatus(newStatus);
            
            // Trigger library update to refresh samples
            SamplifyProperties::getInstance()->getSampleLibrary()->refreshCurrentSamples();
        }
    }
}
