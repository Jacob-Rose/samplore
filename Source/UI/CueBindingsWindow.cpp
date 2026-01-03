/*
  ==============================================================================

    CueBindingsWindow.cpp
    Created: 2025
    Author:  Samplore Team

  ==============================================================================
*/

#include "CueBindingsWindow.h"
#include "../SamplifyProperties.h"

namespace samplore
{
    //==============================================================================
    // CueBindingRowComponent
    //==============================================================================

    CueBindingRowComponent::CueBindingRowComponent(const juce::KeyPress& key, const CueBinding& binding)
        : mKey(key), mBinding(binding)
    {
        mPlayButton.setButtonText("Play");
        mPlayButton.addListener(this);
        addAndMakeVisible(mPlayButton);

        mDeleteButton.setButtonText("X");
        mDeleteButton.addListener(this);
        addAndMakeVisible(mDeleteButton);
    }

    void CueBindingRowComponent::paint(Graphics& g)
    {
        auto& theme = ThemeManager::getInstance();
        auto bounds = getLocalBounds();

        // Background
        g.setColour(theme.getColorForRole(ThemeManager::ColorRole::Surface));
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);

        // Key indicator
        g.setColour(theme.getColorForRole(ThemeManager::ColorRole::AccentPrimary));
        auto keyRect = bounds.removeFromLeft(50).reduced(4);
        g.fillRoundedRectangle(keyRect.toFloat(), 4.0f);

        g.setColour(theme.getColorForRole(ThemeManager::ColorRole::TextPrimary));
        g.setFont(Font(16.0f, Font::bold));
        g.drawText("[" + CueManager::getKeyDisplayString(mKey) + "]",
                   keyRect, Justification::centred);

        // Sample name
        bounds.removeFromLeft(8);
        auto nameRect = bounds.removeFromLeft(200);
        g.setFont(Font(14.0f));
        juce::String sampleName = mBinding.mDisplayName.isNotEmpty()
            ? mBinding.mDisplayName
            : (mBinding.mSample.isNull() ? "(invalid)" : mBinding.mSample.getFile().getFileNameWithoutExtension());
        g.drawText(sampleName, nameRect, Justification::centredLeft, true);

        // Start time if non-zero
        if (mBinding.mStartTime > 0.001)
        {
            bounds.removeFromLeft(8);
            auto timeRect = bounds.removeFromLeft(80);
            g.setColour(theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
            g.setFont(Font(12.0f));
            int percentage = static_cast<int>(mBinding.mStartTime * 100);
            g.drawText("@ " + juce::String(percentage) + "%", timeRect, Justification::centredLeft);
        }
    }

    void CueBindingRowComponent::resized()
    {
        auto bounds = getLocalBounds();

        // Buttons on the right
        mDeleteButton.setBounds(bounds.removeFromRight(30).reduced(4));
        bounds.removeFromRight(4);
        mPlayButton.setBounds(bounds.removeFromRight(50).reduced(4));
    }

    void CueBindingRowComponent::buttonClicked(Button* button)
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
    // CueBindingsListComponent
    //==============================================================================

    CueBindingsListComponent::CueBindingsListComponent()
    {
        mCueModeLabel.setText("Cue Mode (keyboard priority):", dontSendNotification);
        mCueModeLabel.setJustificationType(Justification::centredLeft);
        addAndMakeVisible(mCueModeLabel);

        mCueModeToggle.setToggleState(CueManager::getInstance().isCueModeEnabled(), dontSendNotification);
        mCueModeToggle.addListener(this);
        addAndMakeVisible(mCueModeToggle);

        mHeaderLabel.setText("Cue Bindings", dontSendNotification);
        mHeaderLabel.setFont(Font(18.0f, Font::bold));
        mHeaderLabel.setJustificationType(Justification::centredLeft);
        addAndMakeVisible(mHeaderLabel);

        mBindingsViewport.setViewedComponent(&mBindingsContainer, false);
        mBindingsViewport.setScrollBarsShown(true, false);
        addAndMakeVisible(mBindingsViewport);

        mClearAllButton.setButtonText("Clear All Bindings");
        mClearAllButton.addListener(this);
        addAndMakeVisible(mClearAllButton);

        CueManager::getInstance().addChangeListener(this);
        ThemeManager::getInstance().addListener(this);

        refreshBindingsList();
    }

    CueBindingsListComponent::~CueBindingsListComponent()
    {
        CueManager::getInstance().removeChangeListener(this);
        ThemeManager::getInstance().removeListener(this);
    }

    void CueBindingsListComponent::paint(Graphics& g)
    {
        auto& theme = ThemeManager::getInstance();
        g.fillAll(theme.getColorForRole(ThemeManager::ColorRole::Background));
    }

    void CueBindingsListComponent::resized()
    {
        auto bounds = getLocalBounds().reduced(10);

        // Cue mode toggle at top
        auto toggleRow = bounds.removeFromTop(30);
        mCueModeLabel.setBounds(toggleRow.removeFromLeft(200));
        mCueModeToggle.setBounds(toggleRow.removeFromLeft(30));

        bounds.removeFromTop(10);

        // Header
        mHeaderLabel.setBounds(bounds.removeFromTop(30));

        bounds.removeFromTop(5);

        // Clear button at bottom
        mClearAllButton.setBounds(bounds.removeFromBottom(30));
        bounds.removeFromBottom(10);

        // Bindings list viewport fills remaining space
        mBindingsViewport.setBounds(bounds);

        // Layout binding rows in container
        int rowHeight = 40;
        int spacing = 5;
        int totalHeight = static_cast<int>(mRowComponents.size()) * (rowHeight + spacing);
        mBindingsContainer.setSize(bounds.getWidth() - 20, juce::jmax(totalHeight, bounds.getHeight()));

        int y = 0;
        for (auto& row : mRowComponents)
        {
            row->setBounds(0, y, mBindingsContainer.getWidth(), rowHeight);
            y += rowHeight + spacing;
        }
    }

    void CueBindingsListComponent::changeListenerCallback(ChangeBroadcaster* source)
    {
        // Refresh when cue manager changes
        mCueModeToggle.setToggleState(CueManager::getInstance().isCueModeEnabled(), dontSendNotification);
        refreshBindingsList();
    }

    void CueBindingsListComponent::buttonClicked(Button* button)
    {
        if (button == &mCueModeToggle)
        {
            CueManager::getInstance().setCueModeEnabled(mCueModeToggle.getToggleState());
        }
        else if (button == &mClearAllButton)
        {
            auto options = MessageBoxOptions()
                .withIconType(MessageBoxIconType::WarningIcon)
                .withTitle("Clear All Cue Bindings?")
                .withMessage("This will remove all keyboard cue bindings. Are you sure?")
                .withButton("Yes")
                .withButton("No");

            NativeMessageBox::showAsync(options, [](int result)
            {
                if (result == 1)
                {
                    CueManager::getInstance().clearAllBindings();
                }
            });
        }
    }

    void CueBindingsListComponent::themeChanged(ThemeManager::Theme newTheme)
    {
        repaint();
    }

    void CueBindingsListComponent::colorChanged(ThemeManager::ColorRole role, Colour newColor)
    {
        repaint();
    }

    void CueBindingsListComponent::refreshBindingsList()
    {
        mRowComponents.clear();
        mBindingsContainer.removeAllChildren();

        const auto& bindings = CueManager::getInstance().getBindings();
        for (const auto& [key, binding] : bindings)
        {
            auto row = std::make_unique<CueBindingRowComponent>(key, binding);
            mBindingsContainer.addAndMakeVisible(row.get());
            mRowComponents.push_back(std::move(row));
        }

        resized();
        repaint();
    }

    //==============================================================================
    // CueBindingsWindow
    //==============================================================================

    CueBindingsWindow::CueBindingsWindow()
        : DocumentWindow("Cue Bindings",
                         ThemeManager::getInstance().getColorForRole(ThemeManager::ColorRole::Background),
                         DocumentWindow::closeButton)
    {
        mContent = std::make_unique<CueBindingsListComponent>();
        setContentOwned(mContent.release(), false);

        setSize(450, 400);
        setResizable(true, true);
        setUsingNativeTitleBar(true);

        centreWithSize(getWidth(), getHeight());

        CueManager::getInstance().addChangeListener(this);
        ThemeManager::getInstance().addListener(this);
    }

    CueBindingsWindow::~CueBindingsWindow()
    {
        CueManager::getInstance().removeChangeListener(this);
        ThemeManager::getInstance().removeListener(this);
    }

    void CueBindingsWindow::closeButtonPressed()
    {
        setVisible(false);
    }

    void CueBindingsWindow::changeListenerCallback(ChangeBroadcaster* source)
    {
        // Update title bar color when theme changes
        setBackgroundColour(ThemeManager::getInstance().getColorForRole(ThemeManager::ColorRole::Background));
    }

    void CueBindingsWindow::themeChanged(ThemeManager::Theme newTheme)
    {
        setBackgroundColour(ThemeManager::getInstance().getColorForRole(ThemeManager::ColorRole::Background));
        repaint();
    }

    void CueBindingsWindow::colorChanged(ThemeManager::ColorRole role, Colour newColor)
    {
        if (role == ThemeManager::ColorRole::Background)
        {
            setBackgroundColour(newColor);
        }
        repaint();
    }
}
