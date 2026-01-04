/*
  ==============================================================================

    KeyCaptureOverlay.cpp
    Created: 2025
    Author:  Samplore Team

  ==============================================================================
*/

#include "KeyCaptureOverlay.h"
#include "../CueManager.h"
#include "../SamplifyProperties.h"
#include "OverlayPanel.h"

namespace samplore
{
    KeyCaptureOverlay::KeyCaptureOverlay()
    {
        setWantsKeyboardFocus(true);
        addKeyListener(this);

        // Set initial size so OverlayPanel has something to work with
        setSize(400, 200);

        mInstructionLabel.setJustificationType(Justification::centred);
        mInstructionLabel.setFont(FontOptions(16.0f));
        addAndMakeVisible(mInstructionLabel);

        mKeyDisplayLabel.setJustificationType(Justification::centred);
        mKeyDisplayLabel.setFont(FontOptions(48.0f).withStyle("Bold"));
        addAndMakeVisible(mKeyDisplayLabel);

        mCancelButton.onClick = [this]() {
            setState(State::Cancelled);
            if (mParentOverlay)
                mParentOverlay->hide();
        };
        addAndMakeVisible(mCancelButton);

        ThemeManager::getInstance().addListener(this);
        reset();
    }

    KeyCaptureOverlay::~KeyCaptureOverlay()
    {
        ThemeManager::getInstance().removeListener(this);
        removeKeyListener(this);
    }

    void KeyCaptureOverlay::setSample(Sample::Reference sample, double startTime)
    {
        mSample = sample;
        mStartTime = startTime;
    }

    void KeyCaptureOverlay::reset()
    {
        mState = State::WaitingForFirstKey;
        mCapturedKey = KeyPress();
        mSample = nullptr;
        mStartTime = 0.0;
        mKeyDisplayLabel.setText("", dontSendNotification);
        mInstructionLabel.setText("Press a key (0-9 or A-Z) to bind", dontSendNotification);
    }

    void KeyCaptureOverlay::prepareForDisplay()
    {
        // Reset state
        reset();

        // Get current sample from audio player
        auto* props = SamplifyProperties::getInstance();
        if (props && props->getAudioPlayer())
        {
            mSample = props->getAudioPlayer()->getSampleReference();
            mStartTime = props->getAudioPlayer()->getStartCueRelative();
        }

        if (mSample.isNull())
        {
            mInstructionLabel.setText("No sample loaded!", dontSendNotification);
        }

        // Defer focus grab to ensure component is fully laid out
        Component::SafePointer<KeyCaptureOverlay> safeThis(this);
        MessageManager::callAsync([safeThis]() {
            if (safeThis != nullptr && safeThis->isShowing())
                safeThis->grabKeyboardFocus();
        });
    }

    void KeyCaptureOverlay::paint(Graphics& g)
    {
        auto& theme = ThemeManager::getInstance();
        g.fillAll(theme.getColorForRole(ThemeManager::ColorRole::Background));
    }

    void KeyCaptureOverlay::resized()
    {
        auto bounds = getLocalBounds().reduced(20);

        mInstructionLabel.setBounds(bounds.removeFromTop(30));
        bounds.removeFromTop(10);

        mKeyDisplayLabel.setBounds(bounds.removeFromTop(80));
        bounds.removeFromTop(20);

        auto buttonArea = bounds.removeFromBottom(35);
        mCancelButton.setBounds(buttonArea.withSizeKeepingCentre(100, 30));
    }

    bool KeyCaptureOverlay::keyPressed(const KeyPress& key, Component*)
    {
        // Escape to cancel
        if (key == KeyPress::escapeKey)
        {
            setState(State::Cancelled);
            if (mParentOverlay)
                mParentOverlay->hide();
            return true;
        }

        if (!isValidCueKey(key))
        {
            mInstructionLabel.setText("Invalid key. Use 0-9 or A-Z", dontSendNotification);
            return true;
        }

        switch (mState)
        {
            case State::WaitingForFirstKey:
            {
                mCapturedKey = key;
                mKeyDisplayLabel.setText(CueManager::getKeyDisplayString(key), dontSendNotification);

                // Check if already bound
                if (CueManager::getInstance().hasBinding(key))
                {
                    mInstructionLabel.setText("Key occupied! Press again to replace, or different key",
                                              dontSendNotification);
                }
                else
                {
                    mInstructionLabel.setText("Press " + CueManager::getKeyDisplayString(key) +
                                              " again to confirm", dontSendNotification);
                }
                setState(State::WaitingForConfirmation);
                break;
            }

            case State::WaitingForConfirmation:
            {
                if (key.getKeyCode() == mCapturedKey.getKeyCode())
                {
                    // Confirmed - create binding
                    createBinding();
                    setState(State::Confirmed);

                    // Close overlay
                    if (mParentOverlay)
                        mParentOverlay->hide();
                }
                else
                {
                    // Different key - start over with new key
                    mCapturedKey = key;
                    mKeyDisplayLabel.setText(CueManager::getKeyDisplayString(key), dontSendNotification);

                    if (CueManager::getInstance().hasBinding(key))
                    {
                        mInstructionLabel.setText("Key occupied! Press again to replace",
                                                  dontSendNotification);
                    }
                    else
                    {
                        mInstructionLabel.setText("Press " + CueManager::getKeyDisplayString(key) +
                                                  " again to confirm", dontSendNotification);
                    }
                }
                break;
            }

            default:
                break;
        }

        return true;
    }

    void KeyCaptureOverlay::visibilityChanged()
    {
        // Note: prepareForDisplay() is called explicitly before showing
        // This is here as a fallback for any edge cases
        if (isVisible() && mState != State::WaitingForFirstKey)
        {
            reset();
        }
    }

    void KeyCaptureOverlay::themeChanged(ThemeManager::Theme)
    {
        repaint();
    }

    void KeyCaptureOverlay::colorChanged(ThemeManager::ColorRole, Colour)
    {
        repaint();
    }

    void KeyCaptureOverlay::setState(State newState)
    {
        mState = newState;
    }

    void KeyCaptureOverlay::createBinding()
    {
        if (mSample.isNull() || !mCapturedKey.isValid())
            return;

        CueManager::getInstance().addBinding(mCapturedKey, mSample, mStartTime);
    }

    bool KeyCaptureOverlay::isValidCueKey(const KeyPress& key) const
    {
        int code = key.getKeyCode();

        // Must have no modifiers (or just shift for letters is ok)
        if (key.getModifiers().isCommandDown() ||
            key.getModifiers().isCtrlDown() ||
            key.getModifiers().isAltDown())
        {
            return false;
        }

        // 0-9
        if (code >= '0' && code <= '9')
            return true;

        // a-z (case insensitive)
        if (code >= 'a' && code <= 'z')
            return true;
        if (code >= 'A' && code <= 'Z')
            return true;

        return false;
    }
}
