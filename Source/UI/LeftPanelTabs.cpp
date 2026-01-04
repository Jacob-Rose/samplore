/*
  ==============================================================================

    LeftPanelTabs.cpp
    Created: 2025
    Author:  Samplore Team

  ==============================================================================
*/

#include "LeftPanelTabs.h"

namespace samplore
{
    //==============================================================================
    // CuePanel::CueRowComponent
    //==============================================================================

    CuePanel::CueRowComponent::CueRowComponent(const juce::KeyPress& key, const CueBinding& binding)
        : mKey(key), mBinding(binding)
    {
        mPlayButton.addListener(this);
        addAndMakeVisible(mPlayButton);

        mDeleteButton.addListener(this);
        addAndMakeVisible(mDeleteButton);
    }

    void CuePanel::CueRowComponent::paint(Graphics& g)
    {
        auto& theme = ThemeManager::getInstance();
        auto bounds = getLocalBounds().reduced(2);

        // Background
        g.setColour(theme.getColorForRole(ThemeManager::ColorRole::Surface));
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);

        // Key badge - use cue's color
        auto keyRect = bounds.removeFromLeft(36).reduced(4);
        Colour cueColor = mBinding.getColor();
        g.setColour(cueColor);
        g.fillRoundedRectangle(keyRect.toFloat(), 4.0f);

        // Text color - use contrasting color (dark text on light, light text on dark)
        g.setColour(cueColor.getBrightness() > 0.5f ? Colours::black : Colours::white);
        g.setFont(FontOptions(14.0f).withStyle("Bold"));
        g.drawText(CueManager::getKeyDisplayString(mKey), keyRect, Justification::centred);

        // Color bar indicator on left edge
        auto colorBarRect = bounds.removeFromLeft(4).toFloat();
        g.setColour(cueColor.withAlpha(0.8f));
        g.fillRect(colorBarRect);

        // Sample name
        bounds.removeFromLeft(6);
        auto nameRect = bounds.withTrimmedRight(70);
        g.setColour(theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
        g.setFont(FontOptions(13.0f));

        juce::String name = mBinding.mDisplayName.isNotEmpty()
            ? mBinding.mDisplayName
            : (mBinding.mSample.isNull() ? "(invalid)" : mBinding.mSample.getFile().getFileNameWithoutExtension());
        g.drawText(name, nameRect, Justification::centredLeft, true);
    }

    void CuePanel::CueRowComponent::resized()
    {
        auto bounds = getLocalBounds().reduced(2);
        mDeleteButton.setBounds(bounds.removeFromRight(28).reduced(4));
        mPlayButton.setBounds(bounds.removeFromRight(28).reduced(4));
    }

    void CuePanel::CueRowComponent::buttonClicked(Button* button)
    {
        if (button == &mPlayButton)
        {
            CueManager::getInstance().triggerBinding(mBinding);
        }
        else if (button == &mDeleteButton)
        {
            CueManager::getInstance().removeBinding(mKey);
        }
    }

    //==============================================================================
    // CuePanel
    //==============================================================================

    CuePanel::CuePanel()
    {
        // Rack selector dropdown
        mRackSelector.addListener(this);
        addAndMakeVisible(mRackSelector);

        mNewRackButton.addListener(this);
        mNewRackButton.setTooltip("Create new cue rack");
        addAndMakeVisible(mNewRackButton);

        mDeleteRackButton.addListener(this);
        mDeleteRackButton.setTooltip("Delete current rack");
        addAndMakeVisible(mDeleteRackButton);

        mCueModeLabel.setText("Cue Mode:", dontSendNotification);
        mCueModeLabel.setJustificationType(Justification::centredRight);
        addAndMakeVisible(mCueModeLabel);

        mCueModeToggle.setToggleState(CueManager::getInstance().isCueModeEnabled(), dontSendNotification);
        mCueModeToggle.addListener(this);
        addAndMakeVisible(mCueModeToggle);

        mBindingsViewport.setViewedComponent(&mBindingsContainer, false);
        mBindingsViewport.setScrollBarsShown(true, false);
        addAndMakeVisible(mBindingsViewport);

        mClearAllButton.addListener(this);
        addAndMakeVisible(mClearAllButton);

        CueManager::getInstance().addChangeListener(this);
        ThemeManager::getInstance().addListener(this);

        // Set initial enabled state based on cue mode
        bool cueModeEnabled = CueManager::getInstance().isCueModeEnabled();
        mRackSelector.setEnabled(!cueModeEnabled);
        mNewRackButton.setEnabled(!cueModeEnabled);
        mDeleteRackButton.setEnabled(!cueModeEnabled);

        refreshRackList();
        refreshBindings();
    }

    CuePanel::~CuePanel()
    {
        CueManager::getInstance().removeChangeListener(this);
        ThemeManager::getInstance().removeListener(this);
    }

    void CuePanel::paint(Graphics& g)
    {
        g.fillAll(ThemeManager::getInstance().getColorForRole(ThemeManager::ColorRole::Background));
    }

    void CuePanel::resized()
    {
        auto bounds = getLocalBounds().reduced(8);

        // Top row: rack selector
        auto rackRow = bounds.removeFromTop(28);
        mDeleteRackButton.setBounds(rackRow.removeFromRight(28));
        rackRow.removeFromRight(4);
        mNewRackButton.setBounds(rackRow.removeFromRight(28));
        rackRow.removeFromRight(4);
        mRackSelector.setBounds(rackRow);

        bounds.removeFromTop(6);

        // Second row: cue mode toggle
        auto modeRow = bounds.removeFromTop(24);
        mCueModeToggle.setBounds(modeRow.removeFromRight(24));
        mCueModeLabel.setBounds(modeRow);

        bounds.removeFromTop(6);

        // Bottom: clear button
        mClearAllButton.setBounds(bounds.removeFromBottom(28));
        bounds.removeFromBottom(8);

        // Middle: bindings list
        mBindingsViewport.setBounds(bounds);

        // Layout rows
        int rowHeight = 36;
        int spacing = 4;
        int totalHeight = static_cast<int>(mRows.size()) * (rowHeight + spacing);
        mBindingsContainer.setSize(bounds.getWidth() - 12, jmax(totalHeight, bounds.getHeight()));

        int y = 0;
        for (auto& row : mRows)
        {
            row->setBounds(0, y, mBindingsContainer.getWidth(), rowHeight);
            y += rowHeight + spacing;
        }
    }

    void CuePanel::changeListenerCallback(ChangeBroadcaster*)
    {
        bool cueModeEnabled = CueManager::getInstance().isCueModeEnabled();
        mCueModeToggle.setToggleState(cueModeEnabled, dontSendNotification);

        // Disable rack switching while cue mode is active to prevent binding issues
        mRackSelector.setEnabled(!cueModeEnabled);
        mNewRackButton.setEnabled(!cueModeEnabled);
        mDeleteRackButton.setEnabled(!cueModeEnabled);

        refreshRackList();
        refreshBindings();
    }

    void CuePanel::buttonClicked(Button* button)
    {
        if (button == &mCueModeToggle)
        {
            CueManager::getInstance().setCueModeEnabled(mCueModeToggle.getToggleState());
        }
        else if (button == &mNewRackButton)
        {
            showNewRackDialog();
        }
        else if (button == &mDeleteRackButton)
        {
            showDeleteRackConfirmation();
        }
        else if (button == &mClearAllButton)
        {
            auto options = MessageBoxOptions()
                .withIconType(MessageBoxIconType::WarningIcon)
                .withTitle("Clear All Cues?")
                .withMessage("Remove all cue bindings from this rack?")
                .withButton("Yes")
                .withButton("No");

            NativeMessageBox::showAsync(options, [](int result) {
                if (result == 1)
                    CueManager::getInstance().clearAllBindings();
            });
        }
    }

    void CuePanel::comboBoxChanged(ComboBox* comboBox)
    {
        if (comboBox == &mRackSelector)
        {
            juce::String selectedName = mRackSelector.getText();
            auto& cueManager = CueManager::getInstance();

            // Only load if selecting a different rack
            if (selectedName.isNotEmpty() && selectedName != cueManager.getCurrentRackName())
            {
                // Save current rack before switching
                cueManager.saveCurrentRack();
                cueManager.loadRack(selectedName);
            }
        }
    }

    void CuePanel::themeChanged(ThemeManager::Theme) { repaint(); }
    void CuePanel::colorChanged(ThemeManager::ColorRole, Colour) { repaint(); }

    void CuePanel::refreshBindings()
    {
        mRows.clear();
        mBindingsContainer.removeAllChildren();

        for (const auto& [key, binding] : CueManager::getInstance().getBindings())
        {
            auto row = std::make_unique<CueRowComponent>(key, binding);
            mBindingsContainer.addAndMakeVisible(row.get());
            mRows.push_back(std::move(row));
        }

        resized();
        repaint();
    }

    void CuePanel::refreshRackList()
    {
        auto& cueManager = CueManager::getInstance();
        juce::StringArray rackNames = cueManager.getRackNames();
        juce::String currentRack = cueManager.getCurrentRackName();

        mRackSelector.clear(dontSendNotification);

        int selectedIndex = -1;
        for (int i = 0; i < rackNames.size(); ++i)
        {
            mRackSelector.addItem(rackNames[i], i + 1);
            if (rackNames[i] == currentRack)
                selectedIndex = i;
        }

        if (selectedIndex >= 0)
            mRackSelector.setSelectedItemIndex(selectedIndex, dontSendNotification);
    }

    void CuePanel::showNewRackDialog()
    {
        auto* alertWindow = new AlertWindow("New Cue Rack", "Enter a name for the new cue rack:", MessageBoxIconType::QuestionIcon);
        alertWindow->addTextEditor("rackName", "", "Rack Name:");
        alertWindow->addButton("Create", 1, KeyPress(KeyPress::returnKey));
        alertWindow->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

        alertWindow->enterModalState(true, ModalCallbackFunction::create([alertWindow](int result)
        {
            if (result == 1)
            {
                juce::String newName = alertWindow->getTextEditorContents("rackName").trim();
                if (newName.isNotEmpty())
                {
                    if (!CueManager::getInstance().createNewRack(newName))
                    {
                        // Rack already exists
                        NativeMessageBox::showMessageBoxAsync(MessageBoxIconType::WarningIcon,
                            "Cannot Create Rack",
                            "A rack with that name already exists.");
                    }
                }
            }
            delete alertWindow;
        }), true);
    }

    void CuePanel::showDeleteRackConfirmation()
    {
        auto& cueManager = CueManager::getInstance();
        juce::String currentRack = cueManager.getCurrentRackName();

        if (currentRack.isEmpty())
            return;

        // Don't allow deleting if it's the only rack
        if (cueManager.getRackNames().size() <= 1)
        {
            NativeMessageBox::showMessageBoxAsync(MessageBoxIconType::WarningIcon,
                "Cannot Delete Rack",
                "You cannot delete the last remaining cue rack.");
            return;
        }

        auto options = MessageBoxOptions()
            .withIconType(MessageBoxIconType::WarningIcon)
            .withTitle("Delete Cue Rack?")
            .withMessage("Delete the rack \"" + currentRack + "\" and all its cue bindings?")
            .withButton("Delete")
            .withButton("Cancel");

        // Capture currentRack by value
        juce::String rackToDelete = currentRack;
        NativeMessageBox::showAsync(options, [rackToDelete](int result) {
            if (result == 1)
                CueManager::getInstance().deleteRack(rackToDelete);
        });
    }

    //==============================================================================
    // LeftPanelTabs
    //==============================================================================

    LeftPanelTabs::LeftPanelTabs()
    {
        auto& theme = ThemeManager::getInstance();

        mTabs.setTabBarDepth(28);
        mTabs.setOutline(0);

        // Add tabs - using addTab with colour
        Colour tabBg = theme.getColorForRole(ThemeManager::ColorRole::Background);
        mTabs.addTab("Folders", tabBg, &mDirectoryExplorer, false);
        mTabs.addTab("Cues", tabBg, &mCuePanel, false);

        addAndMakeVisible(mTabs);

        ThemeManager::getInstance().addListener(this);
    }

    LeftPanelTabs::~LeftPanelTabs()
    {
        ThemeManager::getInstance().removeListener(this);
    }

    void LeftPanelTabs::paint(Graphics& g)
    {
        g.fillAll(ThemeManager::getInstance().getColorForRole(ThemeManager::ColorRole::Background));
    }

    void LeftPanelTabs::resized()
    {
        mTabs.setBounds(getLocalBounds());
    }

    void LeftPanelTabs::themeChanged(ThemeManager::Theme)
    {
        auto& theme = ThemeManager::getInstance();
        Colour tabBg = theme.getColorForRole(ThemeManager::ColorRole::Background);

        for (int i = 0; i < mTabs.getNumTabs(); ++i)
            mTabs.setTabBackgroundColour(i, tabBg);

        repaint();
    }

    void LeftPanelTabs::colorChanged(ThemeManager::ColorRole role, Colour newColor)
    {
        if (role == ThemeManager::ColorRole::Background)
        {
            for (int i = 0; i < mTabs.getNumTabs(); ++i)
                mTabs.setTabBackgroundColour(i, newColor);
        }
        repaint();
    }
}
