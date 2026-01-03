/*
  ==============================================================================

    InputContext.cpp
    Created: 2025
    Author:  Samplore Team

  ==============================================================================
*/

#include "InputContext.h"
#include <algorithm>

namespace samplore
{
    //==============================================================================
    // InputContext
    //==============================================================================

    InputContext::InputContext(const juce::String& name, int priority)
        : mName(name), mPriority(priority)
    {
    }

    void InputContext::bind(const juce::KeyPress& key, const juce::String& actionName,
                            std::function<void()> callback, const juce::String& description)
    {
        mBindings[normalizeKeyPress(key)] = { actionName, callback, description };
    }

    void InputContext::unbind(const juce::KeyPress& key)
    {
        mBindings.erase(normalizeKeyPress(key));
    }

    void InputContext::clear()
    {
        mBindings.clear();
    }

    bool InputContext::tryHandle(const juce::KeyPress& key)
    {
        if (!mEnabled)
            return false;

        auto it = mBindings.find(normalizeKeyPress(key));
        if (it != mBindings.end() && it->second.mCallback)
        {
            it->second.mCallback();
            return true;
        }
        return false;
    }

    bool InputContext::hasBinding(const juce::KeyPress& key) const
    {
        return mBindings.find(normalizeKeyPress(key)) != mBindings.end();
    }

    const InputBinding* InputContext::getBinding(const juce::KeyPress& key) const
    {
        auto it = mBindings.find(normalizeKeyPress(key));
        return (it != mBindings.end()) ? &it->second : nullptr;
    }

    //==============================================================================
    // InputContextManager
    //==============================================================================

    std::unique_ptr<InputContextManager> InputContextManager::sInstance = nullptr;

    void InputContextManager::initInstance()
    {
        sInstance = std::make_unique<InputContextManager>();
    }

    void InputContextManager::cleanupInstance()
    {
        if (sInstance)
            sInstance.reset();
    }

    InputContextManager& InputContextManager::getInstance()
    {
        jassert(sInstance != nullptr);
        return *sInstance;
    }

    InputContextManager::InputContextManager()
    {
    }

    InputContextManager::~InputContextManager()
    {
    }

    std::shared_ptr<InputContext> InputContextManager::createContext(const juce::String& name, int priority)
    {
        // Check if already exists
        for (auto& ctx : mContexts)
        {
            if (ctx->getName() == name)
                return ctx;
        }

        auto context = std::make_shared<InputContext>(name, priority);
        mContexts.push_back(context);
        sortContexts();
        sendChangeMessage();
        return context;
    }

    std::shared_ptr<InputContext> InputContextManager::getContext(const juce::String& name)
    {
        for (auto& ctx : mContexts)
        {
            if (ctx->getName() == name)
                return ctx;
        }
        return nullptr;
    }

    void InputContextManager::removeContext(const juce::String& name)
    {
        mContexts.erase(
            std::remove_if(mContexts.begin(), mContexts.end(),
                [&name](const auto& ctx) { return ctx->getName() == name; }),
            mContexts.end()
        );
        sendChangeMessage();
    }

    bool InputContextManager::handleKeyPress(const juce::KeyPress& key)
    {
        // Iterate contexts in priority order (highest first, already sorted)
        for (auto& context : mContexts)
        {
            if (context->tryHandle(key))
                return true;
        }
        return false;
    }

    void InputContextManager::sortContexts()
    {
        // Sort by priority descending (highest priority first)
        std::sort(mContexts.begin(), mContexts.end(),
            [](const auto& a, const auto& b) {
                return a->getPriority() > b->getPriority();
            });
    }
}
