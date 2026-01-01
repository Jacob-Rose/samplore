/*
  ==============================================================================

    PerformanceProfiler.h
    Created: 1 Jan 2026
    Author:  OpenCode
    
    Summary: Simple performance profiling utilities for paint operations

  ==============================================================================
*/

#ifndef PERFORMANCEPROFILER_H
#define PERFORMANCEPROFILER_H

#include "JuceHeader.h"
#include <map>
#include <vector>

namespace samplore
{
    /// Singleton manager for collecting paint performance metrics
    class PerformanceProfiler
    {
    public:
        static PerformanceProfiler& getInstance()
        {
            static PerformanceProfiler instance;
            return instance;
        }
        
        /// Records a paint timing for a component
        void recordPaint(const String& componentName, double milliseconds)
        {
            const ScopedLock lock(mMutex);
            
            auto& stats = mStats[componentName];
            stats.count++;
            stats.totalTime += milliseconds;
            stats.minTime = jmin(stats.minTime, milliseconds);
            stats.maxTime = jmax(stats.maxTime, milliseconds);
            stats.lastTime = milliseconds;
            
            // Keep recent samples for better statistics
            stats.recentSamples.push_back(milliseconds);
            if (stats.recentSamples.size() > 100)
                stats.recentSamples.erase(stats.recentSamples.begin());
        }
        
        /// Prints statistics to debug output
        void printStatistics()
        {
            const ScopedLock lock(mMutex);
            
            DBG("========================================");
            DBG("Paint Performance Statistics");
            DBG("========================================");
            
            // Sort by total time descending
            std::vector<std::pair<String, PaintStats>> sorted;
            for (const auto& pair : mStats)
                sorted.push_back(pair);
            
            std::sort(sorted.begin(), sorted.end(),
                [](const auto& a, const auto& b) {
                    return a.second.totalTime > b.second.totalTime;
                });
            
            for (const auto& pair : sorted)
            {
                const auto& name = pair.first;
                const auto& stats = pair.second;
                
                double avgTime = stats.totalTime / stats.count;
                
                DBG(name + ":");
                DBG("  Count: " + String(stats.count));
                DBG("  Total: " + String(stats.totalTime, 2) + " ms");
                DBG("  Avg:   " + String(avgTime, 3) + " ms");
                DBG("  Min:   " + String(stats.minTime, 3) + " ms");
                DBG("  Max:   " + String(stats.maxTime, 3) + " ms");
                DBG("  Last:  " + String(stats.lastTime, 3) + " ms");
            }
            
            DBG("========================================");
        }
        
        /// Clears all statistics
        void reset()
        {
            const ScopedLock lock(mMutex);
            mStats.clear();
        }
        
        /// Enables/disables profiling
        void setEnabled(bool enabled) { mEnabled = enabled; }
        bool isEnabled() const { return mEnabled; }
        
    private:
        PerformanceProfiler() = default;
        ~PerformanceProfiler() = default;
        
        struct PaintStats
        {
            int64 count = 0;
            double totalTime = 0.0;
            double minTime = std::numeric_limits<double>::max();
            double maxTime = 0.0;
            double lastTime = 0.0;
            std::vector<double> recentSamples;
        };
        
        std::map<String, PaintStats> mStats;
        CriticalSection mMutex;
        bool mEnabled = false;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerformanceProfiler)
    };
    
    /// RAII helper for measuring paint time
    class ScopedPaintTimer
    {
    public:
        ScopedPaintTimer(const String& componentName)
            : mComponentName(componentName)
            , mStartTime(Time::getMillisecondCounterHiRes())
        {
        }
        
        ~ScopedPaintTimer()
        {
            if (PerformanceProfiler::getInstance().isEnabled())
            {
                double elapsed = Time::getMillisecondCounterHiRes() - mStartTime;
                PerformanceProfiler::getInstance().recordPaint(mComponentName, elapsed);
            }
        }
        
    private:
        String mComponentName;
        double mStartTime;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScopedPaintTimer)
    };
}

// Convenience macro for profiling paint methods
#define PROFILE_PAINT(name) samplore::ScopedPaintTimer _paintTimer(name)

#endif
