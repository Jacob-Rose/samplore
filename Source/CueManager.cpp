/*
  ==============================================================================

    CueManager.cpp
    Created: 2025
    Author:  Samplore Team

  ==============================================================================
*/

#include "CueManager.h"
#include "SamplifyProperties.h"
#include "SampleLibrary.h"

namespace samplore
{
    std::unique_ptr<CueManager> CueManager::sInstance = nullptr;

    //==============================================================================
    void CueManager::initInstance()
    {
        sInstance = std::make_unique<CueManager>();
        sInstance->loadAllRacks();
    }

    void CueManager::cleanupInstance()
    {
        if (sInstance)
        {
            sInstance->saveCurrentRack();
            sInstance.reset();
        }
    }

    CueManager& CueManager::getInstance()
    {
        jassert(sInstance != nullptr);
        return *sInstance;
    }

    //==============================================================================
    CueManager::CueManager()
    {
        // Create and register the cue input context (starts disabled)
        mInputContext = InputContextManager::getInstance().createContext("Cues", CUE_CONTEXT_PRIORITY);
        mInputContext->setEnabled(false);
    }

    CueManager::~CueManager()
    {
        // Context will be cleaned up when InputContextManager is destroyed
    }

    //==============================================================================
    bool CueManager::isCueModeEnabled() const
    {
        return mInputContext && mInputContext->isEnabled();
    }

    void CueManager::setCueModeEnabled(bool enabled)
    {
        if (mInputContext && mInputContext->isEnabled() != enabled)
        {
            mInputContext->setEnabled(enabled);
            sendChangeMessage();
        }
    }

    void CueManager::toggleCueMode()
    {
        setCueModeEnabled(!isCueModeEnabled());
    }

    //==============================================================================
    bool CueManager::addBinding(const juce::KeyPress& key, Sample::Reference sample,
                                 double startTime, const juce::String& displayName)
    {
        if (!key.isValid() || sample.isNull())
            return false;

        // Create display name from sample filename if not provided
        juce::String name = displayName;
        if (name.isEmpty())
        {
            name = sample.getFile().getFileNameWithoutExtension();
        }

        // Store in our binding map
        mBindings[key] = CueBinding(sample, startTime, name);

        // Rebuild the input context bindings
        rebuildInputContext();

        sendChangeMessage();
        saveCurrentRack();
        return true;
    }

    bool CueManager::removeBinding(const juce::KeyPress& key)
    {
        auto it = mBindings.find(key);
        if (it != mBindings.end())
        {
            mBindings.erase(it);
            rebuildInputContext();
            sendChangeMessage();
            saveCurrentRack();
            return true;
        }
        return false;
    }

    void CueManager::removeAllBindingsForSample(Sample::Reference sample)
    {
        bool removed = false;
        auto it = mBindings.begin();
        while (it != mBindings.end())
        {
            if (!it->second.mSample.isNull() && it->second.mSample == sample)
            {
                it = mBindings.erase(it);
                removed = true;
            }
            else
            {
                ++it;
            }
        }

        if (removed)
        {
            rebuildInputContext();
            sendChangeMessage();
            saveCurrentRack();
        }
    }

    void CueManager::clearAllBindings()
    {
        if (!mBindings.empty())
        {
            mBindings.clear();
            rebuildInputContext();
            sendChangeMessage();
            saveCurrentRack();
        }
    }

    const CueBinding* CueManager::getBinding(const juce::KeyPress& key) const
    {
        auto it = mBindings.find(key);
        if (it != mBindings.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    bool CueManager::hasBinding(const juce::KeyPress& key) const
    {
        return mBindings.find(key) != mBindings.end();
    }

    //==============================================================================
    void CueManager::triggerCue(const juce::KeyPress& key)
    {
        const CueBinding* binding = getBinding(key);
        if (binding && binding->isValid())
        {
            triggerBinding(*binding);
        }
    }

    void CueManager::triggerBinding(const CueBinding& binding)
    {
        if (binding.mSample.isNull())
            return;

        auto* props = SamplifyProperties::getInstance();
        if (!props)
            return;

        auto audioPlayer = props->getAudioPlayer();
        if (audioPlayer)
        {
            audioPlayer->loadFile(binding.mSample);
            audioPlayer->playSample(static_cast<float>(binding.mStartTime));
        }
    }

    //==============================================================================
    void CueManager::rebuildInputContext()
    {
        if (!mInputContext)
            return;

        bool wasEnabled = mInputContext->isEnabled();
        mInputContext->clear();

        // Add all bindings to the input context
        for (const auto& [key, binding] : mBindings)
        {
            if (binding.isValid())
            {
                // Capture key by value for the lambda
                juce::KeyPress capturedKey = key;
                mInputContext->bind(key, binding.mDisplayName,
                    [this, capturedKey]() { triggerCue(capturedKey); },
                    binding.mDisplayName);
            }
        }

        mInputContext->setEnabled(wasEnabled);
    }

    //==============================================================================
    std::vector<juce::KeyPress> CueManager::getAvailableKeys()
    {
        std::vector<juce::KeyPress> keys;

        // Number keys 0-9
        for (int i = 0; i <= 9; ++i)
        {
            keys.push_back(juce::KeyPress('0' + i));
        }

        // Letter keys a-z
        for (int i = 0; i < 26; ++i)
        {
            keys.push_back(juce::KeyPress('a' + i));
        }

        return keys;
    }

    juce::String CueManager::getKeyDisplayString(const juce::KeyPress& key)
    {
        int keyCode = key.getKeyCode();

        if (keyCode >= '0' && keyCode <= '9')
        {
            return juce::String::charToString(static_cast<juce::juce_wchar>(keyCode));
        }
        else if (keyCode >= 'a' && keyCode <= 'z')
        {
            return juce::String::charToString(static_cast<juce::juce_wchar>(keyCode - 32)); // Uppercase
        }

        return key.getTextDescription();
    }

    //==============================================================================
    // Rack Management
    //==============================================================================

    juce::File CueManager::getRacksDirectory() const
    {
        auto* props = SamplifyProperties::getInstance();
        if (!props)
            return juce::File();

        auto* settings = props->getUserSettings();
        if (!settings)
            return juce::File();

        juce::File appDataDir = settings->getFile().getParentDirectory();
        juce::File racksDir = appDataDir.getChildFile("CueRacks");
        if (!racksDir.exists())
            racksDir.createDirectory();
        return racksDir;
    }

    juce::File CueManager::getRackFile(const juce::String& rackName) const
    {
        return getRacksDirectory().getChildFile(rackName + ".xml");
    }

    juce::StringArray CueManager::getRackNames() const
    {
        juce::StringArray names;
        juce::File racksDir = getRacksDirectory();

        if (racksDir.exists())
        {
            for (const auto& entry : juce::RangedDirectoryIterator(racksDir, false, "*.xml"))
            {
                names.add(entry.getFile().getFileNameWithoutExtension());
            }
        }

        names.sort(true);
        return names;
    }

    bool CueManager::loadRack(const juce::String& name)
    {
        CueRack rack;
        if (!loadRackFromFile(name, rack))
            return false;

        mBindings = rack.mBindings;
        mCurrentRackName = name;
        rebuildInputContext();
        sendChangeMessage();
        return true;
    }

    bool CueManager::saveCurrentRack()
    {
        if (mCurrentRackName.isEmpty())
            return false;

        CueRack rack(mCurrentRackName);
        rack.mBindings = mBindings;
        return saveRackToFile(rack);
    }

    bool CueManager::saveRackAs(const juce::String& name)
    {
        if (name.isEmpty())
            return false;

        mCurrentRackName = name;
        CueRack rack(name);
        rack.mBindings = mBindings;
        bool success = saveRackToFile(rack);
        if (success)
            sendChangeMessage();
        return success;
    }

    bool CueManager::createNewRack(const juce::String& name)
    {
        if (name.isEmpty())
            return false;

        // Check if rack already exists
        if (getRackFile(name).exists())
            return false;

        // Save current rack first if it has a name
        if (mCurrentRackName.isNotEmpty())
            saveCurrentRack();

        // Start fresh
        mBindings.clear();
        mCurrentRackName = name;
        rebuildInputContext();

        // Save empty rack
        CueRack rack(name);
        saveRackToFile(rack);

        sendChangeMessage();
        return true;
    }

    bool CueManager::deleteRack(const juce::String& name)
    {
        if (name.isEmpty())
            return false;

        juce::File rackFile = getRackFile(name);
        if (!rackFile.exists())
            return false;

        bool deleted = rackFile.deleteFile();

        // If we deleted the current rack, switch to another or create default
        if (deleted && mCurrentRackName == name)
        {
            juce::StringArray remaining = getRackNames();
            if (remaining.isEmpty())
            {
                createNewRack("Default");
            }
            else
            {
                loadRack(remaining[0]);
            }
        }

        sendChangeMessage();
        return deleted;
    }

    bool CueManager::renameRack(const juce::String& oldName, const juce::String& newName)
    {
        if (oldName.isEmpty() || newName.isEmpty() || oldName == newName)
            return false;

        juce::File oldFile = getRackFile(oldName);
        juce::File newFile = getRackFile(newName);

        if (!oldFile.exists() || newFile.exists())
            return false;

        // Load, rename, save, delete old
        CueRack rack;
        if (!loadRackFromFile(oldName, rack))
            return false;

        rack.mName = newName;
        if (!saveRackToFile(rack))
            return false;

        oldFile.deleteFile();

        if (mCurrentRackName == oldName)
            mCurrentRackName = newName;

        sendChangeMessage();
        return true;
    }

    bool CueManager::saveRackToFile(const CueRack& rack)
    {
        juce::File rackFile = getRackFile(rack.mName);

        juce::XmlElement xml("CueRack");
        xml.setAttribute("name", rack.mName);

        for (const auto& [key, binding] : rack.mBindings)
        {
            if (binding.isValid())
            {
                auto* cueXml = xml.createNewChildElement("Cue");
                cueXml->setAttribute("key", key.getTextDescription());
                cueXml->setAttribute("file", binding.mSample.getFile().getFullPathName());
                cueXml->setAttribute("startTime", binding.mStartTime);
                cueXml->setAttribute("displayName", binding.mDisplayName);
                cueXml->setAttribute("hue", static_cast<double>(binding.mHue));
            }
        }

        return xml.writeTo(rackFile);
    }

    bool CueManager::loadRackFromFile(const juce::String& name, CueRack& outRack)
    {
        juce::File rackFile = getRackFile(name);

        if (!rackFile.exists())
            return false;

        auto xml = juce::XmlDocument::parse(rackFile);
        if (!xml || xml->getTagName() != "CueRack")
            return false;

        auto* props = SamplifyProperties::getInstance();
        if (!props)
            return false;

        auto library = props->getSampleLibrary();
        if (!library)
            return false;

        outRack.mName = xml->getStringAttribute("name", name);
        outRack.mBindings.clear();

        for (auto* cueXml : xml->getChildWithTagNameIterator("Cue"))
        {
            juce::String keyDesc = cueXml->getStringAttribute("key");
            juce::String filePath = cueXml->getStringAttribute("file");
            double startTime = cueXml->getDoubleAttribute("startTime", 0.0);
            juce::String displayName = cueXml->getStringAttribute("displayName");
            float hue = static_cast<float>(cueXml->getDoubleAttribute("hue", -1.0));

            if (keyDesc.isNotEmpty() && filePath.isNotEmpty())
            {
                juce::KeyPress key = juce::KeyPress::createFromDescription(keyDesc);
                juce::File file(filePath);

                if (key.isValid() && file.existsAsFile())
                {
                    // Search for sample in library
                    Sample::Reference sampleRef = library->findSampleByFile(file);

                    if (!sampleRef.isNull())
                    {
                        outRack.mBindings[key] = CueBinding(sampleRef, startTime, displayName, hue);
                    }
                }
            }
        }

        return true;
    }

    void CueManager::saveAllRacks()
    {
        // Save current rack
        saveCurrentRack();

        // Also save cue mode state to settings
        auto* props = SamplifyProperties::getInstance();
        if (props)
        {
            auto* settings = props->getUserSettings();
            if (settings)
            {
                settings->setValue("cue_mode_enabled", isCueModeEnabled());
                settings->setValue("current_cue_rack", mCurrentRackName);
                props->savePropertiesFile();
            }
        }
    }

    void CueManager::loadAllRacks()
    {
        auto* props = SamplifyProperties::getInstance();
        if (!props)
            return;

        auto* settings = props->getUserSettings();
        if (!settings)
            return;

        bool cueModeEnabled = settings->getBoolValue("cue_mode_enabled", false);
        juce::String lastRackName = settings->getValue("current_cue_rack", "Default");

        // Ensure we have at least one rack
        juce::StringArray rackNames = getRackNames();
        if (rackNames.isEmpty())
        {
            createNewRack("Default");
        }
        else if (rackNames.contains(lastRackName))
        {
            loadRack(lastRackName);
        }
        else
        {
            loadRack(rackNames[0]);
        }

        setCueModeEnabled(cueModeEnabled);
    }
}
