/*
  ==============================================================================

    PerformanceProfiler.h
    Created: 1 Jan 2026
    Author:  OpenCode

    Summary: Comprehensive performance profiling with rolling statistics

  ==============================================================================
*/

#ifndef PERFORMANCEPROFILER_H
#define PERFORMANCEPROFILER_H


#include "JuceHeader.h"
#include <map>
#include <vector>
#include <deque>
#include <algorithm>
#include <numeric>

namespace samplore
{
    /// Comprehensive performance profiler with rolling statistics
    class PerformanceProfiler : public Timer
    {
    public:
        static PerformanceProfiler& getInstance()
        {
            static PerformanceProfiler instance;
            return instance;
        }

        /// Configuration
        static constexpr double ROLLING_WINDOW_SECONDS = 5.0;
        static constexpr double SLOW_THRESHOLD_MS = 2.0;
        static constexpr int MAX_SAMPLES_PER_OPERATION = 1000;

        struct TimedSample
        {
            double timestamp;   // Time when sample was recorded
            double duration;    // Duration in milliseconds
        };

        struct OperationStats
        {
            std::deque<TimedSample> samples;
            int64 totalCount = 0;
            double allTimeMax = 0.0;
            double allTimeTotal = 0.0;

            // Computed rolling stats (updated periodically)
            double rollingAvg = 0.0;
            double rollingMin = 0.0;
            double rollingMax = 0.0;
            double rollingP50 = 0.0;
            double rollingP95 = 0.0;
            double rollingP99 = 0.0;
            int rollingSampleCount = 0;
            double rollingTotal = 0.0;
        };

        /// Records a timing for an operation
        void record(const String& operationName, double milliseconds)
        {
            if (!mEnabled)
                return;

            const ScopedLock lock(mMutex);

            double now = Time::getMillisecondCounterHiRes();
            auto& stats = mStats[operationName];

            // Add sample
            stats.samples.push_back({now, milliseconds});
            stats.totalCount++;
            stats.allTimeTotal += milliseconds;
            stats.allTimeMax = jmax(stats.allTimeMax, milliseconds);

            // Trim old samples
            double cutoff = now - (ROLLING_WINDOW_SECONDS * 1000.0);
            while (!stats.samples.empty() && stats.samples.front().timestamp < cutoff)
            {
                stats.samples.pop_front();
            }

            // Cap sample count
            while (stats.samples.size() > MAX_SAMPLES_PER_OPERATION)
            {
                stats.samples.pop_front();
            }
        }

        /// Computes rolling statistics for all operations
        void computeRollingStats()
        {
            const ScopedLock lock(mMutex);

            double now = Time::getMillisecondCounterHiRes();
            double cutoff = now - (ROLLING_WINDOW_SECONDS * 1000.0);

            for (auto& pair : mStats)
            {
                auto& stats = pair.second;

                // Collect samples within window
                std::vector<double> windowSamples;
                for (const auto& sample : stats.samples)
                {
                    if (sample.timestamp >= cutoff)
                    {
                        windowSamples.push_back(sample.duration);
                    }
                }

                stats.rollingSampleCount = (int)windowSamples.size();

                if (windowSamples.empty())
                {
                    stats.rollingAvg = 0.0;
                    stats.rollingMin = 0.0;
                    stats.rollingMax = 0.0;
                    stats.rollingP50 = 0.0;
                    stats.rollingP95 = 0.0;
                    stats.rollingP99 = 0.0;
                    stats.rollingTotal = 0.0;
                    continue;
                }

                // Sort for percentiles
                std::sort(windowSamples.begin(), windowSamples.end());

                stats.rollingTotal = std::accumulate(windowSamples.begin(), windowSamples.end(), 0.0);
                stats.rollingAvg = stats.rollingTotal / windowSamples.size();
                stats.rollingMin = windowSamples.front();
                stats.rollingMax = windowSamples.back();

                // Percentiles
                auto percentile = [&](double p) -> double {
                    if (windowSamples.empty()) return 0.0;
                    size_t idx = (size_t)(p * (windowSamples.size() - 1));
                    return windowSamples[jmin(idx, windowSamples.size() - 1)];
                };

                stats.rollingP50 = percentile(0.50);
                stats.rollingP95 = percentile(0.95);
                stats.rollingP99 = percentile(0.99);
            }
        }

        /// Gets the worst N samples in the rolling window for an operation
        std::vector<double> getWorstSamples(const String& operationName, int count = 5)
        {
            const ScopedLock lock(mMutex);

            std::vector<double> result;
            auto it = mStats.find(operationName);
            if (it == mStats.end())
                return result;

            double now = Time::getMillisecondCounterHiRes();
            double cutoff = now - (ROLLING_WINDOW_SECONDS * 1000.0);

            std::vector<double> windowSamples;
            for (const auto& sample : it->second.samples)
            {
                if (sample.timestamp >= cutoff)
                {
                    windowSamples.push_back(sample.duration);
                }
            }

            std::sort(windowSamples.begin(), windowSamples.end(), std::greater<double>());

            for (int i = 0; i < count && i < (int)windowSamples.size(); i++)
            {
                result.push_back(windowSamples[i]);
            }

            return result;
        }

        /// Prints comprehensive statistics
        void printStatistics()
        {
            computeRollingStats();

            const ScopedLock lock(mMutex);

            DBG("================================================================================");
            DBG("PERFORMANCE STATISTICS (Rolling " + String(ROLLING_WINDOW_SECONDS) + "s window)");
            DBG("================================================================================");

            // Sort by rolling total time (most impactful first)
            std::vector<std::pair<String, OperationStats>> sorted;
            for (const auto& pair : mStats)
                sorted.push_back(pair);

            std::sort(sorted.begin(), sorted.end(),
                [](const auto& a, const auto& b) {
                    return a.second.rollingTotal > b.second.rollingTotal;
                });

            for (const auto& pair : sorted)
            {
                const auto& name = pair.first;
                const auto& stats = pair.second;

                if (stats.rollingSampleCount == 0)
                    continue;

                DBG("");
                DBG("--- " + name + " ---");
                DBG("  Samples (5s):  " + String(stats.rollingSampleCount) +
                    "  (" + String(stats.rollingSampleCount / ROLLING_WINDOW_SECONDS, 1) + "/sec)");
                DBG("  Total (5s):    " + String(stats.rollingTotal, 2) + " ms");
                DBG("  Average:       " + String(stats.rollingAvg, 3) + " ms");
                DBG("  Min:           " + String(stats.rollingMin, 3) + " ms");
                DBG("  Max:           " + String(stats.rollingMax, 3) + " ms");
                DBG("  P50:           " + String(stats.rollingP50, 3) + " ms");
                DBG("  P95:           " + String(stats.rollingP95, 3) + " ms");
                DBG("  P99:           " + String(stats.rollingP99, 3) + " ms");
                DBG("  All-time max:  " + String(stats.allTimeMax, 3) + " ms");
                DBG("  All-time count:" + String(stats.totalCount));

                // Show worst frames
                auto worst = getWorstSamples(name, 5);
                if (!worst.empty())
                {
                    String worstStr = "  Worst 5:       ";
                    for (int i = 0; i < (int)worst.size(); i++)
                    {
                        if (i > 0) worstStr += ", ";
                        worstStr += String(worst[i], 2) + "ms";
                    }
                    DBG(worstStr);
                }
            }

            DBG("");
            DBG("================================================================================");
            printFrameTimeSummary();
            DBG("================================================================================");
        }

        /// Prints a quick frame time summary (for HUD-style display)
        void printFrameTimeSummary()
        {
            const ScopedLock lock(mMutex);

            // Aggregate key scroll operations
            double totalScrollTime = 0.0;
            int scrollOps = 0;

            std::vector<String> scrollOperations = {
                "SampleViewport::visibleAreaChanged",
                "SampleContainer::updateVisibleItems",
                "SampleTile::setSample",
                "SampleTile::resized",
                "SampleTile::paint"
            };

            for (const auto& opName : scrollOperations)
            {
                auto it = mStats.find(opName);
                if (it != mStats.end() && it->second.rollingSampleCount > 0)
                {
                    totalScrollTime += it->second.rollingTotal;
                    scrollOps += it->second.rollingSampleCount;
                }
            }

            DBG("SCROLL PERFORMANCE SUMMARY:");
            DBG("  Total scroll-related time (5s): " + String(totalScrollTime, 1) + " ms");
            DBG("  Total scroll operations (5s):   " + String(scrollOps));
            if (scrollOps > 0)
            {
                DBG("  Avg time per scroll event:      " + String(totalScrollTime / scrollOps, 3) + " ms");
            }

            // Find the slowest operation
            String slowestOp;
            double slowestAvg = 0.0;
            for (const auto& opName : scrollOperations)
            {
                auto it = mStats.find(opName);
                if (it != mStats.end() && it->second.rollingAvg > slowestAvg)
                {
                    slowestAvg = it->second.rollingAvg;
                    slowestOp = opName;
                }
            }
            if (!slowestOp.isEmpty())
            {
                DBG("  Slowest operation (avg):        " + slowestOp + " @ " + String(slowestAvg, 3) + " ms");
            }

            // Find operation with worst peak
            String worstPeakOp;
            double worstPeak = 0.0;
            for (const auto& opName : scrollOperations)
            {
                auto it = mStats.find(opName);
                if (it != mStats.end() && it->second.rollingMax > worstPeak)
                {
                    worstPeak = it->second.rollingMax;
                    worstPeakOp = opName;
                }
            }
            if (!worstPeakOp.isEmpty())
            {
                DBG("  Worst peak (5s):                " + worstPeakOp + " @ " + String(worstPeak, 2) + " ms");
            }
        }

        /// Live HUD string for overlay (if you want to show in UI)
        String getLiveHUD()
        {
            computeRollingStats();
            const ScopedLock lock(mMutex);

            String hud;

            auto getStats = [&](const String& name) -> String {
                auto it = mStats.find(name);
                if (it == mStats.end() || it->second.rollingSampleCount == 0)
                    return name.fromLastOccurrenceOf("::", false, false) + ": --";

                const auto& s = it->second;
                return name.fromLastOccurrenceOf("::", false, false) +
                       ": " + String(s.rollingAvg, 2) + "ms avg, " +
                       String(s.rollingMax, 1) + "ms peak";
            };

            hud += getStats("SampleViewport::visibleAreaChanged") + "\n";
            hud += getStats("SampleContainer::updateVisibleItems") + "\n";
            hud += getStats("SampleTile::setSample") + "\n";
            hud += getStats("SampleTile::paint") + "\n";

            return hud;
        }

        /// Enables/disables profiling
        void setEnabled(bool enabled)
        {
            mEnabled = enabled;
            if (enabled)
            {
                startTimer(1000); // Update stats every second
                DBG("PerformanceProfiler ENABLED - press P to print stats");
            }
            else
            {
                stopTimer();
                DBG("PerformanceProfiler DISABLED");
            }
        }

        bool isEnabled() const { return mEnabled; }

        /// Clears all statistics
        void reset()
        {
            const ScopedLock lock(mMutex);
            mStats.clear();
            DBG("PerformanceProfiler stats RESET");
        }

        // Timer callback - auto-print summary periodically if enabled
        void timerCallback() override
        {
            computeRollingStats();
            // Uncomment to auto-print every second:
            // printFrameTimeSummary();
        }

    private:
        PerformanceProfiler() = default;
        ~PerformanceProfiler() { stopTimer(); }

        std::map<String, OperationStats> mStats;
        CriticalSection mMutex;
        bool mEnabled = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerformanceProfiler)
    };

    /// RAII helper for measuring operation time
    class ScopedProfileTimer
    {
    public:
        ScopedProfileTimer(const String& name, double thresholdMs = -1.0)
            : mName(name)
            , mStartTime(Time::getMillisecondCounterHiRes())
        {
        }

        ~ScopedProfileTimer()
        {
            double elapsed = Time::getMillisecondCounterHiRes() - mStartTime;
            PerformanceProfiler::getInstance().record(mName, elapsed);
        }

    private:
        String mName;
        double mStartTime;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScopedProfileTimer)
    };

    /// Simpler timer just for paint (backwards compat)
    using ScopedPaintTimer = ScopedProfileTimer;

    /// Utility to log function entry/exit with timing
    class FunctionTracer
    {
    public:
        FunctionTracer(const String& funcName)
            : mFuncName(funcName)
            , mStartTime(Time::getMillisecondCounterHiRes())
        {
            DBG(">> ENTER: " + mFuncName);
        }

        ~FunctionTracer()
        {
            double elapsed = Time::getMillisecondCounterHiRes() - mStartTime;
            DBG("<< EXIT:  " + mFuncName + " (" + String(elapsed, 2) + " ms)");
        }

    private:
        String mFuncName;
        double mStartTime;
    };
}

// Convenience macros
#define PROFILE_PAINT(name) samplore::ScopedProfileTimer _paintTimer(name)
#define PROFILE_SCOPE(name) samplore::ScopedProfileTimer _scopeTimer(name)
#define PROFILE_RESIZED(name) samplore::ScopedProfileTimer _resizedTimer(String("resized::") + name)
#define PROFILE_VISIBLE_AREA(name) samplore::ScopedProfileTimer _visibleTimer(String("visibleArea::") + name)
#define PROFILE_UPDATE(name) samplore::ScopedProfileTimer _updateTimer(String("update::") + name)
#define TRACE_FUNCTION() samplore::FunctionTracer _funcTracer(__FUNCTION__)

#endif
