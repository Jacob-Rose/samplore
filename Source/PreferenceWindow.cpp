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

using namespace samplify;

PreferenceWindow::PreferenceWindow() : DialogWindow("Preferences", AppValues::getInstance().MAIN_BACKGROUND_COLOR, true)
{
    setContentComponent(&mView);
    setSize(600, 800);
    setWantsKeyboardFocus(true);
    setResizable(false, false);
    setUsingNativeTitleBar(false);
    setTitleBarButtonsRequired(closeButton, false);
}

void PreferenceWindow::closeButtonPressed()
{
    exitModalState(0);
}


bool PreferenceWindow::keyPressed(const KeyPress& key)
{
    // Handle Escape, Ctrl+W, or Ctrl+Q to close the preferences window
    // Use ctrlModifier on Linux/Windows, commandModifier on Mac
#if JUCE_MAC
    auto modifier = ModifierKeys::commandModifier;
#else
    auto modifier = ModifierKeys::ctrlModifier;
#endif

    if (key == KeyPress::escapeKey ||
        key == KeyPress('w', modifier, 0) ||
        key == KeyPress('q', modifier, 0))
    {
        exitModalState(0);
        return true;  // Consume the key press
    }

    return DialogWindow::keyPressed(key);
}

PreferenceWindow::View::View()
{
    auto& theme = ThemeManager::getInstance();

    // ===== THEME SECTION =====
    mThemeLabel.setText("Theme", dontSendNotification);
    mThemeLabel.setFont(Font(18.0f, Font::bold));
    mThemeLabel.setColour(Label::textColourId, theme.get(ThemeManager::ColorRole::TextPrimary));
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
    mColorCustomizationLabel.setFont(Font(18.0f, Font::bold));
    mColorCustomizationLabel.setColour(Label::textColourId, theme.get(ThemeManager::ColorRole::TextPrimary));
    addAndMakeVisible(mColorCustomizationLabel);

    mPrimaryColorLabel.setText("Primary Color:", dontSendNotification);
    mPrimaryColorLabel.setColour(Label::textColourId, theme.get(ThemeManager::ColorRole::TextSecondary));
    addAndMakeVisible(mPrimaryColorLabel);

    mPrimaryColorButton.setName("Primary Color");
    mPrimaryColorButton.setButtonText("Choose Color");
    mPrimaryColorButton.addListener(this);
    addAndMakeVisible(mPrimaryColorButton);

    mAccentColorLabel.setText("Accent Color:", dontSendNotification);
    mAccentColorLabel.setColour(Label::textColourId, theme.get(ThemeManager::ColorRole::TextSecondary));
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
    mColorPresetsLabel.setFont(Font(18.0f, Font::bold));
    mColorPresetsLabel.setColour(Label::textColourId, theme.get(ThemeManager::ColorRole::TextPrimary));
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
    mAppearanceLabel.setFont(Font(18.0f, Font::bold));
    mAppearanceLabel.setColour(Label::textColourId, theme.get(ThemeManager::ColorRole::TextPrimary));
    addAndMakeVisible(mAppearanceLabel);

    mTileSizeLabel.setText("Min Tile Size (px):", dontSendNotification);
    mTileSizeLabel.setColour(Label::textColourId, theme.get(ThemeManager::ColorRole::TextSecondary));
    addAndMakeVisible(mTileSizeLabel);

    mSampleMinSizeValue.setName("Tile Size");
    mSampleMinSizeValue.setInputRestrictions(3, "0123456789");
    mSampleMinSizeValue.setText(String(AppValues::getInstance().SAMPLE_TILE_MIN_WIDTH));
    mSampleMinSizeValue.addListener(this);
    addAndMakeVisible(mSampleMinSizeValue);

    mThumbnailLinesLabel.setText("Tile Waveform Lines:", dontSendNotification);
    mThumbnailLinesLabel.setColour(Label::textColourId, theme.get(ThemeManager::ColorRole::TextSecondary));
    addAndMakeVisible(mThumbnailLinesLabel);

    mThumbnailLineCount.setName("Waveform Lines");
    mThumbnailLineCount.setInputRestrictions(3, "0123456789");
    mThumbnailLineCount.setText(String(AppValues::getInstance().AUDIO_THUMBNAIL_LINE_COUNT));
    mThumbnailLineCount.addListener(this);
    addAndMakeVisible(mThumbnailLineCount);


    mThumbnailLinesPlayerLabel.setText("Player Waveform Lines:", dontSendNotification);
    mThumbnailLinesPlayerLabel.setColour(Label::textColourId, theme.get(ThemeManager::ColorRole::TextSecondary));
    addAndMakeVisible(mThumbnailLinesPlayerLabel);

    mThumbnailLineCountPlayer.setName("Player Waveform Lines");
    mThumbnailLineCountPlayer.setInputRestrictions(3, "0123456789");
    mThumbnailLineCountPlayer.setText(String(AppValues::getInstance().AUDIO_THUMBNAIL_LINE_COUNT_PLAYER));
    mThumbnailLineCountPlayer.addListener(this);
    addAndMakeVisible(mThumbnailLineCountPlayer);
    // ===== CLOSE BUTTON =====
    mCloseButton.setName("Close");
    mCloseButton.setButtonText("Close");
    mCloseButton.addListener(this);
    addAndMakeVisible(mCloseButton);


    updateColorButtons();
}

void PreferenceWindow::View::buttonClicked(Button* button)
{
    auto& theme = ThemeManager::getInstance();
    String buttonName = button->getName();

    // Color picker buttons
    if (buttonName == "Primary Color")
    {
        mColorEditMode = ColorEditMode::Primary;
        mColourSelector = std::make_unique<ColourSelector>();
        mColourSelector->setSize(300, 300);
        mColourSelector->setCurrentColour(theme.get(ThemeManager::ColorRole::AccentPrimary));
        mColourSelector->addChangeListener(this);
        CallOutBox::launchAsynchronously(std::move(mColourSelector), button->getScreenBounds(), nullptr);
    }
    else if (buttonName == "Accent Color")
    {
        mColorEditMode = ColorEditMode::Accent;
        mColourSelector = std::make_unique<ColourSelector>();
        mColourSelector->setSize(300, 300);
        mColourSelector->setCurrentColour(theme.get(ThemeManager::ColorRole::AccentSecondary));
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
    else if (buttonName == "Close")
    {
        // Find parent PreferenceWindow and close it
        if (auto* parentWindow = findParentComponentOfClass<PreferenceWindow>())
        {
            parentWindow->exitModalState(0);
        }
    }
}

void PreferenceWindow::View::changeListenerCallback(ChangeBroadcaster* source)
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
        else // Accent
        {
            theme.setCustomColor(ThemeManager::ColorRole::AccentSecondary, newColour);
            theme.setCustomColor(ThemeManager::ColorRole::WaveformSecondary, newColour);
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

void PreferenceWindow::View::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
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
}

void PreferenceWindow::View::textEditorTextChanged(TextEditor& editor)
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
void PreferenceWindow::View::updateColorButtons()
{
    auto& theme = ThemeManager::getInstance();

    Colour primaryColor = theme.get(ThemeManager::ColorRole::AccentPrimary);
    mPrimaryColorButton.setColour(TextButton::buttonColourId, primaryColor);
    mPrimaryColorButton.setColour(TextButton::textColourOffId,
        primaryColor.getPerceivedBrightness() > 0.5f ? Colours::black : Colours::white);

    Colour accentColor = theme.get(ThemeManager::ColorRole::AccentSecondary);
    mAccentColorButton.setColour(TextButton::buttonColourId, accentColor);
    mAccentColorButton.setColour(TextButton::textColourOffId,
        accentColor.getPerceivedBrightness() > 0.5f ? Colours::black : Colours::white);
}

void PreferenceWindow::View::applyColorPreset(const String& presetName)
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
        theme.get(ThemeManager::ColorRole::AccentPrimary));
    theme.setCustomColor(ThemeManager::ColorRole::WaveformSecondary,
        theme.get(ThemeManager::ColorRole::AccentSecondary));

    SamplifyMainComponent::setupLookAndFeel(getLookAndFeel());
    updateColorButtons();

    if (auto* mainComp = SamplifyMainComponent::getInstance())
    {
        mainComp->getSamplePlayerComponent().updateThemeColors();
        mainComp->repaint();
    }
    repaint();
}

void PreferenceWindow::View::paint(Graphics& g)
{
    auto& theme = ThemeManager::getInstance();
    g.fillAll(theme.get(ThemeManager::ColorRole::Background));

    // Draw section separators
    g.setColour(theme.get(ThemeManager::ColorRole::Border));
    g.drawLine(16, 80, getWidth() - 16, 80, 1.0f);      // After theme section
    g.drawLine(16, 280, getWidth() - 16, 280, 1.0f);   // After color customization
    g.drawLine(16, 430, getWidth() - 16, 430, 1.0f);   // After color presets
}

void PreferenceWindow::View::resized()
{
    const int margin = 16;
    const int labelHeight = 24;
    const int controlHeight = 36;
    const int sectionSpacing = 24;
    const int itemSpacing = 8;

    int y = margin;

    // ===== THEME SECTION =====
    mThemeLabel.setBounds(margin, y, getWidth() - 2 * margin, labelHeight);
    y += labelHeight + itemSpacing;

    mThemeSelector.setBounds(margin, y, getWidth() - 2 * margin, controlHeight);
    y += controlHeight + sectionSpacing;

    // Separator line drawn in paint()
    y += itemSpacing;

    // ===== COLOR CUSTOMIZATION SECTION =====
    mColorCustomizationLabel.setBounds(margin, y, getWidth() - 2 * margin, labelHeight);
    y += labelHeight + itemSpacing;

    // Primary color row
    mPrimaryColorLabel.setBounds(margin, y, 120, controlHeight);
    mPrimaryColorButton.setBounds(margin + 130, y, getWidth() - margin - 130 - margin, controlHeight);
    y += controlHeight + itemSpacing;

    // Accent color row
    mAccentColorLabel.setBounds(margin, y, 120, controlHeight);
    mAccentColorButton.setBounds(margin + 130, y, getWidth() - margin - 130 - margin, controlHeight);
    y += controlHeight + itemSpacing;

    // Reset button
    mResetColorsButton.setBounds(margin, y, getWidth() - 2 * margin, controlHeight);
    y += controlHeight + sectionSpacing;

    // Separator line drawn in paint()
    y += itemSpacing;

    // ===== COLOR PRESETS SECTION =====
    mColorPresetsLabel.setBounds(margin, y, getWidth() - 2 * margin, labelHeight);
    y += labelHeight + itemSpacing;

    // Preset buttons in a grid (2 columns)
    int presetWidth = (getWidth() - 3 * margin) / 2;
    mPresetStudioDark.setBounds(margin, y, presetWidth, controlHeight);
    mPresetStudioLight.setBounds(margin + presetWidth + margin, y, presetWidth, controlHeight);
    y += controlHeight + itemSpacing;

    mPresetAbleton.setBounds(margin, y, presetWidth, controlHeight);
    mPresetProTools.setBounds(margin + presetWidth + margin, y, presetWidth, controlHeight);
    y += controlHeight + itemSpacing;

    mPresetHighContrast.setBounds(margin, y, getWidth() - 2 * margin, controlHeight);
    y += controlHeight + sectionSpacing;

    // Separator line drawn in paint()
    y += itemSpacing;

    // ===== APPEARANCE SECTION =====
    mAppearanceLabel.setBounds(margin, y, getWidth() - 2 * margin, labelHeight);
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
    y += controlHeight + margin;

    // ===== CLOSE BUTTON =====
    y += sectionSpacing;
    mCloseButton.setBounds(getWidth() - margin - 120, y, 120, controlHeight);
}
