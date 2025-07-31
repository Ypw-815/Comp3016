#include "PerformanceProfiler.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

PerformanceProfiler* PerformanceProfiler::instance = nullptr;

void PerformanceProfiler::BeginFrame() {
    if (!enableProfiling) return;
    
    frameStartTime = std::chrono::high_resolution_clock::now();
    currentFrame = {}; 
}

void PerformanceProfiler::EndFrame() {
    if (!enableProfiling) return;
    
    auto frameEndTime = std::chrono::high_resolution_clock::now();
    auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(frameEndTime - frameStartTime);
    
    
    currentFrame.frameTime = frameDuration.count() / 1000.0f; 
    currentFrame.fps = (currentFrame.frameTime > 0) ? (1000.0f / currentFrame.frameTime) : 0.0f;
    
    
    smoothedFPS = smoothedFPS * (1.0f - fpsAlpha) + currentFrame.fps * fpsAlpha;
    
    
    currentFrame.cpuTime = 0.0f;
    for (const auto& [name, timing] : timingSections) {
        currentFrame.cpuTime += timing.totalTime;
    }
    
    
    currentFrame.gpuTime = CalculateGPUTime();
    
    
    AddFrameToHistory(currentFrame);
    
    
    if (enableDetailedLogging) {
        PrintRealTimeStats();
    }
    
    
    if (currentFrame.frameTime > warningFrameTime) {
        std::cout << "[PERFORMANCE WARNING] Frame time: " << currentFrame.frameTime 
                  << "ms (Target: " << (1000.0f / targetFPS) << "ms)" << std::endl;
    }
    
    lastFrameTime = frameEndTime;
}

void PerformanceProfiler::BeginSection(const std::string& sectionName) {
    if (!enableProfiling) return;
    
    auto& section = timingSections[sectionName];
    section.startTime = std::chrono::high_resolution_clock::now();
}

void PerformanceProfiler::EndSection(const std::string& sectionName) {
    if (!enableProfiling) return;
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto& section = timingSections[sectionName];
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - section.startTime);
    float sectionTime = duration.count() / 1000.0f; 
    
    
    section.totalTime += sectionTime;
    section.callCount++;
    section.avgTime = section.totalTime / section.callCount;
    section.maxTime = std::max(section.maxTime, sectionTime);
    section.minTime = std::min(section.minTime, sectionTime);
}

void PerformanceProfiler::UpdateDrawCallStats(int drawCalls, int triangles) {
    currentFrame.drawCalls = drawCalls;
    currentFrame.triangles = triangles;
}

void PerformanceProfiler::UpdateMemoryUsage(size_t memoryBytes) {
    currentFrame.memoryUsage = memoryBytes;
}

float PerformanceProfiler::GetAverageFPS(int frameCount) const {
    if (frameHistory.empty()) return 0.0f;
    
    int count = std::min(frameCount, (int)frameHistory.size());
    float totalFPS = 0.0f;
    
    for (int i = frameHistory.size() - count; i < frameHistory.size(); i++) {
        totalFPS += frameHistory[i].fps;
    }
    
    return totalFPS / count;
}

float PerformanceProfiler::GetAverageFrameTime(int frameCount) const {
    if (frameHistory.empty()) return 0.0f;
    
    int count = std::min(frameCount, (int)frameHistory.size());
    float totalTime = 0.0f;
    
    for (int i = frameHistory.size() - count; i < frameHistory.size(); i++) {
        totalTime += frameHistory[i].frameTime;
    }
    
    return totalTime / count;
}

bool PerformanceProfiler::IsPerformanceCritical() const {
    if (frameHistory.size() < 10) return false;
    
    
    int criticalFrames = 0;
    for (int i = std::max(0, (int)frameHistory.size() - 10); i < frameHistory.size(); i++) {
        if (frameHistory[i].frameTime > criticalFrameTime) {
            criticalFrames++;
        }
    }
    
    return criticalFrames >= 3; 
}

std::string PerformanceProfiler::GetPerformanceReport() const {
    std::stringstream report;
    
    report << "=== Performance Report ===\\n";
    report << "Current FPS: " << std::fixed << std::setprecision(1) << currentFrame.fps << "\\n";
    report << "Current Frame Time: " << currentFrame.frameTime << "ms\\n";
    report << "Average FPS (60 frames): " << GetAverageFPS(60) << "\\n";
    report << "Average Frame Time (60 frames): " << GetAverageFrameTime(60) << "ms\\n";
    report << "Smoothed FPS: " << smoothedFPS << "\\n";
    
    report << "\\n=== Resource Usage ===\\n";
    report << "Draw Calls: " << currentFrame.drawCalls << "\\n";
    report << "Triangles: " << currentFrame.triangles << "\\n";
    report << "Memory Usage: " << (currentFrame.memoryUsage / 1024 / 1024) << " MB\\n";
    report << "CPU Time: " << currentFrame.cpuTime << "ms\\n";
    report << "GPU Time (est): " << currentFrame.gpuTime << "ms\\n";
    
    report << "\\n=== Section Timings ===\\n";
    for (const auto& [name, timing] : timingSections) {
        report << name << ": avg=" << timing.avgTime << "ms, max=" << timing.maxTime 
               << "ms, calls=" << timing.callCount << "\\n";
    }
    
    return report.str();
}

std::vector<std::string> PerformanceProfiler::GetBottlenecks() const {
    std::vector<std::string> bottlenecks;
    
    
    if (GetAverageFPS(30) < targetFPS * 0.9f) {
        bottlenecks.push_back("Low FPS: " + std::to_string(GetAverageFPS(30)) + " (target: " + std::to_string(targetFPS) + ")");
    }
    
    
    if (currentFrame.drawCalls > 1000) {
        bottlenecks.push_back("High draw calls: " + std::to_string(currentFrame.drawCalls));
    }
    
    
    if (currentFrame.triangles > 500000) {
        bottlenecks.push_back("High triangle count: " + std::to_string(currentFrame.triangles));
    }
    
    
    if (currentFrame.memoryUsage > 1024 * 1024 * 1024) { 
        bottlenecks.push_back("High memory usage: " + std::to_string(currentFrame.memoryUsage / 1024 / 1024) + " MB");
    }
    
    
    for (const auto& [name, timing] : timingSections) {
        if (timing.avgTime > 5.0f) { 
            bottlenecks.push_back("Slow section '" + name + "': " + std::to_string(timing.avgTime) + "ms");
        }
    }
    
    return bottlenecks;
}

void PerformanceProfiler::ExportToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open performance log file: " << filename << std::endl;
        return;
    }
    
    file << GetPerformanceReport() << std::endl;
    
    file << "\\n=== Frame History ===\\n";
    file << "Frame,FPS,FrameTime(ms),CPUTime(ms),GPUTime(ms),DrawCalls,Triangles,Memory(MB)\\n";
    
    for (size_t i = 0; i < frameHistory.size(); i++) {
        const auto& frame = frameHistory[i];
        file << i << "," << frame.fps << "," << frame.frameTime << "," 
             << frame.cpuTime << "," << frame.gpuTime << "," 
             << frame.drawCalls << "," << frame.triangles << "," 
             << (frame.memoryUsage / 1024 / 1024) << "\\n";
    }
    
    file.close();
}

void PerformanceProfiler::PrintRealTimeStats() const {
    std::cout << "[PERF] FPS: " << std::fixed << std::setprecision(1) << currentFrame.fps 
              << " | Frame: " << currentFrame.frameTime << "ms"
              << " | Draw Calls: " << currentFrame.drawCalls 
              << " | Triangles: " << currentFrame.triangles << std::endl;
}

void PerformanceProfiler::ClearHistory() {
    frameHistory.clear();
    timingSections.clear();
}

size_t PerformanceProfiler::GetEstimatedMemoryUsage() const {
    
    size_t baseMemory = sizeof(*this);
    baseMemory += frameHistory.capacity() * sizeof(FrameStats);
    baseMemory += timingSections.size() * (sizeof(std::string) + sizeof(SectionTiming));
    
    return baseMemory + currentFrame.memoryUsage;
}

float PerformanceProfiler::GetGPULoad() const {
    
    float drawCallLoad = std::min(1.0f, currentFrame.drawCalls / 1000.0f);
    float triangleLoad = std::min(1.0f, currentFrame.triangles / 500000.0f);
    
    return std::max(drawCallLoad, triangleLoad);
}

void PerformanceProfiler::AddFrameToHistory(const FrameStats& frame) {
    frameHistory.push_back(frame);
    
    
    if (frameHistory.size() > maxFrameHistory) {
        frameHistory.erase(frameHistory.begin());
    }
}

float PerformanceProfiler::CalculateGPUTime() const {
    
    float baseGPUTime = currentFrame.drawCalls * 0.01f; 
    float triangleTime = currentFrame.triangles / 100000.0f; 
    
    return baseGPUTime + triangleTime;
}


void PerformanceMonitor::RenderOverlay() {
    auto& profiler = PerformanceProfiler::getInstance();
    
    
    
    static int frameCounter = 0;
    frameCounter++;
    
    if (frameCounter % 60 == 0) { 
        std::cout << "\\n[PERFORMANCE OVERLAY]" << std::endl;
        std::cout << "FPS: " << profiler.GetCurrentFPS() << std::endl;
        std::cout << "Frame Time: " << profiler.GetCurrentFrameTime() << "ms" << std::endl;
        std::cout << "GPU Load: " << (profiler.GetGPULoad() * 100) << "%" << std::endl;
        
        auto bottlenecks = profiler.GetBottlenecks();
        if (!bottlenecks.empty()) {
            std::cout << "Bottlenecks:" << std::endl;
            for (const auto& bottleneck : bottlenecks) {
                std::cout << "  - " << bottleneck << std::endl;
            }
        }
        std::cout << std::endl;
    }
}

void PerformanceMonitor::LogPerformanceWarnings() {
    auto& profiler = PerformanceProfiler::getInstance();
    
    if (profiler.IsPerformanceCritical()) {
        std::cout << "[WARNING] Performance is critical!" << std::endl;
        
        auto bottlenecks = profiler.GetBottlenecks();
        for (const auto& bottleneck : bottlenecks) {
            std::cout << "[WARNING] " << bottleneck << std::endl;
        }
    }
}

void PerformanceMonitor::CheckPerformanceThresholds() {
    auto& profiler = PerformanceProfiler::getInstance();
    
    static bool warningShown = false;
    
    if (profiler.GetCurrentFPS() < 30.0f && !warningShown) {
        std::cout << "[CRITICAL] FPS dropped below 30! Current: " << profiler.GetCurrentFPS() << std::endl;
        warningShown = true;
    } else if (profiler.GetCurrentFPS() > 35.0f) {
        warningShown = false; 
    }
}
