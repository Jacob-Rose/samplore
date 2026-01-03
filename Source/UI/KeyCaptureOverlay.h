/*
  ==============================================================================

    KeyCaptureOverlay.h
    Created: 2025
    Author:  Samplore Team

    Overlay for capturing a key binding - user presses key twice to confirm

  ==============================================================================
*/

#ifndef KEYCAPTUREOVERLAY_H
#define KEYCAPTUREOVERLAY_H

#include "JuceHeader.h"
#include "IOverlayPanelContent.h"
#include "../ThemeManager.h"
#include "../Sample.h"

namespace samplore
{
    class KeyCaptureOverlay : public Component,
                               public IOverlayPanelContent,
                               public KeyListener,
                               public ThemeManager::Listener
    {
    public:
        KeyCaptureOverlay();
        ~KeyCaptureOverlay() override;

        /// Set the sample to bind (uses current player sample if not set)
        void setSample(Sample::Reference sample, double startTime = 0.0);

        /// Reset state for new capture
        void reset();

        /// Prepare for display - resets and fetches current sample
        void prepareForDisplay();

        // Component
        void paint(Graphics& g) override;
        void resized() override;
        bool keyPressed(const KeyPress& key, Component* originatingComponent) override;

        // IOverlayPanelContent
        String getOverlayTitle() const override { return "Bind Key"; }
        bool shouldShowBackButton() const override { return false; }
        void onOverlayBackButton() override {}
        void setParentOverlay(OverlayPanel* parent) override { mParentOverlay = parent; }

        // Called when overlay is shown/hidden
        void visibilityChanged() override;

        // ThemeManager::Listener
        void themeChanged(ThemeManager::Theme newTheme) override;
        void colorChanged(ThemeManager::ColorRole role, Colour newColor) override;

    private:
        enum class State
        {
            WaitingForFirstKey,
            WaitingForConfirmation,
            Confirmed,
            Cancelled
        };

        void setState(State newState);
        void createBinding();
        bool isValidCueKey(const KeyPress& key) const;

        State mState = State::WaitingForFirstKey;
        KeyPress mCapturedKey;
        Sample::Reference mSample = nullptr;
        double mStartTime = 0.0;
        OverlayPanel* mParentOverlay = nullptr;

        Label mInstructionLabel;
        Label mKeyDisplayLabel;
        TextButton mCancelButton{"Cancel"};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KeyCaptureOverlay)
    };
}

#endif // KEYCAPTUREOVERLAY_H
