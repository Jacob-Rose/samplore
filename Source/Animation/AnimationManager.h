/*
  ==============================================================================

    AnimationManager.h
    Created: 2025
    Author:  SamplifyPlus Team

  ==============================================================================
*/

#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include "JuceHeader.h"
#include <functional>
#include <vector>

namespace samplify
{
    // ====== Easing Functions ======
    namespace Easing
    {
        inline float linear(float t) { return t; }

        inline float easeOutCubic(float t) {
            return 1.0f - std::pow(1.0f - t, 3.0f);
        }

        inline float easeInCubic(float t) {
            return t * t * t;
        }

        inline float easeInOutCubic(float t) {
            return t < 0.5f
                ? 4.0f * t * t * t
                : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
        }

        inline float easeOutQuad(float t) {
            return 1.0f - (1.0f - t) * (1.0f - t);
        }

        inline float easeInQuad(float t) {
            return t * t;
        }

        inline float easeInOutQuad(float t) {
            return t < 0.5f
                ? 2.0f * t * t
                : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
        }
    }

    // ====== Animation Base Class ======
    class Animation
    {
    public:
        using EasingFunction = std::function<float(float)>;
        using UpdateCallback = std::function<void(float)>;
        using CompletionCallback = std::function<void()>;

        Animation(float durationMs, EasingFunction easing = Easing::easeOutCubic)
            : duration(durationMs)
            , elapsed(0.0f)
            , easingFunction(easing)
            , isComplete(false)
            , loop(false)
        {}

        virtual ~Animation() {}

        void setLoop(bool shouldLoop) { loop = shouldLoop; }
        void setOnComplete(CompletionCallback callback) { onComplete = callback; }

        bool update(float deltaTimeMs)
        {
            if (isComplete && !loop)
                return false;

            elapsed += deltaTimeMs;

            if (elapsed >= duration)
            {
                if (loop)
                {
                    elapsed = std::fmod(elapsed, duration);
                }
                else
                {
                    elapsed = duration;
                    isComplete = true;
                }
            }

            float progress = duration > 0.0f ? (elapsed / duration) : 1.0f;
            float easedProgress = easingFunction(progress);

            updateValue(easedProgress);

            if (isComplete && onComplete)
            {
                onComplete();
            }

            return !isComplete || loop;
        }

        bool finished() const { return isComplete; }

    protected:
        virtual void updateValue(float progress) = 0;

        float duration;
        float elapsed;
        EasingFunction easingFunction;
        bool isComplete;
        bool loop;
        CompletionCallback onComplete;
    };

    // ====== Specific Animation Types ======

    class FloatAnimation : public Animation
    {
    public:
        FloatAnimation(float* target, float from, float to, float durationMs,
                      EasingFunction easing = Easing::easeOutCubic)
            : Animation(durationMs, easing)
            , targetPtr(target)
            , fromValue(from)
            , toValue(to)
        {
            if (targetPtr)
                *targetPtr = fromValue;
        }

    protected:
        void updateValue(float progress) override
        {
            if (targetPtr)
            {
                *targetPtr = fromValue + (toValue - fromValue) * progress;
            }
        }

    private:
        float* targetPtr;
        float fromValue;
        float toValue;
    };

    class ColourAnimation : public Animation
    {
    public:
        ColourAnimation(Colour* target, Colour from, Colour to, float durationMs,
                       EasingFunction easing = Easing::easeOutCubic)
            : Animation(durationMs, easing)
            , targetPtr(target)
            , fromColour(from)
            , toColour(to)
        {
            if (targetPtr)
                *targetPtr = fromColour;
        }

    protected:
        void updateValue(float progress) override
        {
            if (targetPtr)
            {
                *targetPtr = fromColour.interpolatedWith(toColour, progress);
            }
        }

    private:
        Colour* targetPtr;
        Colour fromColour;
        Colour toColour;
    };

    class BoundsAnimation : public Animation
    {
    public:
        BoundsAnimation(Rectangle<int>* target, Rectangle<int> from, Rectangle<int> to,
                       float durationMs, EasingFunction easing = Easing::easeOutCubic)
            : Animation(durationMs, easing)
            , targetPtr(target)
            , fromBounds(from)
            , toBounds(to)
        {
            if (targetPtr)
                *targetPtr = fromBounds;
        }

    protected:
        void updateValue(float progress) override
        {
            if (targetPtr)
            {
                int x = (int)(fromBounds.getX() + (toBounds.getX() - fromBounds.getX()) * progress);
                int y = (int)(fromBounds.getY() + (toBounds.getY() - fromBounds.getY()) * progress);
                int w = (int)(fromBounds.getWidth() + (toBounds.getWidth() - fromBounds.getWidth()) * progress);
                int h = (int)(fromBounds.getHeight() + (toBounds.getHeight() - fromBounds.getHeight()) * progress);
                *targetPtr = Rectangle<int>(x, y, w, h);
            }
        }

    private:
        Rectangle<int>* targetPtr;
        Rectangle<int> fromBounds;
        Rectangle<int> toBounds;
    };

    // ====== Animator Component Mixin ======
    class AnimatedComponent : private Timer
    {
    public:
        AnimatedComponent() : animationSpeed(1.0f) {}

        virtual ~AnimatedComponent()
        {
            stopAllAnimations();
        }

        // Float animation
        void animateFloat(float* target, float from, float to, float durationMs,
                         Animation::EasingFunction easing = Easing::easeOutCubic)
        {
            auto anim = std::make_unique<FloatAnimation>(target, from, to, durationMs / animationSpeed, easing);
            addAnimation(std::move(anim));
        }

        // Color animation
        void animateColour(Colour* target, Colour from, Colour to, float durationMs,
                          Animation::EasingFunction easing = Easing::easeOutCubic)
        {
            auto anim = std::make_unique<ColourAnimation>(target, from, to, durationMs / animationSpeed, easing);
            addAnimation(std::move(anim));
        }

        // Bounds animation
        void animateBounds(Rectangle<int>* target, Rectangle<int> from, Rectangle<int> to,
                          float durationMs, Animation::EasingFunction easing = Easing::easeOutCubic)
        {
            auto anim = std::make_unique<BoundsAnimation>(target, from, to, durationMs / animationSpeed, easing);
            addAnimation(std::move(anim));
        }

        // Control
        void stopAllAnimations()
        {
            animations.clear();
            stopTimer();
        }

        void setAnimationSpeed(float speed)
        {
            animationSpeed = jmax(0.1f, speed);
        }

        bool hasActiveAnimations() const
        {
            return !animations.empty();
        }

    protected:
        // Component must call this to trigger repaints
        virtual void onAnimationUpdate() = 0;

    private:
        void addAnimation(std::unique_ptr<Animation> animation)
        {
            animations.push_back(std::move(animation));

            if (!isTimerRunning())
            {
                startTimerHz(60);  // 60 FPS
            }
        }

        void timerCallback() override
        {
            if (animations.empty())
            {
                stopTimer();
                return;
            }

            const float deltaTimeMs = 1000.0f / 60.0f;  // ~16.67ms per frame at 60 FPS

            // Update all animations
            for (auto it = animations.begin(); it != animations.end();)
            {
                bool stillRunning = (*it)->update(deltaTimeMs);

                if (!stillRunning)
                {
                    it = animations.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            // Notify component to repaint
            onAnimationUpdate();

            if (animations.empty())
            {
                stopTimer();
            }
        }

        std::vector<std::unique_ptr<Animation>> animations;
        float animationSpeed;
    };
}

#endif // ANIMATIONMANAGER_H
