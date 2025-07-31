/**
 * @file PerformanceProfiler.h
 * @brief Real-time performance monitoring and profiling system
 * 
 * Provides comprehensive performance analysis tools for game development:
 * - Frame rate monitoring and statistics
 * - CPU/GPU timing measurements
 * - Memory usage tracking
 * - Rendering statistics (draw calls, triangles)
 * - Section-based code profiling
 * - Performance data logging and export
 * 
 * Features:
 * - Singleton pattern for global access
 * - Low-overhead timing measurements
 * - Configurable history buffer for trend analysis
 * - Real-time performance metrics
 * - Detailed performance logging to files
 */

#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>

/**
 * @brief Performance monitoring and profiling system
 * 
 * Singleton class that provides comprehensive performance analysis
 * capabilities for real-time applications. Tracks frame timing,
 * rendering statistics, and custom code section performance.
 * 
 * Key capabilities:
 * - Frame-by-frame performance tracking
 * - Custom timing sections for code profiling
 * - Memory usage monitoring
 * - Rendering performance metrics
 * - Performance data export and logging
 */
class PerformanceProfiler {
public:
    /**
     * @brief Frame performance statistics structure
     * 
     * Contains all relevant performance metrics for a single frame
     * including timing, memory usage, and rendering statistics.
     */
    struct FrameStats {
        float frameTime;      // Total frame time in milliseconds
        float fps;           // Frames per second
        float cpuTime;       // CPU processing time in milliseconds
        float gpuTime;       // GPU rendering time in milliseconds
        size_t memoryUsage;  // Current memory usage in bytes
        int drawCalls;       // Number of draw calls issued this frame
        int triangles;       // Total triangles rendered this frame
    };

    /**
     * @brief Timing data for custom code sections
     * 
     * Tracks performance statistics for user-defined code sections
     * including average, minimum, and maximum execution times.
     */
    struct SectionTiming {
        std::chrono::high_resolution_clock::time_point startTime;  // Section start timestamp
        float totalTime = 0.0f;     // Cumulative execution time
        int callCount = 0;          // Number of times section was executed
        float avgTime = 0.0f;       // Average execution time per call
        float maxTime = 0.0f;       // Maximum recorded execution time
        float minTime = FLT_MAX;    // Minimum recorded execution time
    };

private:
    static PerformanceProfiler* instance;  // Singleton instance
    
    // Frame performance tracking
    std::vector<FrameStats> frameHistory;  // Historical frame performance data
    FrameStats currentFrame;               // Current frame statistics
    
    // Custom section timing
    std::unordered_map<std::string, SectionTiming> timingSections;  // Named timing sections
    
    // Configuration settings
    bool enableProfiling = true;        // Master enable/disable switch
    bool enableDetailedLogging = false; // Enable detailed log output
    int maxFrameHistory = 300;          // Maximum frames to keep in history (5 seconds at 60fps)
    
    // Frame timing infrastructure
    std::chrono::high_resolution_clock::time_point frameStartTime;  // Current frame start time
    std::chrono::high_resolution_clock::time_point lastFrameTime;   // Previous frame timestamp
    
    // Performance smoothing and analysis
    float smoothedFPS = 60.0f;      // Exponentially smoothed FPS for stable display
    float fpsAlpha = 0.1f;          // Smoothing factor for FPS calculation (0.0-1.0)
    
    // Performance thresholds for warnings
    float targetFPS = 60.0f;            // Target frame rate for performance assessment
    float warningFrameTime = 20.0f;     // Frame time threshold for performance warnings (ms)
    float criticalFrameTime = 33.3f;    // Frame time threshold for critical performance issues (ms)

public:
    /**
     * @brief Get singleton instance of the performance profiler
     * 
     * Provides global access to the performance profiler using
     * the singleton pattern. Creates instance on first access.
     * 
     * @return Reference to the singleton PerformanceProfiler instance
     */
    static PerformanceProfiler& getInstance() {
        if (!instance) {
            instance = new PerformanceProfiler();
        }
        return *instance;
    }

    /**
     * @brief Mark the beginning of a new frame
     * 
     * Call this at the start of each frame to begin frame timing.
     * Resets frame-specific counters and starts timing measurements.
     */
    void BeginFrame();
    
    /**
     * @brief Mark the end of the current frame
     * 
     * Call this at the end of each frame to complete timing measurements.
     * Calculates frame statistics and updates performance history.
     */
    void EndFrame();
    
    /**
     * @brief Begin timing a custom code section
     * 
     * Starts timing measurement for a named code section.
     * Use with EndSection() to measure execution time of specific code blocks.
     * 
     * @param sectionName Unique identifier for the code section
     */
    void BeginSection(const std::string& sectionName);
    
    /**
     * @brief End timing a custom code section
     * 
     * Stops timing measurement for a named code section and updates statistics.
     * 
     * @param sectionName Name of the section to stop timing
     */
    void EndSection(const std::string& sectionName);
    
    /**
     * @brief Update rendering statistics for the current frame
     * 
     * Records the number of draw calls and triangles rendered this frame.
     * Call this after each frame's rendering is complete.
     * 
     * @param drawCalls Number of draw calls issued this frame
     * @param triangles Total number of triangles rendered this frame
     */
    void UpdateDrawCallStats(int drawCalls, int triangles);
    
    /**
     * @brief Update memory usage statistics
     * 
     * Records the current memory usage for performance tracking.
     * 
     * @param memoryBytes Current memory usage in bytes
     */
    void UpdateMemoryUsage(size_t memoryBytes);
    
    // Performance query methods
    /**
     * @brief Get average FPS over specified number of frames
     * 
     * @param frameCount Number of recent frames to average (default: 60)
     * @return Average frames per second over the specified period
     */
    float GetAverageFPS(int frameCount = 60) const;
    
    /**
     * @brief Get average frame time over specified number of frames
     * 
     * @param frameCount Number of recent frames to average (default: 60)
     * @return Average frame time in milliseconds
     */
    float GetAverageFrameTime(int frameCount = 60) const;
    
    /**
     * @brief Get current frame's FPS
     * @return Frames per second for the current frame
     */
    float GetCurrentFPS() const { return currentFrame.fps; }
    
    /**
     * @brief Get current frame's timing
     * @return Frame time in milliseconds for the current frame
     */
    float GetCurrentFrameTime() const { return currentFrame.frameTime; }
    
    // Performance analysis methods
    /**
     * @brief Check if performance is currently critical
     * 
     * Determines if frame time exceeds critical thresholds indicating
     * severe performance issues that need immediate attention.
     * 
     * @return True if performance is critically poor
     */
    bool IsPerformanceCritical() const;
    
    /**
     * @brief Generate comprehensive performance report
     * 
     * Creates a detailed text report of current performance metrics
     * including FPS, frame times, and section timings.
     * 
     * @return Formatted performance report string
     */
    std::string GetPerformanceReport() const;
    
    /**
     * @brief Identify performance bottlenecks
     * 
     * Analyzes timing data to identify the slowest code sections
     * and potential performance bottlenecks.
     * 
     * @return Vector of strings describing identified bottlenecks
     */
    std::vector<std::string> GetBottlenecks() const;
    
    // Configuration methods
    /**
     * @brief Enable or disable performance profiling
     * @param enable True to enable profiling, false to disable
     */
    void SetEnableProfiling(bool enable) { enableProfiling = enable; }
    
    /**
     * @brief Enable or disable detailed logging
     * @param enable True to enable detailed logs, false for basic logging
     */
    void SetDetailedLogging(bool enable) { enableDetailedLogging = enable; }
    
    /**
     * @brief Set target FPS for performance evaluation
     * @param fps Target frames per second
     */
    void SetTargetFPS(float fps) { targetFPS = fps; }
    
    // Data export and reporting methods
    /**
     * @brief Export performance data to file
     * 
     * Saves comprehensive performance history and statistics to a file
     * for offline analysis and debugging.
     * 
     * @param filename Path to output file
     */
    void ExportToFile(const std::string& filename) const;
    
    /**
     * @brief Print real-time performance statistics to console
     * 
     * Outputs current performance metrics to standard output
     * for immediate debugging feedback.
     */
    void PrintRealTimeStats() const;
    
    /**
     * @brief Clear all performance history data
     * 
     * Resets all accumulated performance statistics and timing data.
     * Useful for starting fresh measurements.
     */
    void ClearHistory();
    
    /**
     * @brief Get estimated memory usage of the profiler itself
     * 
     * Calculates approximate memory footprint of the performance
     * profiler's data structures.
     * 
     * @return Estimated memory usage in bytes
     */
    size_t GetEstimatedMemoryUsage() const;
    float GetGPULoad() const; 
    
private:
    PerformanceProfiler() = default;
    void AddFrameToHistory(const FrameStats& frame);
    float CalculateGPUTime() const;
};


class ScopedTimer {
private:
    std::string sectionName;
    
public:
    explicit ScopedTimer(const std::string& name) : sectionName(name) {
        PerformanceProfiler::getInstance().BeginSection(sectionName);
    }
    
    ~ScopedTimer() {
        PerformanceProfiler::getInstance().EndSection(sectionName);
    }
};


#define PROFILE_FUNCTION() ScopedTimer timer(__FUNCTION__)
#define PROFILE_SECTION(name) ScopedTimer timer(name)


class PerformanceMonitor {
public:
    static void RenderOverlay();
    static void LogPerformanceWarnings();
    static void CheckPerformanceThresholds();
};
