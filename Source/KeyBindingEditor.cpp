/*
  ==============================================================================

    KeyBindingEditor.cpp
    Author:  Jake Rose

    UI component for editing key bindings

  ==============================================================================
*/

#include "KeyBindingEditor.h"
#include "SamplifyLookAndFeel.h"

using namespace samplore;

KeyBindingEditor::KeyBindingEditor()
{
    ThemeManager::getInstance().addListener(this);
    
    // Setup viewport
    addAndMakeVisible(mViewport);
    mViewport.setViewedComponent(&mContentComponent, false);
    mViewport.setScrollBarsShown(true, false);
    
    // Setup buttons
    addAndMakeVisible(mResetButton);
    mResetButton.setButtonText("Reset to Defaults");
    mResetButton.addListener(this);
    
    addAndMakeVisible(mCloseButton);
    mCloseButton.setButtonText("Close");
    mCloseButton.addListener(this);
    
    // Setup key capture label (initially hidden)
    addAndMakeVisible(mCaptureLabel);
    mCaptureLabel.setText("Press a key...", dontSendNotification);
    mCaptureLabel.setJustificationType(Justification::centred);
    mCaptureLabel.setVisible(false);
    
    rebuildKeyBindingList();
    updateAllComponentColors();
}

KeyBindingEditor::~KeyBindingEditor()
{
    ThemeManager::getInstance().removeListener(this);
}

void KeyBindingEditor::paint(Graphics& g)
{
    auto& theme = ThemeManager::getInstance();
    g.fillAll(theme.getColorForRole(ThemeManager::ColorRole::Background));
}

void KeyBindingEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Bottom button area
    auto buttonArea = bounds.removeFromBottom(40);
    buttonArea.removeFromRight(10);
    mCloseButton.setBounds(buttonArea.removeFromRight(80));
    buttonArea.removeFromRight(10);
    mResetButton.setBounds(buttonArea.removeFromRight(120));
    
    // Key capture overlay (covers entire area when active)
    if (mIsCapturingKey)
    {
        mCaptureLabel.setBounds(bounds);
    }
    else
    {
        // Viewport takes remaining space
        bounds.removeFromRight(10);
        bounds.removeFromLeft(10);
        bounds.removeFromTop(10);
        mViewport.setBounds(bounds);
        
        // Content component width
        mContentComponent.setSize(bounds.getWidth() - mViewport.getScrollBarThickness(), 
                                  mContentComponent.getHeight());
    }
}

void KeyBindingEditor::themeChanged(ThemeManager::Theme newTheme)
{
    updateAllComponentColors();
    repaint();
}

void KeyBindingEditor::colorChanged(ThemeManager::ColorRole role, Colour newColor)
{
    updateAllComponentColors();
    repaint();
}

void KeyBindingEditor::buttonClicked(Button* button)
{
    if (button == &mResetButton)
    {
        KeyBindingManager::getInstance().resetAllKeys();
        rebuildKeyBindingList();
    }
    else if (button == &mCloseButton)
    {
        // Close the dialog - this would be handled by parent
        if (auto* parent = getParentComponent())
        {
            parent->exitModalState(0);
        }
    }
    
    // Handle rebind buttons
    for (auto& row : mKeyRows)
    {
        if (button == row.rebindButton)
        {
            startKeyCapture(row.action);
            break;
        }
    }
}

void KeyBindingEditor::rebuildKeyBindingList()
{
    // Clear existing rows
    mKeyRows.clear();
    mContentComponent.removeAllChildren();
    
    auto& keyManager = KeyBindingManager::getInstance();
    int y = 10;
    
    // Create row for each action
    for (int i = 0; i <= static_cast<int>(KeyBindingManager::Action::ToggleCueBindings); ++i)
    {
        auto action = static_cast<KeyBindingManager::Action>(i);
        
        KeyBindingRow row;
        row.action = action;
        
        // Action label
        row.actionLabel = new Label();
        row.actionLabel->setText(keyManager.getActionName(action), dontSendNotification);
        row.actionLabel->setBounds(10, y, 200, 25);
        mContentComponent.addAndMakeVisible(row.actionLabel);
        
        // Key label
        row.keyLabel = new Label();
        row.keyLabel->setText(keyManager.getKeyString(action), dontSendNotification);
        row.keyLabel->setBounds(220, y, 100, 25);
        mContentComponent.addAndMakeVisible(row.keyLabel);
        
        // Rebind button
        row.rebindButton = new TextButton();
        row.rebindButton->setButtonText("Rebind");
        row.rebindButton->setBounds(330, y, 80, 25);
        row.rebindButton->addListener(this);
        mContentComponent.addAndMakeVisible(row.rebindButton);
        
        mKeyRows.add(row);
        y += 35;
    }
    
    mContentComponent.setSize(420, y);
    updateAllComponentColors();
}

void KeyBindingEditor::startKeyCapture(KeyBindingManager::Action action)
{
    mIsCapturingKey = true;
    mCapturingAction = action;
    mCaptureLabel.setVisible(true);
    mViewport.setVisible(false);
    repaint();
    
    // Request keyboard focus
    grabKeyboardFocus();
}

void KeyBindingEditor::updateAllComponentColors()
{
    auto& theme = ThemeManager::getInstance();
    auto background = theme.getColorForRole(ThemeManager::ColorRole::Background);
    auto foreground = theme.getColorForRole(ThemeManager::ColorRole::TextPrimary);
    auto accent = theme.getColorForRole(ThemeManager::ColorRole::AccentSecondary);
    
    // Update labels
    for (auto& row : mKeyRows)
    {
        if (row.actionLabel)
        {
            row.actionLabel->setColour(Label::textColourId, foreground);
            row.actionLabel->setColour(Label::backgroundColourId, background);
        }
        if (row.keyLabel)
        {
            row.keyLabel->setColour(Label::textColourId, foreground);
            row.keyLabel->setColour(Label::backgroundColourId, background);
        }
        if (row.rebindButton)
        {
            row.rebindButton->setColour(TextButton::buttonColourId, accent);
            row.rebindButton->setColour(TextButton::textColourOnId, foreground);
        }
    }
    
    // Update main buttons
    mResetButton.setColour(TextButton::buttonColourId, accent);
    mResetButton.setColour(TextButton::textColourOnId, foreground);
    mCloseButton.setColour(TextButton::buttonColourId, accent);
    mCloseButton.setColour(TextButton::textColourOnId, foreground);
    
    // Update capture label
    mCaptureLabel.setColour(Label::textColourId, foreground);
    mCaptureLabel.setColour(Label::backgroundColourId, background);
}

bool KeyBindingEditor::keyPressed(const KeyPress& key, Component* originatingComponent)
{
    if (mIsCapturingKey)
    {
        // Set the new key binding
        KeyBindingManager::getInstance().setKey(mCapturingAction, key);

        // Exit capture mode
        mIsCapturingKey = false;
        mCaptureLabel.setVisible(false);
        mViewport.setVisible(true);

        // Rebuild the list to show updated key
        rebuildKeyBindingList();

        return true;
    }

    return false;
}