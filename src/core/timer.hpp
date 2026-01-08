#pragma once

#include <chrono>
#include <string>

namespace usd_vk {

class Timer {
public:
    Timer();
    void start();
    void stop();
    void reset();
    void restart();

    double elapsed() const;
    double elapsedMs() const;

private:
    std::chrono::high_resolution_clock::time_point m_start;
    std::chrono::high_resolution_clock::time_point m_end;
    bool m_running = false;
};

class ScopedTimer {
public:
    explicit ScopedTimer(const char* name);
    explicit ScopedTimer(double& output);
    ~ScopedTimer();

private:
    const char* m_name = nullptr;
    double* m_output = nullptr;
    Timer m_timer;
};

} // namespace usd_vk
